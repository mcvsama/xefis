/* vim:ts=4
 *
 * Copyleft 2012…2016  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

// Local:
#include "xbee.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/blob.h>
#include <xefis/utility/string.h>

// Neutrino:
#include <neutrino/bus/serial_port.h>
#include <neutrino/qt/qdom.h>
#include <neutrino/string.h>
#include <neutrino/time_helper.h>

// Qt:
#include <QtCore/QTimer>

// Lib:
#include <boost/endian/conversion.hpp>

// System:
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

// Standard:
#include <cstddef>
#include <random>
#include <tuple>
#include <iomanip>


XBee::XBee (xf::Logger const& logger, std::string_view const& instance):
	XBeeIO (instance),
	_logger (logger.with_scope (std::string (kLoggerScope) + "#" + instance))
{
	_restart_timer = new QTimer (this);
	_restart_timer->setInterval (kRestartAfter.in<si::Millisecond>());
	_restart_timer->setSingleShot (true);
	QObject::connect (_restart_timer, SIGNAL (timeout()), this, SLOT (open_device()));

	// Ping timer pings modem periodically. After each ping alive-check-timer is started
	// to see if there's response. If there's none, failure() is called.
	_periodic_ping_timer = new QTimer (this);
	_periodic_ping_timer->setInterval (kPeriodicAliveCheck.in<si::Millisecond>());
	_periodic_ping_timer->setSingleShot (false);
	QObject::connect (_periodic_ping_timer, SIGNAL (timeout()), this, SLOT (periodic_ping()));

	// Clear channel assessment timer.
	_clear_channel_timer = new QTimer (this);
	_clear_channel_timer->setInterval (kClearChannelCheck.in<si::Millisecond>());
	_clear_channel_timer->setSingleShot (false);
	QObject::connect (_clear_channel_timer, SIGNAL (timeout()), this, SLOT (clear_channel_check()));

	_periodic_pong_timer = new QTimer (this);
	_periodic_pong_timer->setInterval (kPeriodicAliveCheckTimeout.in<si::Millisecond>());
	_periodic_pong_timer->setSingleShot (true);
	QObject::connect (_periodic_pong_timer, SIGNAL (timeout()), this, SLOT (periodic_pong_timeout()));

	_pong_timer = new QTimer (this);
	_pong_timer->setSingleShot (true);
	QObject::connect (_pong_timer, SIGNAL (timeout()), this, SLOT (pong_timeout()));

	_after_reset_timer = new QTimer (this);
	_after_reset_timer->setInterval (kAfterRestartGraceTime.in<si::Millisecond>());
	_after_reset_timer->setSingleShot (true);
	QObject::connect (_after_reset_timer, SIGNAL (timeout()), this, SLOT (continue_after_reset()));

	_rssi_timer = new QTimer (this);
	_rssi_timer->setInterval (kRSSITimeout.in<si::Millisecond>());
	_rssi_timer->setSingleShot (true);
	QObject::connect (_rssi_timer, SIGNAL (timeout()), this, SLOT (rssi_timeout()));
	_rssi_timer->start();

	if (*_io.local_address == 0xffff)
	{
		_logger << "Can't use local address ff:ff, 64-bit addressing is unsupported. Setting to default 00:00." << std::endl;
		*_io.local_address = 0x0000;
	}

	if (*_io.remote_address == 0xffff)
	{
		_logger << "Can't use remote address ff:ff, 64-bit addressing is unsupported. Setting to default 00:00." << std::endl;
		*_io.remote_address = 0x0000;
	}

	_io.serviceable.set_fallback (false);
	_io.input_errors.set_fallback (0);
	_io.failures.set_fallback (0);
	_io.cca_failures.set_fallback (0);

	open_device();
}


XBee::~XBee()
{
	if (_device != 0)
		::close (_device);
}


void
XBee::process (xf::Cycle const&)
{
	// If device is not open, skip.
	if (!_notifier)
		return;

	if (_io.send && _send_changed.serial_changed() && configured())
	{
		std::string data = _output_buffer + *_io.send;
		std::vector<std::string> packets = packetize (data, 100); // Max 100 bytes per packet according to XBee docs.

		auto send_back_to_output_buffer = [&] (std::string_view const& front) -> void
		{
			// Add the rest of packets back to output buffer:
			_output_buffer.clear();
			_output_buffer += front;

			for (auto const& s: packets)
				_output_buffer += s;
		};

		while (!packets.empty())
		{
			std::string s = packets.front();
			std::string frame = make_frame (make_tx16_command (*_io.remote_address, s));
			packets.erase (packets.begin());

			int written = 0;
			switch (send_frame (frame, written))
			{
				case SendResult::Success:
					break;

				case SendResult::Retry:
					if (send_failed_with_retry())
					{
						// Probably too fast data transmission for given modem settings.
						// Drop this packet.
						_logger << "Possibly too fast data transmission. Consider increasing baud rate of the modem." << std::endl;
						failure ("multiple EAGAIN during write, restarting");
					}
					break;

				case SendResult::Failure:
					send_back_to_output_buffer (s);
					failure ("sending packet");
					break;
			}
		}
	}
}


void
XBee::read()
{
	std::string buffer;

	bool err = false;
	bool exc = xf::Exception::catch_and_log (_logger, [&] {
		// Read as much as possible:
		for (;;)
		{
			std::string::size_type prev_size = buffer.size();
			std::string::size_type try_read = 1024u;
			buffer.resize (prev_size + try_read);
			int n = ::read (_device, &buffer[prev_size], try_read);

			if (n < 0)
			{
				if (errno == EAGAIN || errno == EWOULDBLOCK)
				{
					// Nothing to read (read would block)
					buffer.resize (prev_size);
					break;
				}
				else
				{
					_logger << "Error while reading from serial port: " << strerror (errno) << std::endl;
					err = true;
					break;
				}
			}
			else
			{
				buffer.resize (prev_size + neutrino::to_unsigned (n));

				if (n == 0)
				{
					_read_failure_count++;
					if (_read_failure_count > kMaxReadFailureCount)
					{
						failure ("multiple read failures");
						_read_failure_count = 0;
					}
				}
				else
					_read_failure_count = 0;

				if (n < static_cast<int> (try_read))
					break;
			}
		}
	});

	if (exc || err)
		failure ("read()");

	if (!buffer.empty())
	{
		_input_buffer += buffer;
		process_input();
	}
}


void
XBee::open_device()
{
	try {
		_logger << "Opening device " << *_io.device_path << std::endl;

		reset();

		_device = ::open (_io.device_path->c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);

		if (_device < 0)
		{
			_logger << "Could not open device file " << *_io.device_path << ": " << strerror (errno) << std::endl;
			restart();
		}
		else
		{
			if (!set_device_options())
				failure ("set_device_options()");
			else
			{
				_notifier = std::make_unique<QSocketNotifier> (_device, QSocketNotifier::Read, this);
				_notifier->setEnabled (true);
				QObject::connect (_notifier.get(), SIGNAL (activated (int)), this, SLOT (read()));

				configure_modem();
			}
		}
	}
	catch (...)
	{
		failure ("exception in open_device()");
	}
}


void
XBee::failure (std::string_view const& reason)
{
	auto log = _logger << "Failure detected";

	if (!reason.empty())
		log << ": " << reason;

	log << ", closing device " << *_io.device_path << std::endl;
	_notifier.reset();
	::close (_device);
	_io.failures = *_io.failures + 1;
	restart();
}


void
XBee::reset()
{
	pong();
	stop_periodic_ping();
	_configuration_step = ConfigurationStep::Unconfigured;
	_io.serviceable = false;
	_output_buffer.clear();
	_restart_timer->stop();
	_after_reset_timer->stop();
	_io.receive = xf::nil;
}


void
XBee::restart()
{
	reset();
	_restart_timer->start();
}


void
XBee::periodic_ping()
{
	// Start or restart periodic ping timer:
	_periodic_ping_timer->start();
	// Start CCA timer too:
	if (!_clear_channel_timer->isActive())
		_clear_channel_timer->start();

	int written = 0;
	switch (send_frame (make_frame (make_at_command ("AI", kPeriodicPingFrameID)), written))
	{
		case SendResult::Success:
			_periodic_pong_timer->start();
			break;

		case SendResult::Retry:
			if (send_failed_with_retry())
			{
				// Restart:
				_logger << "Could not send ATAI command. Probably too fast data transmission. Consider increasing baud rate of the modem." << std::endl;
				failure ("multiple EAGAIN during write, restarting");
			}
			break;

		case SendResult::Failure:
			failure ("sending ping packet");
			break;
	}
}


void
XBee::clear_channel_check()
{
	int written = 0;
	switch (send_frame (make_frame (make_at_command ("EC", kClearChannelFrameID)), written))
	{
		case SendResult::Success:
			break;

		case SendResult::Retry:
			if (send_failed_with_retry())
			{
				// Restart:
				_logger << "Could not send ATEC command. Probably too fast data transmission. Consider increasing baud rate of the modem." << std::endl;
				failure ("multiple EAGAIN during write, restarting");
			}
			break;

		case SendResult::Failure:
			// Nothing serious happened, ignore.
			break;
	}
}


void
XBee::pong_timeout()
{
	failure ("alive-check timeout");
}


void
XBee::periodic_pong_timeout()
{
	failure ("periodic alive-check timeout");
}


void
XBee::continue_after_reset()
{
	configure_modem (static_cast<uint8_t> (_configuration_step), ATResponseStatus::OK, "");
}


void
XBee::rssi_timeout()
{
	_io.rssi = xf::nil;
}


bool
XBee::set_device_options()
{
	_logger << "Setting baud rate to " << *_io.baud_rate << std::endl;

#if 0 // TODO
	SerialPort::Configuration configuration;
	configuration.set_read_timeout (0.1_s);
	configuration.set_baud_rate (*_io.baud_rate);
#else
	termios options;
	bzero (&options, sizeof (options));

	// Min characters to be read:
	options.c_cc[VMIN] = 0;
	// Time to wait for data (tenths of seconds):
	options.c_cc[VTIME] = 1;
	// Set output and local modes to defaults:
	options.c_cflag = CS8 | CREAD | CLOCAL;
	options.c_iflag = IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = 0;

	cfsetispeed (&options, *_io.baud_rate);
	cfsetospeed (&options, *_io.baud_rate);

	tcflush (_device, TCIOFLUSH);

	if (tcsetattr (_device, TCSANOW, &options) != 0)
	{
		_logger << "Could not setup serial port: " << *_io.device_path << ": " << strerror (errno) << std::endl;
		return false;
	}

	if (tcflow (_device, TCOON | TCION) != 0)
	{
		_logger << "Could not enable flow: tcflow(): " << *_io.device_path << ": " << strerror (errno) << std::endl;
		return false;
	}
#endif

	return true;
}


void
XBee::configure_modem (uint8_t frame_id, ATResponseStatus status, std::string_view const& response)
{
	auto request_at = [&] (ConfigurationStep next_step, std::string const& at, std::vector<uint8_t> data_bytes = {}) -> void
	{
		_configuration_step = next_step;

		std::string full_at = at;

		for (uint8_t b: data_bytes)
			full_at += static_cast<char> (b);

		if (*_io.debug)
			debug() << "Sending AT command " << at << ": " << neutrino::to_hex_string (full_at) << std::endl;

		int written = 0;
		_last_at_command = full_at;
		if (send_frame (make_frame (make_at_command (full_at, static_cast<uint8_t> (next_step))), written) != SendResult::Success)
			failure ("initialization: " + at);
		else
			ping (kCommandTimeout);
	};

	if (status != ATResponseStatus::OK && status != ATResponseStatus::StartConfig)
	{
		failure ("initialization fail at command: AT" + _last_at_command);
	}
	else if (frame_id != static_cast<uint8_t> (_configuration_step))
	{
		_logger << "Unexpected response from modem with wrong frame ID: 0x"
			  << std::hex << std::setw (2) << std::setfill ('0') << frame_id << std::dec << std::endl;
		failure ("communication protocol failure");
	}
	else
	{
		pong();

		switch (_configuration_step)
		{
			case ConfigurationStep::Unconfigured:
				_logger << "Starting modem configuration." << std::endl;
				_io.serviceable = false;

				request_at (ConfigurationStep::SoftwareReset, "FR");
				// Note: this will cause immediate response and also 'watchdog reset' after a while.
				// Disregard the immediate response and wait for watchdog reset message.
				break;

			case ConfigurationStep::SoftwareReset:
				// Disregard this response. Wait for ModemStatus::WatchdogReset.
				ping (kCommandTimeout);
				break;

			case ConfigurationStep::AfterSoftwareReset:
				request_at (ConfigurationStep::DisableIOUART, "IU", { 0x00 });
				break;

			case ConfigurationStep::DisableIOUART:
				request_at (ConfigurationStep::ReadHardwareVersion, "HV");
				break;

			case ConfigurationStep::ReadHardwareVersion:
				_logger << "Hardware version: " << neutrino::to_hex_string (response) << std::endl;

				request_at (ConfigurationStep::ReadFirmwareVersion, "VR");
				break;

			case ConfigurationStep::ReadFirmwareVersion:
				_logger << "Firmware version: " << neutrino::to_hex_string (response) << std::endl;

				request_at (ConfigurationStep::ReadSerialNumberH, "SH");
				break;

			case ConfigurationStep::ReadSerialNumberH:
				_serial_number_bin = response;
				request_at (ConfigurationStep::ReadSerialNumberL, "SL");
				break;

			case ConfigurationStep::ReadSerialNumberL:
				_serial_number_bin += response;
				_logger << "Serial number: " << neutrino::to_hex_string (_serial_number_bin) << std::endl;

				request_at (ConfigurationStep::DisableSleep, "SM", { 0x00 });
				break;

			case ConfigurationStep::DisableSleep:
				request_at (ConfigurationStep::DisableEncryption, "EE", { 0x00 });
				break;

			case ConfigurationStep::DisableEncryption:
				request_at (ConfigurationStep::DisableACKs, "MM", { 0x01 });
				break;

			case ConfigurationStep::DisableACKs:
				// Max association sleep period: 100 ms = 0x64
				request_at (ConfigurationStep::SetAssociationSleepPeriod, "DP", { 0x00, 0x64 });
				break;

			case ConfigurationStep::SetAssociationSleepPeriod:
				request_at (ConfigurationStep::SetAssociationParams, "A1", { 0x00 });
				break;

			case ConfigurationStep::SetAssociationParams:
				request_at (ConfigurationStep::SetChannel, "CH", { static_cast<uint8_t> (*_io.channel) });
				break;

			case ConfigurationStep::SetChannel:
				request_at (ConfigurationStep::SetPersonalAreaNetworkID, "ID", { static_cast<uint8_t> (*_io.pan_id >> 8), static_cast<uint8_t> (*_io.pan_id) });
				break;

			case ConfigurationStep::SetPersonalAreaNetworkID:
				request_at (ConfigurationStep::SetDestinationAddressH, "DH", { 0x00, 0x00, 0x00, 0x00 });
				break;

			case ConfigurationStep::SetDestinationAddressH:
				request_at (ConfigurationStep::SetDestinationAddressL, "DL", { 0x00, 0x00, static_cast<uint8_t> (*_io.remote_address >> 8), static_cast<uint8_t> (*_io.remote_address) });
				break;

			case ConfigurationStep::SetDestinationAddressL:
				request_at (ConfigurationStep::SetLocalAddress, "MY", { static_cast<uint8_t> (*_io.local_address >> 8), static_cast<uint8_t> (*_io.local_address) });
				break;

			case ConfigurationStep::SetLocalAddress:
				if (_io.power_level)
				{
					request_at (ConfigurationStep::SetPowerLevel, "PL", { static_cast<uint8_t> (*_io.power_level) });
					break;
				}
				else
					_configuration_step = ConfigurationStep::SetPowerLevel;
				// Fall-through.

			case ConfigurationStep::SetPowerLevel:
				request_at (ConfigurationStep::SetCoordinatorMode, "CE", { 0x00 });
				break;

			case ConfigurationStep::SetCoordinatorMode:
				_logger << "Modem configured." << std::endl;
				_configuration_step = ConfigurationStep::Configured;
				_io.serviceable = true;
				periodic_ping();
				break;

			case ConfigurationStep::Configured:
				_configuration_step = ConfigurationStep::Unconfigured;
				stop_periodic_ping();
				break;
		}
	}
}


int
XBee::baud_rate_to_xbee_code (int baud_rate)
{
	switch (baud_rate)
	{
		case 1200:		return 0;
		case 2400:		return 1;
		case 4800:		return 2;
		case 9600:		return 3;
		case 19200:		return 4;
		case 38400:		return 5;
		case 57600:		return 6;
		case 115200:	return 7;
		default:		return 0;
	}
}


std::string
XBee::make_frame (std::string_view const& data) const
{
	if (data.size() > 0xffff)
		throw xf::Exception ("max frame size is 0xffff");

	std::string result;

	// Frame delimiter:
	result.push_back (0x7e);
	// Data size:
	uint16_t size = data.size();
	result.push_back ((size >> 8) & 0xff); // MSB
	result.push_back ((size >> 0) & 0xff); // LSB
	// Data:
	result += data;
	// Checksum:
	uint8_t checksum = 0xff;

	for (char c: data)
		checksum -= static_cast<uint8_t> (c);

	result.push_back (static_cast<char> (checksum));

	return result;
}


std::string
XBee::make_tx64_command (uint64_t address, std::string_view const& data) const
{
	std::string result;

	// API ID:
	result.push_back (static_cast<uint8_t> (SendAPI::TX64));
	// Frame ID for ACK (select none):
	result.push_back (0x00);
	// Destination address, MSB first:
	for (int shift = 5; shift >= 0; --shift)
		result.push_back ((address >> (8 * shift)) & 0xff);
	// Options (0x01 - disable ACK):
	result.push_back (0x01);
	// Data:
	result += data;

	return result;
}


std::string
XBee::make_tx16_command (uint16_t address, std::string_view const& data) const
{
	std::string result;

	// API ID:
	result.push_back (static_cast<uint8_t> (SendAPI::TX16));
	// Frame ID for ACK (select none):
	result.push_back (0x00);
	// Destination address, MSB first:
	for (int shift = 1; shift >= 0; --shift)
		result.push_back ((address >> (8 * shift)) & 0xff);
	// Options (0x01 - disable ACK):
	result.push_back (0x01);
	// Data:
	result += data;

	return result;
}


std::string
XBee::make_at_command (std::string_view const& at_command, uint8_t frame_id)
{
	std::string result;

	// API ID:
	result.push_back (static_cast<uint8_t> (SendAPI::ATCommand));
	// Frame ID for ACK (select ATFrameID just to get any response):
	result.push_back (static_cast<char> (frame_id));
	// Command:
	result += at_command;

	return result;
}


XBee::SendResult
XBee::send_frame (std::string_view const& frame, int& written)
{
	written = ::write (_device, frame.data(), frame.size());

	if (written == -1)
	{
		_logger << "Write error " << strerror (errno) << std::endl;

		written = 0;
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return SendResult::Retry;
		else
			return SendResult::Failure;
	}
	else if (written < static_cast<int> (frame.size()))
	{
		_logger << "Write buffer overrun." << std::endl;
		return SendResult::Retry;
	}

	return SendResult::Success;
}


bool
XBee::send_failed_with_retry()
{
	_write_failure_count++;
	bool should_restart = _write_failure_count > kMaxWriteFailureCount || _output_buffer.size() > kMaxOutputBufferSize;

	if (should_restart)
		_write_failure_count = 0;

	return should_restart;
}


std::vector<std::string>
XBee::packetize (std::string_view const& data, std::size_t size) const
{
	if (data.size() <= size)
		return { std::string (data) };

	std::vector<std::string> result;

	for (std::size_t p = 0; p < data.size(); p += size)
		result.push_back (std::string (data.substr (p, size)));

	return result;
}


bool
XBee::vector_to_uint16 (std::vector<uint8_t> const& vector, uint16_t& result) const
{
	if (vector.size() != 2)
		return false;

	result = (static_cast<uint16_t> (vector[0]) >> 8) | vector[1];
	return true;
}


void
XBee::process_input()
{
	ResponseAPI api;
	std::string data;

	while (process_packet (_input_buffer, api, data))
	{
		switch (api)
		{
			case ResponseAPI::RX64:
				process_rx64_frame (data);
				break;

			case ResponseAPI::RX16:
				process_rx16_frame (data);
				break;

			case ResponseAPI::TXStatus:
				// Not really supported/handled. Just ignore.
				break;

			case ResponseAPI::ModemStatus:
				process_modem_status_frame (data);
				break;

			case ResponseAPI::ATResponse:
				process_at_response_frame (data);
				break;
		}
	}
}


bool
XBee::process_packet (std::string& input, ResponseAPI& api, std::string& data)
{
	for (;;)
	{
		std::string::size_type p = input.find (kPacketDelimiter);

		if (p == std::string::npos)
		{
			input.clear();
			return false;
		}

		// Discard non-parseable data:
		input.erase (input.begin(), input.begin() + neutrino::to_signed (p));

		_io.input_errors = *_io.input_errors + neutrino::to_signed (p);

		// Delimiter (1B) + packet size (2B) + data (1B) + checksum (1B) gives
		// at least 5 bytes:
		if (input.size() < 5)
			return false;

		// Packet size:
		uint32_t size = (static_cast<uint32_t> (input[1]) << 8u) + static_cast<uint32_t> (input[2]);
		if (input.size() < size + 4u) // delimiter, size, checksum = 4B
			return false;

		// Checksum:
		uint8_t checksum = 0;
		for (std::string::size_type i = 3; i < size + 4u; ++i)
			checksum += static_cast<uint8_t> (input[i]);
		if (checksum != 0xff)
		{
			_logger << "Checksum invalid on input packet." << std::endl;
			// Checksum invalid. Discard data up to next packet delimiter.
			input.erase (0);
			// Try parsing again:
			continue;
		}

		// Data is there, checksum is valid, what else do we need?
		api = static_cast<ResponseAPI> (input[3]);
		data = input.substr (4, size - 1);
		// Remove packet from buffer:
		input.erase (input.begin(), input.begin() + size + 4u);

		return true;
	}
}


void
XBee::process_rx64_frame (std::string_view const& frame)
{
	if (*_io.debug)
		debug() << ">> RX64 data: " << neutrino::to_hex_string (frame) << std::endl;

	// At least 11 bytes:
	if (frame.size() < 11)
		return;

	// 64-bit address:
	uint64_t address = 0;
	for (auto b = 0u; b < 8u; ++b)
		address |= (static_cast<uint32_t> (frame[b]) << (8u * (7u - b)));

	// -RSSI dBm:
	int rssi = frame[8];
	rssi = -rssi;

	// Options:
	uint8_t options = static_cast<uint8_t> (frame[9]);
	// We're not going to accept broadcast packets, sorry:
	if (options & 0x06)
	{
		_logger << "Got packet with broadcast " << (options & 0x02 ? "address" : "pan") << ". Ignoring." << std::endl;
		return;
	}

	// Frame data:
	write_output_socket (frame.substr (10));
	report_rssi (rssi);
}


void
XBee::process_rx16_frame (std::string_view const& frame)
{
	if (*_io.debug)
		debug() << ">> RX16 data: " << neutrino::to_hex_string (frame) << std::endl;

	// At least 5 bytes:
	if (frame.size() < 5)
		return;

	// 16-bit address:
	uint16_t address = (static_cast<uint16_t> (frame[0]) << 8) | frame[1];
	// Address must match our peer's address:
	if (address != *_io.remote_address)
	{
		_logger << "Got packet from unknown address: " << neutrino::to_hex_string (frame.substr (0, 2)) << ". Ignoring." << std::endl;
		return;
	}

	// -RSSI dBm:
	int rssi = frame[2];
	rssi = -rssi;

	// Options:
	uint8_t options = static_cast<uint8_t> (frame[3]);
	// We're not going to accept broadcast packets, sorry:
	if (options & 0x06)
	{
		_logger << "Got packet with broadcast " << (options & 0x02 ? "address" : "pan") << ". Ignoring." << std::endl;
		return;
	}

	// Frame data:
	write_output_socket (frame.substr (4));
	report_rssi (rssi);
}


void
XBee::process_modem_status_frame (std::string_view const& data)
{
	if (*_io.debug)
		debug() << ">> Modem status: " << neutrino::to_hex_string (data) << std::endl;

	if (data.size() < 1)
		return;

	ModemStatus status = static_cast<ModemStatus> (data[0]);

	switch (status)
	{
		case ModemStatus::HardwareReset:
			_logger << "Modem reported hardware reset." << std::endl;
			failure ("unexpected hardware reset");
			break;

		case ModemStatus::WatchdogReset:
			_logger << "Modem reported watchdog reset." << std::endl;
			// If caused by configuration process, continue with it.
			if (_configuration_step == ConfigurationStep::SoftwareReset)
			{
				pong();
				_configuration_step = ConfigurationStep::AfterSoftwareReset;
				_after_reset_timer->start();
			}
			// Otherwise treat as failure.
			else
				failure ("unexpected watchdog reset");
			break;

		case ModemStatus::Associated:
			_logger << "Associated." << std::endl;
			break;

		case ModemStatus::Disassociated:
			_logger << "Disassociated." << std::endl;
			break;

		case ModemStatus::SynchronizationLost:
			_logger << "Synchronization lost." << std::endl;
			break;

		case ModemStatus::CoordinatorRealignment:
			_logger << "Coordinator realignment." << std::endl;
			break;

		case ModemStatus::CoordinatorStarted:
			_logger << "Coordinator started." << std::endl;
			break;

		default:
			_logger << "Modem reported unknown status: 0x"
					<< std::hex << static_cast<int> (data[0]) << std::dec << std::endl;
	}
}


void
XBee::process_at_response_frame (std::string_view const& frame)
{
	if (*_io.debug)
		debug() << ">> AT status: " << neutrino::to_hex_string (frame) << std::endl;

	// Response must be at least 4 bytes long:
	if (frame.size() < 4)
		return;

	// AT command response:
	// 1B frame-ID:
	uint8_t frame_id = static_cast<uint8_t> (frame[0]);

	// 2B AT command - skip
	std::string_view command = frame.substr (1, 2);

	// 1B status (0 = OK, 1 = ERROR, 2 = invalid command, 3 = invalid param)
	ATResponseStatus status = static_cast<ATResponseStatus> (frame[3]);

	// Data:
	std::string_view response_data = frame.substr (4);

	if (*_io.debug)
	{
		auto log = debug();
		log << "Command result: " << command << " ";
		switch (status)
		{
			case ATResponseStatus::OK:					log << "OK"; break;
			case ATResponseStatus::ERROR:				log << "ERROR"; break;
			case ATResponseStatus::InvalidCommand:		log << "Invalid command"; break;
			case ATResponseStatus::InvalidParameter:	log << "Invalid parameter"; break;
			default:									log << "?"; break;
		}
		log << ", data: " << neutrino::to_hex_string (response_data) << std::endl;
	}

	// Response data: bytes
	if (frame_id == kPeriodicPingFrameID)
		periodic_pong (status, response_data);
	else if (frame_id == kClearChannelFrameID)
		clear_channel_result (status, response_data);
	else
		configure_modem (frame_id, status, response_data);
}


void
XBee::write_output_socket (std::string_view const& data)
{
	if (configured())
		_io.receive = std::string (data);
}


void
XBee::report_rssi (int dbm)
{
	// Restart timer:
	_rssi_timer->start();

	// Convert dBm to milliwatts:
	si::Power power = 1_mW * std::pow (10.0, 0.1 * static_cast<double> (dbm));
	si::Time now = xf::TimeHelper::now();
	_io.rssi = _rssi_smoother (power, now - _last_rssi_time);
	_last_rssi_time = now;
}


void
XBee::ping (si::Time const timeout)
{
	_pong_timer->stop();
	_pong_timer->setInterval (timeout.in<si::Millisecond>());
	_pong_timer->start();
}


void
XBee::pong()
{
	_pong_timer->stop();
}


void
XBee::periodic_pong (ATResponseStatus status, std::string_view const& data)
{
	if (status != ATResponseStatus::OK)
		failure ("check-alive packet status non-OK");
	else if (data.size() >= 1)
	{
		if (data[0] != 0x00)
			_logger << "Association status: 0x" << std::hex << std::setw (2) << std::setfill ('0') << static_cast<int> (data[0]) << std::dec << std::endl;
	}

	_periodic_pong_timer->stop();
}


void
XBee::stop_periodic_ping()
{
	_periodic_ping_timer->stop();
	_periodic_pong_timer->stop();
	_clear_channel_timer->stop();
}


void
XBee::clear_channel_result (ATResponseStatus status, std::string_view const& result)
{
	if (status == ATResponseStatus::OK && result.size() >= 2)
	{
		// TODO Use Blob & BlobViews instead of std::string & std::string_view
		uint16_t failures = (static_cast<uint16_t> (static_cast<uint8_t> (result[0])) >> 8) | result[1];
		_io.cca_failures = *_io.cca_failures + failures;
	}
}


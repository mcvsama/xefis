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

// Standard:
#include <cstddef>
#include <random>
#include <tuple>
#include <iomanip>

// System:
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

// Qt:
#include <QtXml/QDomElement>

// Lib:
#include <boost/endian/conversion.hpp>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/bus/serial_port.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/blob.h>
#include <xefis/utility/time_helper.h>

// Local:
#include "xbee.h"


XEFIS_REGISTER_MODULE_CLASS ("io/xbee", XBee)


constexpr int		XBee::MaxReadFailureCount;
constexpr int		XBee::MaxWriteFailureCount;
constexpr size_t	XBee::MaxOutputBufferSize;

constexpr uint8_t	XBee::PacketDelimiter;
constexpr uint8_t	XBee::PeriodicPingFrameID;
constexpr uint8_t	XBee::ClearChannelFrameID;

constexpr Time		XBee::CommandTimeout;
constexpr Time		XBee::RestartAfter;
constexpr Time		XBee::PeriodicAliveCheck;
constexpr Time		XBee::PeriodicAliveCheckTimeout;
constexpr Time		XBee::ClearChannelCheck;
constexpr Time		XBee::AfterRestartGraceTime;
constexpr Time		XBee::RSSITimeout;


XBee::XBee (v1::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	parse_settings (config, {
		{ "debug", _debug, false },
		{ "device", _device_path, true },
		{ "baud-rate", _baud_rate, true },
		{ "channel", _channel, true },
		{ "pan-id", _pan_id_string, true },
		{ "local-address", _local_address_string, true },
		{ "remote-address", _remote_address_string, true },
		{ "power-level", _power_level, false },
	});

	parse_properties (config, {
		{ "serviceable", _serviceable, true },
		{ "send", _send, true },
		{ "receive", _receive, true },
		{ "input-errors", _input_errors, true },
		{ "rssi-dbm", _rssi_dbm, true },
		{ "failures", _failures, true },
		{ "cca-failures", _cca_failures, false },
	});

	_restart_timer = new QTimer (this);
	_restart_timer->setInterval (RestartAfter.quantity<Millisecond>());
	_restart_timer->setSingleShot (true);
	QObject::connect (_restart_timer, SIGNAL (timeout()), this, SLOT (open_device()));

	// Ping timer pings modem periodically. After each ping alive-check-timer is started
	// to see if there's response. If there's none, failre() is called.
	_periodic_ping_timer = new QTimer (this);
	_periodic_ping_timer->setInterval (PeriodicAliveCheck.quantity<Millisecond>());
	_periodic_ping_timer->setSingleShot (false);
	QObject::connect (_periodic_ping_timer, SIGNAL (timeout()), this, SLOT (periodic_ping()));

	// Clear channel assessment timer.
	_clear_channel_timer = new QTimer (this);
	_clear_channel_timer->setInterval (ClearChannelCheck.quantity<Millisecond>());
	_clear_channel_timer->setSingleShot (false);
	QObject::connect (_clear_channel_timer, SIGNAL (timeout()), this, SLOT (clear_channel_check()));

	_periodic_pong_timer = new QTimer (this);
	_periodic_pong_timer->setInterval (PeriodicAliveCheckTimeout.quantity<Millisecond>());
	_periodic_pong_timer->setSingleShot (true);
	QObject::connect (_periodic_pong_timer, SIGNAL (timeout()), this, SLOT (periodic_pong_timeout()));

	_pong_timer = new QTimer (this);
	_pong_timer->setSingleShot (true);
	QObject::connect (_pong_timer, SIGNAL (timeout()), this, SLOT (pong_timeout()));

	_after_reset_timer = new QTimer (this);
	_after_reset_timer->setInterval (AfterRestartGraceTime.quantity<Millisecond>());
	_after_reset_timer->setSingleShot (true);
	QObject::connect (_after_reset_timer, SIGNAL (timeout()), this, SLOT (continue_after_reset()));

	_rssi_timer = new QTimer (this);
	_rssi_timer->setInterval (RSSITimeout.quantity<Millisecond>());
	_rssi_timer->setSingleShot (true);
	QObject::connect (_rssi_timer, SIGNAL (timeout()), this, SLOT (rssi_timeout()));
	_rssi_timer->start();

	if (!vector_to_uint16 (xf::parse_hex_string (_local_address_string), _local_address))
	{
		log() << "Error: local address must be 16-bit address in form 00:00 (eg. 12:34)." << std::endl;
		_local_address = 0x0000;
	}
	else if (_local_address == 0xffff)
	{
		log() << "Can't use local address ff:ff, 64-bit addressing is unsupported. Setting to default 00:00." << std::endl;
		_local_address = 0x0000;
	}

	if (!vector_to_uint16 (xf::parse_hex_string (_remote_address_string), _remote_address))
	{
		log() << "Error: remote address must be 16-bit address in form 00:00 (eg. 12:34)." << std::endl;
		_remote_address = 0x0000;
	}
	else if (_remote_address == 0xffff)
	{
		log() << "Can't use remote address ff:ff, 64-bit addressing is unsupported. Setting to default 00:00." << std::endl;
		_remote_address = 0x0000;
	}

	// PAN ID:
	if (!vector_to_uint16 (xf::parse_hex_string (_pan_id_string), _pan_id))
	{
		log() << "Invalid pan-id setting: must be 2-byte binary string (eg. 01:23). Setting pan-id to default 00:00." << std::endl;
		_pan_id = 0x0000;
	}

	_serviceable.set_default (false);
	_input_errors.set_default (0);
	_failures.set_default (0);
	_cca_failures.set_default (0);

	open_device();
}


XBee::~XBee()
{
	if (_device != 0)
		::close (_device);
}


void
XBee::data_updated()
{
	// If device is not open, skip.
	if (!_notifier)
		return;

	if (_send.valid() && _send.fresh() && configured())
	{
		std::string data = _output_buffer + *_send;
		std::vector<std::string> packets = packetize (data, 100); // Max 100 bytes per packet according to XBee docs.

		auto send_back_to_output_buffer = [&] (std::string const& front) -> void
		{
			// Add the rest of packets back to output buffer:
			_output_buffer.clear();
			_output_buffer += front;
			for (auto s: packets)
				_output_buffer += s;
		};

		while (!packets.empty())
		{
			std::string s = packets.front();
			std::string frame = make_frame (make_tx16_command (_remote_address, s));
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
						log() << "Possibly too fast data transmission. Consider increasing baud rate of the modem." << std::endl;
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
	bool exc = xf::Exception::guard ([&] {
		// Read as much as possible:
		for (;;)
		{
			std::string::size_type prev_size = buffer.size();
			std::string::size_type try_read = 1024;
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
					log() << "Error while reading from serial port: " << strerror (errno) << std::endl;
					err = true;
					break;
				}
			}
			else
			{
				buffer.resize (prev_size + n);
				if (n == 0)
				{
					_read_failure_count++;
					if (_read_failure_count > MaxReadFailureCount)
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
		log() << "Opening device " << _device_path.toStdString() << std::endl;

		reset();

		_device = ::open (_device_path.toStdString().c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);

		if (_device < 0)
		{
			log() << "Could not open device file " << _device_path.toStdString() << ": " << strerror (errno) << std::endl;
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
XBee::failure (std::string const& reason)
{
	log() << "Failure detected" << (reason.empty() ? "" : (": " + reason)) << ", closing device " << _device_path.toStdString() << std::endl;

	_notifier.reset();
	::close (_device);

	if (_failures.configured())
		_failures.write (*_failures + 1);

	restart();
}


void
XBee::reset()
{
	pong();
	stop_periodic_ping();
	_configuration_step = ConfigurationStep::Unconfigured;
	_serviceable.write (false);
	_output_buffer.clear();
	_restart_timer->stop();
	_after_reset_timer->stop();
	_send.set_nil();
	_receive.set_nil();
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
	switch (send_frame (make_frame (make_at_command ("AI", PeriodicPingFrameID)), written))
	{
		case SendResult::Success:
			_periodic_pong_timer->start();
			break;

		case SendResult::Retry:
			if (send_failed_with_retry())
			{
				// Restart:
				log() << "Could not send ATAI command. Probably too fast data transmission. Consider increasing baud rate of the modem." << std::endl;
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
	switch (send_frame (make_frame (make_at_command ("EC", ClearChannelFrameID)), written))
	{
		case SendResult::Success:
			break;

		case SendResult::Retry:
			if (send_failed_with_retry())
			{
				// Restart:
				log() << "Could not send ATEC command. Probably too fast data transmission. Consider increasing baud rate of the modem." << std::endl;
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
	_rssi_dbm.set_nil();
}


bool
XBee::set_device_options()
{
	log() << "Setting baud rate to " << _baud_rate << std::endl;

#if 0 // TODO
	SerialPort::Configuration configuration;
	configuration.set_read_timeout (0.1_s);
	configuration.set_baud_rate (_baud_rate);
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

	int baud_rate_const = xf::SerialPort::termios_baud_rate (_baud_rate);
	cfsetispeed (&options, baud_rate_const);
	cfsetospeed (&options, baud_rate_const);

	tcflush (_device, TCIOFLUSH);

	if (tcsetattr (_device, TCSANOW, &options) != 0)
	{
		log() << "Could not setup serial port: " << _device_path.toStdString() << ": " << strerror (errno) << std::endl;
		return false;
	}

	if (tcflow (_device, TCOON | TCION) != 0)
	{
		log() << "Could not enable flow: tcflow(): " << _device_path.toStdString() << ": " << strerror (errno) << std::endl;
		return false;
	}
#endif

	return true;
}


void
XBee::configure_modem (uint8_t frame_id, ATResponseStatus status, std::string const& response)
{
	auto request_at = [&] (ConfigurationStep next_step, std::string const& at, std::vector<uint8_t> data_bytes = {}) -> void
	{
		_configuration_step = next_step;

		std::string full_at = at;
		for (uint8_t b: data_bytes)
			full_at += static_cast<char> (b);

		if (_debug)
			debug() << "Sending AT command " << at << ": " << xf::to_hex_string (full_at) << std::endl;

		int written = 0;
		_last_at_command = full_at;
		if (send_frame (make_frame (make_at_command (full_at, static_cast<uint8_t> (next_step))), written) != SendResult::Success)
			failure ("initialization: " + at);
		else
			ping (CommandTimeout);
	};

	if (status != ATResponseStatus::OK && status != ATResponseStatus::StartConfig)
	{
		failure ("initialization fail at command: AT" + _last_at_command);
	}
	else if (frame_id != static_cast<uint8_t> (_configuration_step))
	{
		log() << "Unexpected response from modem with wrong frame ID: 0x"
			  << std::hex << std::setw (2) << std::setfill ('0') << frame_id << std::dec << std::endl;
		failure ("communication protocol failure");
	}
	else
	{
		pong();

		switch (_configuration_step)
		{
			case ConfigurationStep::Unconfigured:
				log() << "Starting modem configuration." << std::endl;
				_serviceable.write (false);

				request_at (ConfigurationStep::SoftwareReset, "FR");
				// Note: this will cause immediate response and also 'watchdog reset' after a while.
				// Disregard the immediate response and wait for watchdog reset message.
				break;

			case ConfigurationStep::SoftwareReset:
				// Disregard this response. Wait for ModemStatus::WatchdogReset.
				ping (CommandTimeout);
				break;

			case ConfigurationStep::AfterSoftwareReset:
				request_at (ConfigurationStep::DisableIOUART, "IU", { 0x00 });
				break;

			case ConfigurationStep::DisableIOUART:
				request_at (ConfigurationStep::ReadHardwareVersion, "HV");
				break;

			case ConfigurationStep::ReadHardwareVersion:
				log() << "Hardware version: " << xf::to_hex_string (response) << std::endl;

				request_at (ConfigurationStep::ReadFirmwareVersion, "VR");
				break;

			case ConfigurationStep::ReadFirmwareVersion:
				log() << "Firmware version: " << xf::to_hex_string (response) << std::endl;

				request_at (ConfigurationStep::ReadSerialNumberH, "SH");
				break;

			case ConfigurationStep::ReadSerialNumberH:
				_serial_number_bin = response;
				request_at (ConfigurationStep::ReadSerialNumberL, "SL");
				break;

			case ConfigurationStep::ReadSerialNumberL:
				_serial_number_bin += response;
				log() << "Serial number: " << xf::to_hex_string (_serial_number_bin) << std::endl;

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
				request_at (ConfigurationStep::SetChannel, "CH", { static_cast<uint8_t> (_channel) });
				break;

			case ConfigurationStep::SetChannel:
				request_at (ConfigurationStep::SetPersonalAreaNetworkID, "ID", { static_cast<uint8_t> (_pan_id >> 8), static_cast<uint8_t> (_pan_id) });
				break;

			case ConfigurationStep::SetPersonalAreaNetworkID:
				request_at (ConfigurationStep::SetDestinationAddressH, "DH", { 0x00, 0x00, 0x00, 0x00 });
				break;

			case ConfigurationStep::SetDestinationAddressH:
				request_at (ConfigurationStep::SetDestinationAddressL, "DL", { 0x00, 0x00, static_cast<uint8_t> (_remote_address >> 8), static_cast<uint8_t> (_remote_address) });
				break;

			case ConfigurationStep::SetDestinationAddressL:
				request_at (ConfigurationStep::SetLocalAddress, "MY", { static_cast<uint8_t> (_local_address >> 8), static_cast<uint8_t> (_local_address) });
				break;

			case ConfigurationStep::SetLocalAddress:
				if (_power_level)
				{
					request_at (ConfigurationStep::SetPowerLevel, "PL", { static_cast<uint8_t> (*_power_level) });
					break;
				}
				else
					_configuration_step = ConfigurationStep::SetPowerLevel;
				// Fall-through.

			case ConfigurationStep::SetPowerLevel:
				request_at (ConfigurationStep::SetCoordinatorMode, "CE", { 0x00 });
				break;

			case ConfigurationStep::SetCoordinatorMode:
				log() << "Modem configured." << std::endl;
				_configuration_step = ConfigurationStep::Configured;
				_serviceable.write (true);
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
XBee::make_frame (std::string const& data) const
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
	result.push_back (checksum);

	return result;
}


std::string
XBee::make_tx64_command (uint64_t address, std::string const& data) const
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
XBee::make_tx16_command (uint16_t address, std::string const& data) const
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
XBee::make_at_command (std::string const& at_command, uint8_t frame_id)
{
	std::string result;

	// API ID:
	result.push_back (static_cast<uint8_t> (SendAPI::ATCommand));
	// Frame ID for ACK (select ATFrameID just to get any response):
	result.push_back (frame_id);
	// Command:
	result += at_command;

	return result;
}


XBee::SendResult
XBee::send_frame (std::string const& frame, int& written)
{
	written = ::write (_device, frame.data(), frame.size());

	if (written == -1)
	{
		log() << "Write error " << strerror (errno) << std::endl;

		written = 0;
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return SendResult::Retry;
		else
			return SendResult::Failure;
	}
	else if (written < static_cast<int> (frame.size()))
	{
		log() << "Write buffer overrun." << std::endl;
		return SendResult::Retry;
	}

	return SendResult::Success;
}


bool
XBee::send_failed_with_retry()
{
	_write_failure_count++;
	bool should_restart = _write_failure_count > MaxWriteFailureCount || _output_buffer.size() > MaxOutputBufferSize;
	if (should_restart)
		_write_failure_count = 0;
	return should_restart;
}


std::vector<std::string>
XBee::packetize (std::string const& data, std::size_t size) const
{
	if (data.size() <= size)
		return { data };

	std::vector<std::string> result;
	for (std::size_t p = 0; p < data.size(); p += size)
		result.push_back (data.substr (p, size));
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
		std::string::size_type p = input.find (PacketDelimiter);

		if (p == std::string::npos)
		{
			input.clear();
			return false;
		}

		// Discard non-parseable data:
		input.erase (input.begin(), input.begin() + p);

		if (_input_errors.configured())
			_input_errors.write (*_input_errors + p);

		// Delimiter (1B) + packet size (2B) + data (1B) + checksum (1B) gives
		// at least 5 bytes:
		if (input.size() < 5)
			return false;

		// Packet size:
		uint32_t size = (static_cast<uint16_t> (input[1]) << 8) + input[2];
		if (input.size() < size + 4u) // delimiter, size, checksum = 4B
			return false;

		// Checksum:
		uint8_t checksum = 0;
		for (std::string::size_type i = 3; i < size + 4u; ++i)
			checksum += static_cast<uint8_t> (input[i]);
		if (checksum != 0xff)
		{
			log() << "Checksum invalid on input packet." << std::endl;
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
XBee::process_rx64_frame (std::string const& frame)
{
	if (_debug)
		debug() << ">> RX64 data: " << xf::to_hex_string (frame) << std::endl;

	// At least 11 bytes:
	if (frame.size() < 11)
		return;

	// 64-bit address:
	uint64_t address = 0;
	for (int b = 0; b < 8; ++b)
		address |= (static_cast<uint16_t> (frame[b]) << (8 * (7 - b)));

	// -RSSI dBm:
	int rssi = frame[8];
	rssi = -rssi;

	// Options:
	uint8_t options = frame[9];
	// We're not going to accept broadcast packets, sorry:
	if (options & 0x06)
	{
		log() << "Got packet with broadcast " << (options & 0x02 ? "address" : "pan") << ". Ignoring." << std::endl;
		return;
	}

	// Frame data:
	write_output_property (frame.substr (10));
	report_rssi (rssi);
}


void
XBee::process_rx16_frame (std::string const& frame)
{
	if (_debug)
		debug() << ">> RX16 data: " << xf::to_hex_string (frame) << std::endl;

	// At least 5 bytes:
	if (frame.size() < 5)
		return;

	// 16-bit address:
	uint16_t address = (static_cast<uint16_t> (frame[0]) << 8) | frame[1];
	// Address must match our peer's address:
	if (address != _remote_address)
	{
		log() << "Got packet from unknown address: " << xf::to_hex_string (frame.substr (0, 2)) << ". Ignoring." << std::endl;
		return;
	}

	// -RSSI dBm:
	int rssi = frame[2];
	rssi = -rssi;

	// Options:
	uint8_t options = frame[3];
	// We're not going to accept broadcast packets, sorry:
	if (options & 0x06)
	{
		log() << "Got packet with broadcast " << (options & 0x02 ? "address" : "pan") << ". Ignoring." << std::endl;
		return;
	}

	// Frame data:
	write_output_property (frame.substr (4));
	report_rssi (rssi);
}


void
XBee::process_modem_status_frame (std::string const& data)
{
	if (_debug)
		debug() << ">> Modem status: " << xf::to_hex_string (data) << std::endl;

	if (data.size() < 1)
		return;

	ModemStatus status = static_cast<ModemStatus> (data[0]);

	switch (status)
	{
		case ModemStatus::HardwareReset:
			log() << "Modem reported hardware reset." << std::endl;
			failure ("unexpected hardware reset");
			break;

		case ModemStatus::WatchdogReset:
			log() << "Modem reported watchdog reset." << std::endl;
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
			log() << "Associated." << std::endl;
			break;

		case ModemStatus::Disassociated:
			log() << "Disassociated." << std::endl;
			break;

		case ModemStatus::SynchronizationLost:
			log() << "Synchronization lost." << std::endl;
			break;

		case ModemStatus::CoordinatorRealignment:
			log() << "Coordinator realignment." << std::endl;
			break;

		case ModemStatus::CoordinatorStarted:
			log() << "Coordinator started." << std::endl;
			break;

		default:
			log() << "Modem reported unknown status: 0x"
				  << std::hex << static_cast<int> (data[0]) << std::dec << std::endl;
	}
}


void
XBee::process_at_response_frame (std::string const& frame)
{
	if (_debug)
		debug() << ">> AT status: " << xf::to_hex_string (frame) << std::endl;

	// Response must be at least 4 bytes long:
	if (frame.size() < 4)
		return;

	// AT command response:
	// 1B frame-ID:
	uint8_t frame_id = frame[0];

	// 2B AT command - skip
	std::string command = frame.substr (1, 2);

	// 1B status (0 = OK, 1 = ERROR, 2 = invalid command, 3 = invalid param)
	ATResponseStatus status = static_cast<ATResponseStatus> (frame[3]);

	// Data:
	std::string response_data = frame.substr (4);

	if (_debug)
	{
		std::ostream& os = debug();
		os << "Command result: " << command << " ";
		switch (status)
		{
			case ATResponseStatus::OK:					os << "OK"; break;
			case ATResponseStatus::ERROR:				os << "ERROR"; break;
			case ATResponseStatus::InvalidCommand:		os << "Invalid command"; break;
			case ATResponseStatus::InvalidParameter:	os << "Invalid parameter"; break;
			default:									os << "?"; break;
		}
		os << ", data: " << xf::to_hex_string (response_data) << std::endl;
	}

	// Response data: bytes
	if (frame_id == PeriodicPingFrameID)
		periodic_pong (status, response_data);
	else if (frame_id == ClearChannelFrameID)
		clear_channel_result (status, response_data);
	else
		configure_modem (frame_id, status, response_data);
}


void
XBee::write_output_property (std::string const& data)
{
	if (_receive.configured() && configured())
		_receive.write (data);
}


void
XBee::report_rssi (int dbm)
{
	// Restart timer:
	_rssi_timer->start();

	if (_rssi_dbm.configured())
	{
		Time now = xf::TimeHelper::now();
		_rssi_dbm.write (_rssi_smoother.process (static_cast<double> (dbm), now - _last_rssi_time));
		_last_rssi_time = now;
	}
}


void
XBee::ping (Time timeout)
{
	_pong_timer->stop();
	_pong_timer->setInterval (timeout.quantity<Millisecond>());
	_pong_timer->start();
}


void
XBee::pong()
{
	_pong_timer->stop();
}


void
XBee::periodic_pong (ATResponseStatus status, std::string const& data)
{
	if (status != ATResponseStatus::OK)
		failure ("check-alive packet status non-OK");
	else if (data.size() >= 1)
	{
		if (data[0] != 0x00)
			log() << "Association status: 0x" << std::hex << std::setw (2) << std::setfill ('0') << static_cast<int> (data[0]) << std::dec << std::endl;
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
XBee::clear_channel_result (ATResponseStatus status, std::string const& result)
{
	if (status == ATResponseStatus::OK && result.size() >= 2)
	{
		if (_cca_failures.configured())
		{
			uint16_t failures = (static_cast<uint16_t> (result[0]) >> 8) | result[1];
			_cca_failures.write (*_cca_failures + failures);
		}
	}
}


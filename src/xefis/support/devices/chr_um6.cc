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
#include <iomanip>
#include <map>

// Lib:
#include <boost/endian/conversion.hpp>

// Neutrino:
#include <neutrino/time_helper.h>

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "chr_um6.h"


namespace xf {

std::string
CHRUM6::Request::protocol_error_description() const
{
	switch (protocol_error())
	{
		case ProtocolError::None:
			return "none";
			break;

		case ProtocolError::Timeout:
			return "response timeout";
			break;

		case ProtocolError::BadChecksum:
			return "bad checksum";
			break;

		case ProtocolError::UnknownAddress:
			return "unknown address";
			break;

		case ProtocolError::InvalidBatchSize:
			return "invalid batch size";
			break;
	}

	return "";
}


std::string
CHRUM6::Command::firmware_version() const
{
	std::string version (4, ' ');
	uint32_t v = value();
	for (uint32_t i = 3u; i <= 3u; --i)
		version[3u - i] = static_cast<char> ((v >> (8u * i)) & 0xff);
	return version;
}


int16_t
CHRUM6::Read::value_upper16() const noexcept
{
	int16_t d = (value() >> 16) & 0xffff;
	boost::endian::little_to_native (d);
	return d;
}


int16_t
CHRUM6::Read::value_lower16() const noexcept
{
	int16_t d = (value() >> 0) & 0xffff;
	boost::endian::little_to_native (d);
	return d;
}


CHRUM6::CHRUM6 (SerialPort* serial_port, Logger const& logger):
	_serial_port (serial_port)
{
	_serial_port->set_data_ready_callback (std::bind (&CHRUM6::serial_ready, this));
	_serial_port->set_failure_callback (std::bind (&CHRUM6::serial_failure, this));

	_packet_reader = std::make_unique<PacketReader> (Blob { 's', 'n', 'p' }, std::bind (&CHRUM6::parse_packet, this));
	_packet_reader->set_minimum_packet_size (7);
	_packet_reader->set_buffer_capacity (4096);

	set_logger (logger);
}


void
CHRUM6::set_logger (Logger const& logger)
{
	_logger = logger.with_scope (kLoggerScope);
	_serial_port->set_logger (_logger);
}


CHRUM6::Read
CHRUM6::read (ConfigurationAddress address, ReadCallback callback)
{
	_requests.push (std::make_unique<Read> (address, callback));
	Read ret = *dynamic_cast<Read*> (_requests.back().get());
	process_queue();
	return ret;
}


CHRUM6::Read
CHRUM6::read (DataAddress address, ReadCallback callback)
{
	_requests.push (std::make_unique<Read> (address, callback));
	Read ret = *dynamic_cast<Read*> (_requests.back().get());
	process_queue();
	return ret;
}


CHRUM6::Write
CHRUM6::write (ConfigurationAddress address, uint32_t value, WriteCallback callback)
{
	_requests.push (std::make_unique<Write> (address, value, callback));
	Write ret = *dynamic_cast<Write*> (_requests.back().get());
	process_queue();
	return ret;
}


CHRUM6::Write
CHRUM6::write (ConfigurationAddress address, float const value, WriteCallback callback)
{
	uint32_t i;
	static_assert (sizeof (value) == sizeof (i));
	std::memcpy (&i, &value, sizeof (i));
	return write (address, i, callback);
}


CHRUM6::Command
CHRUM6::command (CommandAddress address, CommandCallback callback)
{
	_requests.push (std::make_unique<Command> (address, callback));
	Command ret = *dynamic_cast<Command*> (_requests.back().get());
	process_queue();
	return ret;
}


uint32_t
CHRUM6::sample_rate_setting (si::Frequency const frequency) noexcept
{
	// Use formula from the spec: freq = (280/255) * sample_rate + 20.
	uint32_t x = (std::max ((frequency - 20_Hz), 0.1_Hz) / (280.0 / 255.0)).in<si::Hertz>();
	return xf::clamped (x, 0u, 255u);
}


uint32_t
CHRUM6::bits_for_baud_rate (unsigned int baud_rate)
{
	static std::map<unsigned int, uint32_t> const baud_rates_map {
		{ 9600u, 0 },
		{ 14400u, 1 },
		{ 19200u, 2 },
		{ 38400u, 3 },
		{ 57600u, 4 },
		{ 115200u, 5 },
	};

	auto c = baud_rates_map.find (baud_rate);

	if (c == baud_rates_map.end())
		c = baud_rates_map.upper_bound (baud_rate);

	if (c == baud_rates_map.end())
		return 0;

	return c->second;
}


void
CHRUM6::serial_ready()
{
	_packet_reader->feed (_serial_port->input_buffer());
	_serial_port->input_buffer().clear();
}


void
CHRUM6::serial_failure()
{
	if (_communication_failure_callback)
		_communication_failure_callback();
}


void
CHRUM6::process_queue()
{
	if (!_requests.empty())
	{
		_current_req = std::move (_requests.front());
		_requests.pop();

		send_packet (_current_req->data()->packet_data);
	}
}


Blob
CHRUM6::make_packet (uint32_t address, bool write_operation, uint32_t data)
{
	Blob result = { 's', 'n', 'p' };

	uint8_t packet_type = 0;

	// Bit 7: write (1) or read (0)
	if (write_operation)
		packet_type |= 1u << 7;

	// Bit 6: batch operation? No.

	// Bits 5-2: size of batch operation: 0.

	// Bit 1 is reserved

	// Bit 0: command failed (unused here).

	result.push_back (packet_type);
	result.push_back (static_cast<uint8_t> (address));

	if (write_operation)
	{
		boost::endian::native_to_little (data);
		result.push_back ((data >> 24) & 0xff);
		result.push_back ((data >> 16) & 0xff);
		result.push_back ((data >> 8) & 0xff);
		result.push_back ((data >> 0) & 0xff);
	}

	// Compute checksum:
	uint16_t checksum = 0;
	for (uint8_t c: result)
		checksum += c;
	boost::endian::native_to_little (checksum);

	result.push_back ((checksum >> 8) & 0xff);
	result.push_back ((checksum >> 0) & 0xff);

	return result;
}


std::size_t
CHRUM6::parse_packet()
{
	Blob& packet = _packet_reader->buffer();

	// Packet type byte:
	uint8_t packet_type = packet[3];

	bool has_data = (packet_type >> 7) & 0x01;
	bool failed = packet_type & 0x01;
	bool is_batch = (packet_type >> 6) & 0x01;
	std::size_t data_words = (packet_type >> 2) & 0x0f;
	if (has_data && !is_batch)
		data_words = 1;

	// Ensure that there's enough data in the input buffer:
	std::size_t required_size = 7 + data_words * 4;
	if (packet.size() < required_size)
		return 0;

	// Address byte:
	uint32_t address = static_cast<uint8_t> (packet[4]);

	// Checksum:
	uint16_t checksum = 0;
	for (std::size_t i = 0; i < required_size - 2; ++i)
		checksum += static_cast<uint8_t> (packet[i]);

	uint16_t cmp_checksum = (static_cast<uint16_t> (packet[required_size - 2]) << 8)
						  + (static_cast<uint16_t> (packet[required_size - 1]) << 0);
	boost::endian::little_to_native (cmp_checksum);

	// Skip this packet on error:
	if (checksum != cmp_checksum)
	{
		if (_communication_failure_callback)
			_communication_failure_callback();
		return required_size;
	}

	if (!has_data)
		process_packet (address, failed, false, 0);
	else
	{
		// Read words:
		for (std::size_t w = 0; w < data_words; ++w)
		{
			uint32_t data = (static_cast<uint32_t> (packet[4 * w + 5]) << 24)
						  + (static_cast<uint32_t> (packet[4 * w + 6]) << 16)
						  + (static_cast<uint32_t> (packet[4 * w + 7]) << 8)
						  + (static_cast<uint32_t> (packet[4 * w + 8]) << 0);
			uint32_t relative_address = static_cast<uint8_t> (address) + w;
			if (has_data)
				process_packet (relative_address, failed, true, data);
		}
	}

	return required_size;
}


void
CHRUM6::send_packet (Blob const& packet)
{
	_serial_port->write (packet);
}


void
CHRUM6::process_packet (uint32_t address, bool failed, bool, uint32_t data)
{
	if (_alive_check_callback)
		_alive_check_callback();

	si::Time const now = TimeHelper::now();

	if (failed)
	{
		_logger << "Command 0x"
				<< std::hex << std::setfill ('0') << std::setprecision (2) << static_cast<int> (address)
				<< std::dec << " failed." << std::endl;
	}

	if (address == static_cast<uint32_t> (ProtocolError::BadChecksum) ||
		address == static_cast<uint32_t> (ProtocolError::UnknownAddress) ||
		address == static_cast<uint32_t> (ProtocolError::InvalidBatchSize))
	{
		if (_current_req)
		{
			if (_auto_retry && address == static_cast<uint32_t> (ProtocolError::BadChecksum))
			{
				_current_req->data()->retries++;
				_requests.push (std::move (_current_req));
				process_queue();
			}
			else
			{
				// Handle last request as failed with given protocol error.
				_current_req->data()->finish_timestamp = now;
				_current_req->data()->finished = true;
				_current_req->data()->success = false;
				_current_req->data()->protocol_error = static_cast<ProtocolError> (address);
				_current_req->make_callback();
			}
		}
		else
		{
			// Spurious event?
			_logger << "Got spurious protocol error packet (" << packet_name (address) << ")." << std::endl;
		}
	}
	else if (_current_req && _current_req->address() == address)
	{
		_current_req->data()->finish_timestamp = now;
		_current_req->data()->finished = true;
		_current_req->data()->success = !failed;
		_current_req->data()->value = data;
		_current_req->make_callback();
	}
	else if (_incoming_messages_callback)
	{
		Read req (static_cast<DataAddress> (address));
		req.data()->address = address;
		req.data()->start_timestamp = now;
		req.data()->finish_timestamp = now;
		req.data()->finished = true;
		req.data()->success = !failed;
		req.data()->value = data;

		_incoming_messages_callback (req);
	}
}


const char*
CHRUM6::packet_name (uint32_t address) noexcept
{
	switch (address)
	{
#define CONFIG_CASE(x) case static_cast<uint32_t> (xf::CHRUM6::ConfigurationAddress::x): return #x;
#define DATA_CASE(x) case static_cast<uint32_t> (xf::CHRUM6::DataAddress::x): return #x;
#define COMMAND_CASE(x) case static_cast<uint32_t> (xf::CHRUM6::CommandAddress::x): return #x;
#define ERROR_CASE(x) case static_cast<uint32_t> (xf::CHRUM6::ProtocolError::x): return #x;
		CONFIG_CASE (Communication)
		CONFIG_CASE (MiscConfig)
		CONFIG_CASE (MagRefX)
		CONFIG_CASE (MagRefY)
		CONFIG_CASE (MagRefZ)
		CONFIG_CASE (AccelRefX)
		CONFIG_CASE (AccelRefY)
		CONFIG_CASE (AccelRefZ)
		CONFIG_CASE (EKFMagVariance)
		CONFIG_CASE (EKFAccelVariance)
		CONFIG_CASE (EKFProcessVariance)
		CONFIG_CASE (GyroBiasXY)
		CONFIG_CASE (GyroBiasZ)
		CONFIG_CASE (AccelBiasXY)
		CONFIG_CASE (AccelBiasZ)
		CONFIG_CASE (MagBiasXY)
		CONFIG_CASE (MagBiasZ)
		CONFIG_CASE (AccelCal00)
		CONFIG_CASE (AccelCal01)
		CONFIG_CASE (AccelCal02)
		CONFIG_CASE (AccelCal10)
		CONFIG_CASE (AccelCal11)
		CONFIG_CASE (AccelCal12)
		CONFIG_CASE (AccelCal20)
		CONFIG_CASE (AccelCal21)
		CONFIG_CASE (AccelCal22)
		CONFIG_CASE (GyroCal00)
		CONFIG_CASE (GyroCal01)
		CONFIG_CASE (GyroCal02)
		CONFIG_CASE (GyroCal10)
		CONFIG_CASE (GyroCal11)
		CONFIG_CASE (GyroCal12)
		CONFIG_CASE (GyroCal20)
		CONFIG_CASE (GyroCal21)
		CONFIG_CASE (GyroCal22)
		CONFIG_CASE (MagCal00)
		CONFIG_CASE (MagCal01)
		CONFIG_CASE (MagCal02)
		CONFIG_CASE (MagCal10)
		CONFIG_CASE (MagCal11)
		CONFIG_CASE (MagCal12)
		CONFIG_CASE (MagCal20)
		CONFIG_CASE (MagCal21)
		CONFIG_CASE (MagCal22)
		CONFIG_CASE (GyroXBias0)
		CONFIG_CASE (GyroXBias1)
		CONFIG_CASE (GyroXBias2)
		CONFIG_CASE (GyroXBias3)
		CONFIG_CASE (GyroYBias0)
		CONFIG_CASE (GyroYBias1)
		CONFIG_CASE (GyroYBias2)
		CONFIG_CASE (GyroYBias3)
		CONFIG_CASE (GyroZBias0)
		CONFIG_CASE (GyroZBias1)
		CONFIG_CASE (GyroZBias2)
		CONFIG_CASE (GyroZBias3)
		CONFIG_CASE (GPSHomeLat)
		CONFIG_CASE (GPSHomeLon)
		CONFIG_CASE (GPSHomeAltitude)
		DATA_CASE (Status)
		DATA_CASE (GyroRawXY)
		DATA_CASE (GyroRawZ)
		DATA_CASE (AccelRawXY)
		DATA_CASE (AccelRawZ)
		DATA_CASE (MagRawXY)
		DATA_CASE (MagRawZ)
		DATA_CASE (GyroProcXY)
		DATA_CASE (GyroProcZ)
		DATA_CASE (AccelProcXY)
		DATA_CASE (AccelProcZ)
		DATA_CASE (MagProcXY)
		DATA_CASE (MagProcZ)
		DATA_CASE (EulerPhiTheta)
		DATA_CASE (EulerPsi)
		DATA_CASE (QuatAB)
		DATA_CASE (QuatCD)
		DATA_CASE (ErrorCov00)
		DATA_CASE (ErrorCov01)
		DATA_CASE (ErrorCov02)
		DATA_CASE (ErrorCov03)
		DATA_CASE (ErrorCov10)
		DATA_CASE (ErrorCov11)
		DATA_CASE (ErrorCov12)
		DATA_CASE (ErrorCov13)
		DATA_CASE (ErrorCov20)
		DATA_CASE (ErrorCov21)
		DATA_CASE (ErrorCov22)
		DATA_CASE (ErrorCov23)
		DATA_CASE (ErrorCov30)
		DATA_CASE (ErrorCov31)
		DATA_CASE (ErrorCov32)
		DATA_CASE (ErrorCov33)
		DATA_CASE (Temperature)
		DATA_CASE (GPSLongitude)
		DATA_CASE (GPSLatitude)
		DATA_CASE (GPSAltitude)
		DATA_CASE (GPSPositionN)
		DATA_CASE (GPSPositionE)
		DATA_CASE (GPSPositionH)
		DATA_CASE (GPSCourseSpeed)
		DATA_CASE (GPSSatSummary)
		DATA_CASE (GPSSat12)
		DATA_CASE (GPSSat34)
		DATA_CASE (GPSSat56)
		DATA_CASE (GPSSat78)
		DATA_CASE (GPSSat9A)
		DATA_CASE (GPSSatBC)
		COMMAND_CASE (GetFWVersion)
		COMMAND_CASE (FlashCommit)
		COMMAND_CASE (ZeroGyros)
		COMMAND_CASE (ResetEKF)
		COMMAND_CASE (GetData)
		COMMAND_CASE (SetAccelRef)
		COMMAND_CASE (SetMagRef)
		COMMAND_CASE (ResetToFactory)
		COMMAND_CASE (GPSSetHomePosition)
		ERROR_CASE (BadChecksum)
		ERROR_CASE (UnknownAddress)
		ERROR_CASE (InvalidBatchSize)
#undef CONFIG_CASE
#undef DATA_CASE
#undef COMMAND_CASE
#undef ERROR_CASE
	}

	return "(unknown)";
}

} // namespace xf


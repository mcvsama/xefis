/* vim:ts=4
 *
 * Copyleft 2012…2013  Michał Gawron
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
#include <memory>
#include <ctime>
#include <functional>
#include <iomanip>

// System:
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

// Lib:
#include <boost/endian/conversion.hpp>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/string.h>
#include <xefis/utility/numeric.h>

// Local:
#include "chr_um6.h"


XEFIS_REGISTER_MODULE_CLASS ("io/chr-um6", CHRUM6);


CHRUM6::CHRUM6 (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config),
	_serial_port (std::bind (&CHRUM6::serial_ready, this), std::bind (&CHRUM6::serial_failure, this)),
	_packet_reader ("snp", std::bind (&CHRUM6::parse_packet, this))
{
	std::string device_path;
	std::string baud_rate = "115200";

	for (QDomElement& e: config)
	{
		if (e == "settings")
		{
			parse_settings (e, {
				{ "serial.device", device_path, true },
				{ "serial.baud-rate", baud_rate, true },
				{ "sample-rate", _sample_rate, true },
				{ "ekf.process-variance", _ekf_process_variance, false },
			});
		}
		else if (e == "properties")
		{
			parse_properties (e, {
				{ "serviceable", _serviceable, true },
				{ "failures", _failures, false },
				{ "internal-temperature", _internal_temperature, false },
				{ "orientation.pitch", _orientation_pitch, true },
				{ "orientation.roll", _orientation_roll, true },
				{ "orientation.magnetic-heading", _orientation_magnetic_heading, true },
				{ "acceleration.x", _acceleration_x, false },
				{ "acceleration.y", _acceleration_y, false },
				{ "acceleration.z", _acceleration_z, false },
				{ "rotation.x", _rotation_x, false },
				{ "rotation.y", _rotation_y, false },
				{ "rotation.z", _rotation_z, false },
				{ "magnetic.x", _magnetic_x, false },
				{ "magnetic.y", _magnetic_y, false },
				{ "magnetic.z", _magnetic_z, false },
			});
		}
	}

	_serial_port.set_logger (log());
	_serial_port.set_device_path (device_path);
	_serial_port.set_baud_rate (baud_rate);
	_serial_port.set_data_bits (8);
	_serial_port.set_stop_bits (1);
	_serial_port.set_parity_bit (Xefis::SerialPort::Parity::None);

	_packet_reader.set_minimum_packet_size (11);
	_packet_reader.set_buffer_capacity (4096);

	_restart_timer = new QTimer (this);
	_restart_timer->setInterval (RestartDelay.ms());
	_restart_timer->setSingleShot (true);
	QObject::connect (_restart_timer, SIGNAL (timeout()), this, SLOT (open_device()));

	_status_check_timer = new QTimer (this);
	_status_check_timer->setInterval (StatusCheckInterval.ms());
	_status_check_timer->setSingleShot (false);
	QObject::connect (_status_check_timer, SIGNAL (timeout()), this, SLOT (status_check()));

	_alive_check_timer = new QTimer (this);
	_alive_check_timer->setInterval (AliveCheckInterval.ms());
	_alive_check_timer->setSingleShot (false);
	QObject::connect (_alive_check_timer, SIGNAL (timeout()), this, SLOT (alive_check_failed()));

	_serviceable.set_default (false);
	_failures.set_default (0);

	open_device();
}


void
CHRUM6::open_device()
{
	try {
		_alive_check_timer->start();

		reset();
		if (_serial_port.open())
			initialize();
		else
			restart();
	}
	catch (...)
	{
		failure ("exception in open_device()");
	}
}


void
CHRUM6::failure (std::string const& reason)
{
	log() << "Fatal: failure detected" << (reason.empty() ? "" : ": " + reason) << ", closing device " << _serial_port.device_path() << std::endl;
	if (_failures.configured())
		_failures.write (*_failures + 1);
	_alive_check_timer->stop();
	_status_check_timer->stop();

	restart();
}


void
CHRUM6::alive_check_failed()
{
	failure ("alive check failed");
}


void
CHRUM6::restart()
{
	reset();
	_restart_timer->start();
}


void
CHRUM6::status_check()
{
}


void
CHRUM6::initialize()
{
	uint32_t data = 0;
	data |= static_cast<uint32_t> (CommunicationRegister::BEN);
	data |= static_cast<uint32_t> (CommunicationRegister::EU);
	data |= static_cast<uint32_t> (CommunicationRegister::AP);
	data |= static_cast<uint32_t> (CommunicationRegister::GP);
	data |= static_cast<uint32_t> (CommunicationRegister::MP);
	data |= 0x500; // Baud-rate: 115200
	data |= sample_rate_setting (_sample_rate);
	write_register (Address::Communication, data);

	data = 0;
	data |= static_cast<uint32_t> (MiscConfigRegister::MUE);
	data |= static_cast<uint32_t> (MiscConfigRegister::AUE);
	data |= static_cast<uint32_t> (MiscConfigRegister::CAL);
	data |= static_cast<uint32_t> (MiscConfigRegister::QUAT);
	write_register (Address::MiscConfig, data);

	// Read process-variance value:
	read_register (Address::EKFProcessVariance);

	align();
}


void
CHRUM6::align()
{
	if (has_setting ("ekf.process-variance"))
		write_register (Address::EKFProcessVariance, _ekf_process_variance);

	if (_aligned)
		return;

	// Log firmware version:
	issue_command (Address::GetFWVersion);
	// Accelerators realign:
	issue_command (Address::SetAccelRef);
	// Magnetic realign:
	issue_command (Address::SetMagRef);
	// Send ZERO_GYROS command and wait for completion
	issue_command (Address::ZeroGyros);

	_aligned = true;
}


void
CHRUM6::reset()
{
	_serviceable.write (false);
	_orientation_pitch.set_nil();
	_orientation_roll.set_nil();
	_orientation_magnetic_heading.set_nil();
	_acceleration_x.set_nil();
	_acceleration_y.set_nil();
	_acceleration_z.set_nil();
	_rotation_x.set_nil();
	_rotation_y.set_nil();
	_rotation_z.set_nil();
	_magnetic_x.set_nil();
	_magnetic_y.set_nil();
	_magnetic_z.set_nil();
}


void
CHRUM6::serial_ready()
{
	_alive_check_timer->start();

	_packet_reader.feed (_serial_port.input_buffer());
	_serial_port.input_buffer().clear();
}


void
CHRUM6::serial_failure()
{
	failure ("serial port failed");
}


void
CHRUM6::issue_command (Address address)
{
	_serial_port.write (make_packet (address, Operation::Command));
}


void
CHRUM6::write_register (Address address, uint32_t data)
{
	_serial_port.write (make_packet (address, Operation::Write, data));
}


void
CHRUM6::write_register (Address address, float value)
{
	union { float f; uint32_t i; } u = { value };
	write_register (address, u.i);
}


void
CHRUM6::read_register (Address address)
{
	_serial_port.write (make_packet (address, Operation::Read));
}


std::string
CHRUM6::make_packet (Address address, Operation operation, uint32_t data)
{
	std::string result = "snp";

	uint8_t packet_type = 0;

	// Bit 7: write (1) or read (0)
	if (operation == Operation::Write)
		packet_type |= 1u << 7;

	// Bit 6: batch operation? No.

	// Bits 5-2: size of batch operation: 0.

	// Bit 1 is reserved

	// Bit 0: command failed (unused here).

	result += packet_type;
	result += static_cast<uint8_t> (address);

	if (operation == Operation::Write)
	{
		boost::endian::native_to_little (data);
		result += (data >> 24) & 0xff;
		result += (data >> 16) & 0xff;
		result += (data >> 8) & 0xff;
		result += (data >> 0) & 0xff;
	}

	// Compute checksum:
	uint16_t checksum = 0;
	for (uint8_t c: result)
		checksum += c;
	boost::endian::native_to_little (checksum);

	result += (checksum >> 8) & 0xff;
	result += (checksum >> 0) & 0xff;

log() << "PACKET " << Xefis::to_hex_string (result) << "\n";
	return result;
}


std::size_t
CHRUM6::parse_packet()
{
	std::string& packet = _packet_reader.buffer();

	// Packet type byte:
	uint8_t packet_type = packet[3];

	bool has_data = (packet_type >> 7) & 1;
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
	Address address = static_cast<Address> (packet[4]);

	// Checksum:
	uint16_t checksum = 0;
	for (std::size_t i = 0; i < required_size - 2; ++i)
		checksum += static_cast<uint8_t> (packet[i]);

	uint16_t cmp_checksum = (static_cast<uint16_t> (packet[required_size - 2]) << 8)
						  + (static_cast<uint16_t> (packet[required_size - 1]) << 0);
	boost::endian::little_to_native (cmp_checksum);

	// Skip this packet on error:
	if (checksum != cmp_checksum)
		return required_size;

	if (!has_data)
		command_complete (address, failed);
	else
	{
		// Read words:
		for (std::size_t w = 0; w < data_words; ++w)
		{
			uint32_t data = (static_cast<uint32_t> (packet[4 * w + 5]) << 24)
						  + (static_cast<uint32_t> (packet[4 * w + 6]) << 16)
						  + (static_cast<uint32_t> (packet[4 * w + 7]) << 8)
						  + (static_cast<uint32_t> (packet[4 * w + 8]) << 0);
			Address relative_address = static_cast<Address> (static_cast<uint8_t> (address) + w);
			if (has_data)
				process_data (relative_address, failed, data);
		}

		if (_signal_data_updated)
		{
			signal_data_updated();
			_signal_data_updated = false;
		}
	}

	return required_size;
}


void
CHRUM6::process_data (Address address, bool failed, uint32_t data)
{
	if (failed)
		return;

	switch (address)
	{
		case Address::GetFWVersion:
		{
			std::string version (4, ' ');
			for (int i = 3; i >= 0; --i)
				version[3 - i] = static_cast<char> ((data >> (8 * i)) & 0xff);
			log() << "Firmware version: " << version << std::endl;
			break;
		}

		case Address::EKFProcessVariance:
			log() << "EKF process variance: " << to_float (data) << std::endl;
			break;

		case Address::Temperature:
		{
			if (_internal_temperature.configured())
				_internal_temperature.write (1_degC * to_float (data));
			_signal_data_updated = false;
			break;
		}

		case Address::EulerPhiTheta:
		{
			// Pitch (phi) and Roll (theta):
			const float factor = 0.0109863;
			float phi = factor * upper16 (data);
			float theta = factor * lower16 (data);
			_orientation_pitch.write (1_deg * theta);
			_orientation_roll.write (1_deg * phi);
			_signal_data_updated = true;
			break;
		}

		case Address::EulerPsi:
		{
			// Heading:
			const float factor = 0.0109863;
			float psi = Xefis::floored_mod (factor * upper16 (data), 0.f, 360.f);
			_orientation_magnetic_heading.write (1_deg * psi);
			_signal_data_updated = true;
			break;
		}

		case Address::AccelProcXY:
		{
			if (_acceleration_x.configured() || _acceleration_y.configured())
			{
				const float factor = 0.000183105;
				float x = factor * upper16 (data);
				float y = factor * lower16 (data);
				if (_acceleration_x.configured())
					_acceleration_x.write (x);
				if (_acceleration_y.configured())
					_acceleration_y.write (y);
				_signal_data_updated = true;
			}
			break;
		}

		case Address::AccelProcZ:
		{
			if (_acceleration_z.configured())
			{
				const float factor = 0.000183105;
				float z = factor * upper16 (data);
				_acceleration_z.write (z);
				_signal_data_updated = true;
			}
			break;
		}

		case Address::GyroProcXY:
		{
			if (_rotation_x.configured() || _rotation_y.configured())
			{
				const float factor = 0.0610352;
				float x = factor * upper16 (data);
				float y = factor * lower16 (data);
				if (_rotation_x.configured())
					_rotation_x.write (1_deg * x);
				if (_rotation_y.configured())
					_rotation_y.write (1_deg * y);
				_signal_data_updated = true;
			}
			break;
		}

		case Address::GyroProcZ:
		{
			if (_rotation_z.configured())
			{
				const float factor = 0.0610352;
				float z = factor * upper16 (data);
				_rotation_z.write (1_deg * z);
				_signal_data_updated = true;
			}
			break;
		}

		case Address::MagProcXY:
		{
			if (_magnetic_x.configured() || _magnetic_y.configured())
			{
				const float factor = 0.000305176;
				float x = factor * upper16 (data);
				float y = factor * lower16 (data);
				if (_magnetic_x.configured())
					_magnetic_x.write (x);
				if (_magnetic_y.configured())
					_magnetic_y.write (y);
				_signal_data_updated = true;
			}
			break;
		}

		case Address::MagProcZ:
		{
			if (_magnetic_z.configured())
			{
				const float factor = 0.000305176;
				float z = factor * upper16 (data);
				_magnetic_z.write (z);
				_signal_data_updated = true;
			}
			break;
		}

		default:
			log() << "Unhandled message 0x"
				<< std::hex << std::setfill ('0') << std::setprecision (2) << static_cast<int> (address)
				<< std::dec << std::endl;
			break;
	}
}


void
CHRUM6::command_complete (Address address, bool failed)
{
	switch (address)
	{
		case Address::ZeroGyros:
			// Reset Extended Kalmann Filter:
			issue_command (Address::ResetEKF);

			_serviceable.write (!failed);

			if (failed)
				log() << "Failed to zero gyros." << std::endl;
			else
			{
				log() << "Gyros zeroed." << std::endl;
				_status_check_timer->start();
			}
			break;

		case Address::BadChecksum:
			log() << "Command failed: bad checksum." << std::endl;
			break;

		case Address::UnknownAddress:
			log() << "Command failed: unknown address." << std::endl;
			break;

		case Address::InvalidBatchSize:
			log() << "Command failed: invalid batch size." << std::endl;
			break;

		default:
			if (failed)
			{
				log() << "Command 0x"
					<< std::hex << std::setfill ('0') << std::setprecision (2) << static_cast<int> (address)
					<< std::dec << " failed." << std::endl;
			}
			break;
	}
}


uint32_t
CHRUM6::sample_rate_setting (Frequency frequency) noexcept
{
	// Use formula from the spec: freq = (280/255) * sample_rate + 20.
	int32_t x = ((frequency - 20_Hz) / (280.0 / 255.0)).Hz();
	return Xefis::limit (x, 0, 255);
}


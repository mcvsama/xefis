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
	_packet_reader ({ 's', 'n', 'p' }, std::bind (&CHRUM6::parse_packet, this))
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
				{ "caution", _caution, false },
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

	_packet_reader.set_minimum_packet_size (7);
	_packet_reader.set_buffer_capacity (4096);

	_restart_timer = new QTimer (this);
	_restart_timer->setInterval (RestartDelay.ms());
	_restart_timer->setSingleShot (true);
	QObject::connect (_restart_timer, SIGNAL (timeout()), this, SLOT (open_device()));

	_alive_check_timer = new QTimer (this);
	_alive_check_timer->setInterval (AliveCheckInterval.ms());
	_alive_check_timer->setSingleShot (false);
	QObject::connect (_alive_check_timer, SIGNAL (timeout()), this, SLOT (alive_check_failed()));

	_status_check_timer = new QTimer (this);
	_status_check_timer->setInterval (StatusCheckInterval.ms());
	_status_check_timer->setSingleShot (false);
	QObject::connect (_status_check_timer, SIGNAL (timeout()), this, SLOT (status_check()));

	_initialization_timer = new QTimer (this);
	_initialization_timer->setInterval (InitializationDelay.ms());
	_initialization_timer->setSingleShot (true);
	QObject::connect (_initialization_timer, SIGNAL (timeout()), this, SLOT (initialization_timeout()));

	_serviceable.set_default (false);
	_caution.set_default (false);
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
	_failure_count++;

	restart();
}


void
CHRUM6::alive_check_failed()
{
	failure ("alive check failed");
}


void
CHRUM6::initialization_timeout()
{
	failure ("initialization timeout");
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
	read_register (Address::Status);
}


void
CHRUM6::initialize()
{
	_stage = Stage::Initialize;
	_initialization_step = 0;

	next_initialization_step();
}


void
CHRUM6::next_initialization_step()
{
	if (_stage == Stage::Initialize)
	{
		switch (_initialization_step++)
		{
			case 0:
			{
				log() << "Begin initialization." << std::endl;
				_initialization_timer->start();

				uint32_t data = 0;
				data |= static_cast<uint32_t> (CommunicationRegister::BEN);
				data |= static_cast<uint32_t> (CommunicationRegister::EU);
				data |= static_cast<uint32_t> (CommunicationRegister::AP);
				data |= static_cast<uint32_t> (CommunicationRegister::GP);
				data |= static_cast<uint32_t> (CommunicationRegister::MP);
				data |= static_cast<uint32_t> (CommunicationRegister::TMP);
				data |= 0x500; // TODO according to setting, 0x500 is Baud-rate: 115200
				data |= sample_rate_setting (_sample_rate);
				write_register (Address::Communication, data);
				break;
			}

			case 1:
			{
				uint32_t data = 0;
				data |= static_cast<uint32_t> (MiscConfigRegister::MUE);
				data |= static_cast<uint32_t> (MiscConfigRegister::AUE);
				data |= static_cast<uint32_t> (MiscConfigRegister::CAL);
				data |= static_cast<uint32_t> (MiscConfigRegister::QUAT);
				write_register (Address::MiscConfig, data);
				break;
			}

			case 2:
				// Log firmware version:
				issue_command (Address::GetFWVersion);
				break;

			case 3:
				// Log current process-variance value:
				read_register (Address::EKFProcessVariance);
				break;

			case 4:
				if (has_setting ("ekf.process-variance"))
				{
					write_register (Address::EKFProcessVariance, _ekf_process_variance);
					break;
				}
				else
					++_initialization_step;
				// Fall-over:
			case 5:
				// Reset Extended Kalmann Filter:
				issue_command (Address::ResetEKF);
				break;

			case 6:
				// Accelerators realign:
				issue_command (Address::SetAccelRef);
				break;

			case 7:
				// Magnetic realign:
				issue_command (Address::SetMagRef);
				break;

			case 8:
				if (_gyro_bias_xy && _gyro_bias_z)
				{
					log() << "Setting previously acquired gyro biases." << std::endl;
					write_register (Address::GyroBiasXY, *_gyro_bias_xy);
					write_register (Address::GyroBiasZ, *_gyro_bias_z);
					next_initialization_step();
				}
				else
				{
					// Send ZERO_GYROS command and wait for completion:
					issue_command (Address::ZeroGyros);
				}
				break;

			case 9:
				_stage = Stage::Run;
				initialization_complete();
				break;
		}
	}
}


void
CHRUM6::repeat_initialization_step()
{
	--_initialization_step;
	next_initialization_step();
}


void
CHRUM6::initialization_complete()
{
	log() << "Initialization complete." << std::endl;
	_initialization_timer->stop();
	_serviceable.write (true);
	_status_check_timer->start();
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

	_stage = Stage::Initialize;
	_initialization_step = 0;
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
	send_packet (make_packet (address, Operation::Command));
}


void
CHRUM6::write_register (Address address, uint32_t data)
{
	send_packet (make_packet (address, Operation::Write, data));
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
	send_packet (make_packet (address, Operation::Read));
}


void
CHRUM6::send_packet (Blob const& packet)
{
	_serial_port.write (packet);
}


CHRUM6::Blob
CHRUM6::make_packet (Address address, Operation operation, uint32_t data)
{
	Blob result = { 's', 'n', 'p' };

	uint8_t packet_type = 0;

	// Bit 7: write (1) or read (0)
	if (operation == Operation::Write)
		packet_type |= 1u << 7;

	// Bit 6: batch operation? No.

	// Bits 5-2: size of batch operation: 0.

	// Bit 1 is reserved

	// Bit 0: command failed (unused here).

	result.push_back (packet_type);
	result.push_back (static_cast<uint8_t> (address));

	if (operation == Operation::Write)
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
	Blob& packet = _packet_reader.buffer();

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
	Address address = static_cast<Address> (static_cast<uint8_t> (packet[4]));

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
		log() << "Received packet with wrong checksum 0x" << std::hex << checksum << ", declared 0x" << cmp_checksum << std::dec << std::endl;
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
			Address relative_address = static_cast<Address> (static_cast<uint8_t> (address) + w);
			if (has_data)
				process_packet (relative_address, failed, true, data);
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
CHRUM6::process_packet (Address address, bool failed, bool has_data, uint32_t data)
{
	if (failed)
	{
		log() << "Command 0x"
			<< std::hex << std::setfill ('0') << std::setprecision (2) << static_cast<int> (address)
			<< std::dec << " failed." << std::endl;
	}

	switch (address)
	{
		case Address::Status:
			status_verify (data);
			break;

		case Address::GyroBiasXY:
			if (!failed)
			{
				if (!_gyro_bias_xy)
				{
					_gyro_bias_xy = data;
					log() << "Gyro bias X: " << upper16 (data) << std::endl;
					log() << "Gyro bias Y: " << lower16 (data) << std::endl;
				}
			}
			break;

		case Address::GyroBiasZ:
			if (!failed)
			{
				if (!_gyro_bias_z)
				{
					_gyro_bias_z = data;
					log() << "Gyro bias Z: " << upper16 (data) << std::endl;
				}
			}
			break;

		case Address::Temperature:
		{
			if (failed)
				break;
			if (_internal_temperature.configured())
				_internal_temperature.write (1_K * (273.15 + to_float (data)));
			_signal_data_updated = true;
			break;
		}

		case Address::EulerPhiTheta:
		{
			if (failed || !*_serviceable)
				break;
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
			if (failed || !*_serviceable)
				break;
			// Heading:
			const float factor = 0.0109863;
			float psi = Xefis::floored_mod (factor * upper16 (data), 0.f, 360.f);
			_orientation_magnetic_heading.write (1_deg * psi);
			_signal_data_updated = true;
			break;
		}

		case Address::AccelProcXY:
		{
			if (failed)
				break;
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
			if (failed)
				break;
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
			if (failed)
				break;
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
			if (failed)
				break;
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
			if (failed)
				break;
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
			if (failed)
				break;
			if (_magnetic_z.configured())
			{
				const float factor = 0.000305176;
				float z = factor * upper16 (data);
				_magnetic_z.write (z);
				_signal_data_updated = true;
			}
			break;
		}

		case Address::Communication:
		case Address::MiscConfig:
			next_initialization_step();
			break;

		/*
		 * Command registers.
		 */

		case Address::GetFWVersion:
		{
			if (!failed)
			{
				std::string version (4, ' ');
				for (int i = 3; i >= 0; --i)
					version[3 - i] = static_cast<char> ((data >> (8 * i)) & 0xff);
				log() << "Firmware version: " << version << std::endl;
			}
			next_initialization_step();
			break;
		}

		case Address::FlashCommit:
			log() << "Unexpected FlashCommit packet." << std::endl;
			break;

		case Address::ZeroGyros:
			if (failed)
			{
				log() << "Failed to zero gyros." << std::endl;
				repeat_initialization_step();
			}
			else
			{
				log() << "Gyros zeroed." << std::endl;
				next_initialization_step();
			}
			break;

		case Address::ResetEKF:
			next_initialization_step();
			break;

		case Address::GetData:
			log() << "Unexpected GetData packet." << std::endl;
			break;

		case Address::SetAccelRef:
		case Address::SetMagRef:
			next_initialization_step();
			break;

		case Address::ResetToFactory:
			log() << "Unexpected ResetToFactory packet." << std::endl;
			break;

		case Address::GPSSetHomePosition:
			log() << "Unexpected GPSSetHomePosition packet." << std::endl;
			break;

		case Address::EKFProcessVariance:
			if (failed)
				repeat_initialization_step();
			else
			{
				if (has_data)
					log() << "EKF process variance: " << to_float (data) << std::endl;
				next_initialization_step();
			}
			break;

		case Address::BadChecksum:
			// Don't pop command. Retry.
			log() << "Command failed: bad checksum." << std::endl;
			repeat_initialization_step();
			break;

		case Address::UnknownAddress:
			log() << "Command failed: unknown address." << std::endl;
			break;

		case Address::InvalidBatchSize:
			log() << "Command failed: invalid batch size." << std::endl;
			break;

		default:
			log() << "Unhandled address 0x"
				<< std::hex << std::setfill ('0') << std::setprecision (2) << static_cast<int> (address)
				<< std::dec << "." << std::endl;
			break;
	}
}


uint32_t
CHRUM6::sample_rate_setting (Frequency frequency) noexcept
{
	// Use formula from the spec: freq = (280/255) * sample_rate + 20.
	uint32_t x = (std::max ((frequency - 20_Hz), 0.1_Hz) / (280.0 / 255.0)).Hz();
	return Xefis::limit (x, 0u, 255u);
}


void
CHRUM6::status_verify (uint32_t data)
{
	bool serviceable = true;
	bool caution = false;

	auto isset = [&](StatusRegister reg) -> bool
	{
		return !!(data & static_cast<uint32_t> (reg));
	};

	if (isset (StatusRegister::MagDel))
	{
		caution = true;
		log() << "Magnetic sensor timeout." << std::endl;
	}

	if (isset (StatusRegister::AccelDel))
	{
		caution = true;
		log() << "Acceleration sensor timeout." << std::endl;
	}

	if (isset (StatusRegister::GyroDel))
	{
		caution = true;
		log() << "Gyroscope sensor timeout." << std::endl;
	}

	if (isset (StatusRegister::EKFDivergent))
	{
		caution = true;
		log() << "Divergent EKF - reset performed." << std::endl;
	}

	if (isset (StatusRegister::BusMagError))
	{
		caution = true;
		log() << "Magnetic sensor bus error." << std::endl;
	}

	if (isset (StatusRegister::BusAccelError))
	{
		caution = true;
		log() << "Acceleration sensor bus error." << std::endl;
	}

	if (isset (StatusRegister::BusGyroError))
	{
		caution = true;
		log() << "Gyroscope sensor bus error." << std::endl;
	}

	if (isset (StatusRegister::SelfTestMagZFail))
	{
		serviceable = false;
		log() << "Magnetic sensor Z axis: self test failure." << std::endl;
	}

	if (isset (StatusRegister::SelfTestMagYFail))
	{
		serviceable = false;
		log() << "Magnetic sensor Y axis: self test failure." << std::endl;
	}

	if (isset (StatusRegister::SelfTestMagXFail))
	{
		serviceable = false;
		log() << "Magnetic sensor X axis: self test failure." << std::endl;
	}

	if (isset (StatusRegister::SelfTestAccelZFail))
	{
		serviceable = false;
		log() << "Acceleration sensor Z axis: self test failure." << std::endl;
	}

	if (isset (StatusRegister::SelfTestAccelYFail))
	{
		serviceable = false;
		log() << "Acceleration sensor Y axis: self test failure." << std::endl;
	}

	if (isset (StatusRegister::SelfTestAccelXFail))
	{
		serviceable = false;
		log() << "Acceleration sensor X axis: self test failure." << std::endl;
	}

	if (isset (StatusRegister::SelfTestGyroZFail))
	{
		serviceable = false;
		log() << "Gyroscope sensor Z axis: self test failure." << std::endl;
	}

	if (isset (StatusRegister::SelfTestGyroYFail))
	{
		serviceable = false;
		log() << "Gyroscope sensor Y axis: self test failure." << std::endl;
	}

	if (isset (StatusRegister::SelfTestGyroXFail))
	{
		serviceable = false;
		log() << "Gyroscope sensor X axis: self test failure." << std::endl;
	}

	if (isset (StatusRegister::GyroInitFail))
	{
		serviceable = false;
		log() << "Gyroscope sensor initialization failure." << std::endl;
	}

	if (isset (StatusRegister::AccelInitFail))
	{
		serviceable = false;
		log() << "Gyroscope sensor initialization failure." << std::endl;
	}

	if (isset (StatusRegister::MagInitFail))
	{
		serviceable = false;
		log() << "Gyroscope sensor initialization failure." << std::endl;
	}

	if (!serviceable)
		_serviceable.write (false);
	if (caution)
		_caution.write (true);
}


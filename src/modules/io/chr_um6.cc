/* vim:ts=4
 *
 * Copyleft 2012…2014  Michał Gawron
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


constexpr Time	CHRUM6::RestartDelay;
constexpr Time	CHRUM6::AliveCheckInterval;
constexpr Time	CHRUM6::StatusCheckInterval;
constexpr Time	CHRUM6::InitializationDelay;


CHRUM6::CHRUM6 (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	std::string device_path;

	parse_settings (config, {
		{ "serial.device", device_path, true },
		{ "serial.baud-rate", _baud_rate, true },
		{ "sample-rate", _sample_rate, true },
		{ "ekf.process-variance", _ekf_process_variance, false },
	});

	parse_properties (config, {
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

	_restart_timer = std::make_unique<QTimer> (this);
	_restart_timer->setInterval (RestartDelay.ms());
	_restart_timer->setSingleShot (true);
	QObject::connect (_restart_timer.get(), SIGNAL (timeout()), this, SLOT (open_device()));

	_alive_check_timer = std::make_unique<QTimer> (this);
	_alive_check_timer->setInterval (AliveCheckInterval.ms());
	_alive_check_timer->setSingleShot (false);
	QObject::connect (_alive_check_timer.get(), SIGNAL (timeout()), this, SLOT (alive_check_failed()));

	_status_check_timer = std::make_unique<QTimer> (this);
	_status_check_timer->setInterval (StatusCheckInterval.ms());
	_status_check_timer->setSingleShot (false);
	QObject::connect (_status_check_timer.get(), &QTimer::timeout, this, &CHRUM6::status_check);

	_initialization_timer = std::make_unique<QTimer> (this);
	_initialization_timer->setInterval (InitializationDelay.ms());
	_initialization_timer->setSingleShot (true);
	QObject::connect (_initialization_timer.get(), SIGNAL (timeout()), this, SLOT (initialization_timeout()));

	_serviceable.set_default (false);
	_caution.set_default (false);
	_failures.set_default (0);

	Xefis::SerialPort::Configuration sp_config;
	sp_config.set_device_path (device_path);
	sp_config.set_baud_rate (_baud_rate);
	sp_config.set_data_bits (8);
	sp_config.set_stop_bits (1);
	sp_config.set_parity_bit (Xefis::SerialPort::Parity::None);

	_serial_port = std::make_unique<Xefis::SerialPort>();
	_serial_port->set_configuration (sp_config);
	_serial_port->set_max_read_failures (3);

	_sensor = std::make_unique<Xefis::CHRUM6> (_serial_port.get());
	_sensor->set_logger (log());
	_sensor->set_alive_check_callback (std::bind (&CHRUM6::alive_check, this));
	_sensor->set_communication_failure_callback (std::bind (&CHRUM6::communication_failure, this));
	_sensor->set_incoming_messages_callback (std::bind (&CHRUM6::process_message, this, std::placeholders::_1));
	_sensor->set_auto_retry (true);

	open_device();
}


void
CHRUM6::open_device()
{
	try {
		_alive_check_timer->start();

		reset();
		if (_serial_port->open())
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
	log() << "Fatal: failure detected" << (reason.empty() ? "" : ": " + reason) << ", closing device " << _serial_port->configuration().device_path() << std::endl;
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
	_sensor->read (Xefis::CHRUM6::DataAddress::Status, std::bind (&CHRUM6::status_verify, this, std::placeholders::_1));
}


void
CHRUM6::initialize()
{
	log() << "Begin initialization." << std::endl;

	_stage = Stage::Initialize;
	_initialization_timer->start();

	setup_communication();
}


void
CHRUM6::setup_communication()
{
	uint32_t data = 0;
	data |= static_cast<uint32_t> (Xefis::CHRUM6::CommunicationRegister::BEN);
	data |= static_cast<uint32_t> (Xefis::CHRUM6::CommunicationRegister::EU);
	data |= static_cast<uint32_t> (Xefis::CHRUM6::CommunicationRegister::AP);
	data |= static_cast<uint32_t> (Xefis::CHRUM6::CommunicationRegister::GP);
	data |= static_cast<uint32_t> (Xefis::CHRUM6::CommunicationRegister::MP);
	data |= static_cast<uint32_t> (Xefis::CHRUM6::CommunicationRegister::TMP);
	data |= Xefis::CHRUM6::bits_for_baud_rate (_baud_rate) << 8;
	data |= Xefis::CHRUM6::sample_rate_setting (_sample_rate);

	_sensor->write (ConfigurationAddress::Communication, data, [this] (Xefis::CHRUM6::Write req) {
		describe_errors (req);
		if (req.success())
			setup_misc_config();
	});
}


void
CHRUM6::setup_misc_config()
{
	uint32_t data = 0;
	data |= static_cast<uint32_t> (Xefis::CHRUM6::MiscConfigRegister::MUE);
	data |= static_cast<uint32_t> (Xefis::CHRUM6::MiscConfigRegister::AUE);
	data |= static_cast<uint32_t> (Xefis::CHRUM6::MiscConfigRegister::CAL);
	data |= static_cast<uint32_t> (Xefis::CHRUM6::MiscConfigRegister::QUAT);

	_sensor->write (ConfigurationAddress::MiscConfig, data, [this] (Xefis::CHRUM6::Write req) {
		describe_errors (req);
		if (req.success())
			log_firmware_version();
	});
}


void
CHRUM6::log_firmware_version()
{
	_sensor->command (CommandAddress::GetFWVersion, [this] (Xefis::CHRUM6::Command req) {
		describe_errors (req);
		if (req.success())
		{
			log() << "Firmware version: " << req.firmware_version() << std::endl;
			set_ekf_process_variance();
		}
	});
}


void
CHRUM6::set_ekf_process_variance()
{
	_sensor->write (ConfigurationAddress::EKFProcessVariance, *_ekf_process_variance, [this] (Xefis::CHRUM6::Write req) {
		describe_errors (req);
		if (req.success())
			reset_ekf();
	});
}


void
CHRUM6::reset_ekf()
{
	_sensor->command (CommandAddress::ResetEKF, [this] (Xefis::CHRUM6::Command req) {
		describe_errors (req);
		if (req.success())
			restore_gyro_bias_xy();
	});
}


void
CHRUM6::restore_gyro_bias_xy()
{
	if (_gyro_bias_xy)
	{
		log() << "Restoring previously acquired gyro biases: XY" << std::endl;

		_sensor->write (ConfigurationAddress::GyroBiasXY, *_gyro_bias_xy, [this] (Xefis::CHRUM6::Write req) {
			describe_errors (req);
			if (req.success())
				restore_gyro_bias_z();
		});
	}
	else
		align_gyros();
}


void
CHRUM6::restore_gyro_bias_z()
{
	if (_gyro_bias_z)
	{
		log() << "Restoring previously acquired gyro biases: Z" << std::endl;

		_sensor->write (ConfigurationAddress::GyroBiasZ, *_gyro_bias_z, [this] (Xefis::CHRUM6::Write req) {
			describe_errors (req);
			if (req.success())
				initialization_complete();
		});
	}
	else
		align_gyros();
}


void
CHRUM6::align_gyros()
{
	_sensor->command (CommandAddress::ZeroGyros, [this] (Xefis::CHRUM6::Command req) {
		describe_errors (req);
		if (req.success())
		{
			log() << "Gyros aligned." << std::endl;
			initialization_complete();
		}
	});
}


void
CHRUM6::initialization_complete()
{
	log() << "Initialization complete." << std::endl;
	_stage = Stage::Run;
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
}


void
CHRUM6::alive_check()
{
	_alive_check_timer->start();
}


void
CHRUM6::communication_failure()
{
	failure ("communication failed");
}


void
CHRUM6::process_message (Xefis::CHRUM6::Read req)
{
	switch (req.address())
	{
		case static_cast<uint32_t> (DataAddress::Temperature):
		{
			if (req.success())
				if (_internal_temperature.configured())
					_internal_temperature.write (Temperature::from_degC (req.value_as_float()));
			break;
		}

		case static_cast<uint32_t> (DataAddress::EulerPhiTheta):
		{
			if (req.success() && *_serviceable)
			{
				// Pitch (phi) and Roll (theta):
				const float factor = 0.0109863;
				float phi = factor * req.value_upper16();
				float theta = factor * req.value_lower16();
				_orientation_pitch.write (1_deg * theta);
				_orientation_roll.write (1_deg * phi);
			}
			break;
		}

		case static_cast<uint32_t> (DataAddress::EulerPsi):
		{
			if (req.success() && *_serviceable)
			{
				// Heading:
				const float factor = 0.0109863;
				float psi = Xefis::floored_mod (factor * req.value_upper16(), 0.f, 360.f);
				_orientation_magnetic_heading.write (1_deg * psi);
			}
			break;
		}

		case static_cast<uint32_t> (DataAddress::AccelProcXY):
		{
			if (req.success() && (_acceleration_x.configured() || _acceleration_y.configured()))
			{
				const float factor = 0.000183105;
				float x = factor * req.value_upper16();
				float y = factor * req.value_lower16();
				if (_acceleration_x.configured())
					_acceleration_x.write (x);
				if (_acceleration_y.configured())
					_acceleration_y.write (y);
			}
			break;
		}

		case static_cast<uint32_t> (DataAddress::AccelProcZ):
		{
			if (req.success() && _acceleration_z.configured())
			{
				const float factor = 0.000183105;
				float z = factor * req.value_upper16();
				_acceleration_z.write (z);
			}
			break;
		}

		case static_cast<uint32_t> (DataAddress::GyroProcXY):
		{
			if (req.success() && (_rotation_x.configured() || _rotation_y.configured()))
			{
				const float factor = 0.0610352;
				float x = factor * req.value_upper16();
				float y = factor * req.value_lower16();
				if (_rotation_x.configured())
					_rotation_x.write (1_deg * x);
				if (_rotation_y.configured())
					_rotation_y.write (1_deg * y);
			}
			break;
		}

		case static_cast<uint32_t> (DataAddress::GyroProcZ):
		{
			if (req.success() && _rotation_z.configured())
			{
				const float factor = 0.0610352;
				float z = factor * req.value_upper16();
				_rotation_z.write (1_deg * z);
			}
			break;
		}

		case static_cast<uint32_t> (DataAddress::MagProcXY):
		{
			if (req.success() && (_magnetic_x.configured() || _magnetic_y.configured()))
			{
				const float factor = 0.000305176;
				float x = factor * req.value_upper16();
				float y = factor * req.value_lower16();
				if (_magnetic_x.configured())
					_magnetic_x.write (x);
				if (_magnetic_y.configured())
					_magnetic_y.write (y);
			}
			break;
		}

		case static_cast<uint32_t> (DataAddress::MagProcZ):
		{
			if (req.success() && _magnetic_z.configured())
			{
				const float factor = 0.000305176;
				float z = factor * req.value_upper16();
				_magnetic_z.write (z);
			}
			break;
		}

		// This is sent after ZeroGyros completes:
		case static_cast<uint32_t> (ConfigurationAddress::GyroBiasXY):
		{
			if (req.success())
			{
				_gyro_bias_xy = req.value();
				log() << "Gyro bias X: " << req.value_upper16() << std::endl;
				log() << "Gyro bias Y: " << req.value_lower16() << std::endl;
			}
			break;
		}

		// This is sent after ZeroGyros completes:
		case static_cast<uint32_t> (ConfigurationAddress::GyroBiasZ):
		{
			if (req.success() && !_gyro_bias_z)
			{
				_gyro_bias_z = req.value();
				log() << "Gyro bias Z: " << req.value_upper16() << std::endl;
			}
			break;
		}

		/*
		 * Command registers.
		 */

		case static_cast<uint32_t> (CommandAddress::FlashCommit):
			log() << "Unexpected FlashCommit packet." << std::endl;
			break;

		case static_cast<uint32_t> (CommandAddress::GetData):
			log() << "Unexpected GetData packet." << std::endl;
			break;

		case static_cast<uint32_t> (CommandAddress::ResetToFactory):
			log() << "Unexpected ResetToFactory packet." << std::endl;
			break;

		case static_cast<uint32_t> (CommandAddress::GPSSetHomePosition):
			log() << "Unexpected GPSSetHomePosition packet." << std::endl;
			break;

		default:
			log() << "Unexpected packet " << req.name() << " (0x"
				<< std::hex << std::setfill ('0') << std::setprecision (2) << req.address()
				<< ")." << std::endl;
			break;
	}
}


void
CHRUM6::status_verify (Xefis::CHRUM6::Read req)
{
	typedef Xefis::CHRUM6::StatusRegister StatusRegister;

	bool serviceable = true;
	bool caution = false;

	auto isset = [&](StatusRegister reg) -> bool
	{
		return !!(req.value() & static_cast<uint32_t> (reg));
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


void
CHRUM6::describe_errors (Xefis::CHRUM6::Request const& req)
{
	if (!req.success())
		log() << "Command " << req.name() << " failed; protocol error: " << req.protocol_error_description() << "; retries: " << req.retries() << "." << std::endl;
	else if (req.retries() > 0)
	{
		const char* str_retries = (req.retries() > 1) ? " retries" : " retry";
		log() << "Command " << req.name() << " succeeded after " << req.retries() << str_retries << " (BadChecksum)." << std::endl;
	}
}


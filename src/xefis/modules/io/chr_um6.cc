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
#include <xefis/core/system.h>
#include <xefis/utility/string.h>
#include <xefis/utility/numeric.h>

// Local:
#include "chr_um6.h"


CHRUM6::CHRUM6 (std::unique_ptr<CHRUM6_IO> module_io, xf::SerialPort&& serial_port, std::string const& instance):
	Module (std::move (module_io), instance),
	_serial_port (std::move (serial_port))
{
	_serial_port.set_max_read_failures (3);

	_restart_timer = std::make_unique<QTimer> (this);
	_restart_timer->setInterval (kRestartDelay.quantity<Millisecond>());
	_restart_timer->setSingleShot (true);
	QObject::connect (_restart_timer.get(), SIGNAL (timeout()), this, SLOT (open_device()));

	_alive_check_timer = std::make_unique<QTimer> (this);
	_alive_check_timer->setInterval (kAliveCheckInterval.quantity<Millisecond>());
	_alive_check_timer->setSingleShot (false);
	QObject::connect (_alive_check_timer.get(), SIGNAL (timeout()), this, SLOT (alive_check_failed()));

	_status_check_timer = std::make_unique<QTimer> (this);
	_status_check_timer->setInterval (kStatusCheckInterval.quantity<Millisecond>());
	_status_check_timer->setSingleShot (false);
	QObject::connect (_status_check_timer.get(), &QTimer::timeout, this, &CHRUM6::status_check);

	_initialization_timer = std::make_unique<QTimer> (this);
	_initialization_timer->setInterval (kInitializationDelay.quantity<Millisecond>());
	_initialization_timer->setSingleShot (true);
	QObject::connect (_initialization_timer.get(), SIGNAL (timeout()), this, SLOT (initialization_timeout()));

	_sensor = std::make_unique<xf::CHRUM6> (&_serial_port);
	_sensor->set_logger (log());
	_sensor->set_alive_check_callback (std::bind (&CHRUM6::alive_check, this));
	_sensor->set_communication_failure_callback (std::bind (&CHRUM6::communication_failure, this));
	_sensor->set_incoming_messages_callback (std::bind (&CHRUM6::process_message, this, std::placeholders::_1));
	_sensor->set_auto_retry (true);

	io.output_serviceable = false;
	io.output_caution = false;
	io.output_failures = 0;

	open_device();
}


void
CHRUM6::process (v2::Cycle const&)
{
	if (_sensor && _serial_port.good())
	{
		// Earth acceleration = measured acceleration + centripetal acceleration.

		if (_output_acceleration_x_changed() || _input_centripetal_x_changed())
		{
			Acceleration earth_x = 0_g;
			if (io.output_acceleration_x && io.input_centripetal_x)
				earth_x = *io.output_acceleration_x + *io.input_centripetal_x;
			_sensor->write (xf::CHRUM6::ConfigurationAddress::AccelRefX, static_cast<float> (earth_x.quantity<Gravity>()));
		}

		if (_output_acceleration_y_changed() || _input_centripetal_y_changed())
		{
			Acceleration earth_y = 0_g;
			if (io.output_acceleration_y && io.input_centripetal_y)
				earth_y = *io.output_acceleration_y + *io.input_centripetal_y;
			_sensor->write (xf::CHRUM6::ConfigurationAddress::AccelRefY, static_cast<float> (earth_y.quantity<Gravity>()));
		}

		if (_output_acceleration_z_changed() || _input_centripetal_z_changed())
		{
			Acceleration earth_z = 1_g;
			if (io.output_acceleration_z && io.input_centripetal_z)
				earth_z = *io.output_acceleration_z + *io.input_centripetal_z;
			_sensor->write (xf::CHRUM6::ConfigurationAddress::AccelRefZ, static_cast<float> (earth_z.quantity<Gravity>()));
		}
	}
}


void
CHRUM6::open_device()
{
	bool has_thrown = xf::Exception::guard ([&] {
		_alive_check_timer->start();

		reset();
		if (_serial_port.open())
			initialize();
		else
			restart();
	});

	if (has_thrown)
		failure ("exception in open_device()");
}


void
CHRUM6::failure (std::string const& reason)
{
	log() << "Fatal: failure detected" << (reason.empty() ? "" : ": " + reason) << ", closing device " << _serial_port.configuration().device_path() << std::endl;
	io.output_failures = *io.output_failures + 1;
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
	_sensor->read (xf::CHRUM6::DataAddress::Status, std::bind (&CHRUM6::status_verify, this, std::placeholders::_1));
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
	data |= static_cast<uint32_t> (xf::CHRUM6::CommunicationRegister::BEN);
	data |= static_cast<uint32_t> (xf::CHRUM6::CommunicationRegister::EU);
	data |= static_cast<uint32_t> (xf::CHRUM6::CommunicationRegister::AP);
	data |= static_cast<uint32_t> (xf::CHRUM6::CommunicationRegister::GP);
	data |= static_cast<uint32_t> (xf::CHRUM6::CommunicationRegister::MP);
	data |= static_cast<uint32_t> (xf::CHRUM6::CommunicationRegister::TMP);
	data |= xf::CHRUM6::bits_for_baud_rate (_serial_port.configuration().baud_rate()) << 8;
	data |= xf::CHRUM6::sample_rate_setting (*io.setting_sample_rate);

	_sensor->write (ConfigurationAddress::Communication, data, [this] (xf::CHRUM6::Write req) {
		describe_errors (req);
		if (req.success())
			setup_misc_config();
	});
}


void
CHRUM6::setup_misc_config()
{
	uint32_t data = 0;
	data |= static_cast<uint32_t> (xf::CHRUM6::MiscConfigRegister::MUE);
	data |= static_cast<uint32_t> (xf::CHRUM6::MiscConfigRegister::AUE);
	data |= static_cast<uint32_t> (xf::CHRUM6::MiscConfigRegister::CAL);
	data |= static_cast<uint32_t> (xf::CHRUM6::MiscConfigRegister::QUAT);

	_sensor->write (ConfigurationAddress::MiscConfig, data, [this] (xf::CHRUM6::Write req) {
		describe_errors (req);
		if (req.success())
			log_firmware_version();
	});
}


void
CHRUM6::log_firmware_version()
{
	_sensor->command (CommandAddress::GetFWVersion, [this] (xf::CHRUM6::Command req) {
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
	_sensor->write (ConfigurationAddress::EKFProcessVariance, *io.setting_ekf_process_variance, [this] (xf::CHRUM6::Write req) {
		describe_errors (req);
		if (req.success())
			reset_ekf();
	});
}


void
CHRUM6::reset_ekf()
{
	_sensor->command (CommandAddress::ResetEKF, [this] (xf::CHRUM6::Command req) {
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

		_sensor->write (ConfigurationAddress::GyroBiasXY, *_gyro_bias_xy, [this] (xf::CHRUM6::Write req) {
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

		_sensor->write (ConfigurationAddress::GyroBiasZ, *_gyro_bias_z, [this] (xf::CHRUM6::Write req) {
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
	_sensor->command (CommandAddress::ZeroGyros, [this] (xf::CHRUM6::Command req) {
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
	io.output_serviceable = true;
	_status_check_timer->start();
}


void
CHRUM6::reset()
{
	io.output_serviceable = false;
	io.output_orientation_pitch.set_nil();
	io.output_orientation_roll.set_nil();
	io.output_orientation_heading_magnetic.set_nil();
	io.output_acceleration_x.set_nil();
	io.output_acceleration_y.set_nil();
	io.output_acceleration_z.set_nil();
	io.output_rotation_x.set_nil();
	io.output_rotation_y.set_nil();
	io.output_rotation_z.set_nil();
	io.output_magnetic_x.set_nil();
	io.output_magnetic_y.set_nil();
	io.output_magnetic_z.set_nil();

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
CHRUM6::process_message (xf::CHRUM6::Read req)
{
	switch (req.address())
	{
		case static_cast<uint32_t> (DataAddress::Temperature):
		{
			if (req.success())
				io.output_internal_temperature = Quantity<Celsius> (req.value_as_float());
			break;
		}

		case static_cast<uint32_t> (DataAddress::EulerPhiTheta):
		{
			if (req.success() && io.output_serviceable.value_or (false))
			{
				si::Angle const factor = 0.0109863_deg;
				io.output_orientation_roll = factor * req.value_upper16();
				io.output_orientation_pitch = factor * req.value_lower16();
			}
			break;
		}

		case static_cast<uint32_t> (DataAddress::EulerPsi):
		{
			if (req.success() && io.output_serviceable.value_or (false))
			{
				si::Angle const factor = 0.0109863_deg;
				io.output_orientation_heading_magnetic = xf::floored_mod<si::Angle> (factor * req.value_upper16(), 0_deg, 360_deg);
			}
			break;
		}

		case static_cast<uint32_t> (DataAddress::AccelProcXY):
		{
			if (req.success())
			{
				si::Acceleration const factor = 0.000183105_g;
				io.output_acceleration_x = factor * req.value_upper16();
				io.output_acceleration_y = factor * req.value_lower16();
			}
			break;
		}

		case static_cast<uint32_t> (DataAddress::AccelProcZ):
		{
			if (req.success())
			{
				si::Acceleration const factor = 0.000183105_g;
				io.output_acceleration_z = factor * req.value_upper16();
			}
			break;
		}

		case static_cast<uint32_t> (DataAddress::GyroProcXY):
		{
			if (req.success())
			{
				si::AngularVelocity const factor = 0.0610352_deg / 1_s;
				io.output_rotation_x = factor * req.value_upper16();
				io.output_rotation_y = factor * req.value_lower16();
			}
			break;
		}

		case static_cast<uint32_t> (DataAddress::GyroProcZ):
		{
			if (req.success())
			{
				si::AngularVelocity const factor = 0.0610352_deg / 1_s;
				io.output_rotation_z = factor * req.value_upper16();
			}
			break;
		}

		case static_cast<uint32_t> (DataAddress::MagProcXY):
		{
			if (req.success())
			{
				// Assume values are expressed in Teslas (it's not specified in the documentation):
				si::MagneticField const factor = 0.000305176_T;
				io.output_magnetic_x = factor * req.value_upper16();
				io.output_magnetic_y = factor * req.value_lower16();
			}
			break;
		}

		case static_cast<uint32_t> (DataAddress::MagProcZ):
		{
			if (req.success())
			{
				si::MagneticField const factor = 0.000305176_T;
				io.output_magnetic_z = factor * req.value_upper16();
			}
			break;
		}

		// This is sent after ZeroGyros is completed:
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
CHRUM6::status_verify (xf::CHRUM6::Read req)
{
	typedef xf::CHRUM6::StatusRegister StatusRegister;

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
		io.output_serviceable = false;

	if (caution)
		io.output_caution = true;
}


void
CHRUM6::describe_errors (xf::CHRUM6::Request const& req)
{
	if (!req.success())
		log() << "Command " << req.name() << " failed; protocol error: " << req.protocol_error_description() << "; retries: " << req.retries() << "." << std::endl;
	else if (req.retries() > 0)
	{
		const char* str_retries = (req.retries() > 1) ? " retries" : " retry";
		log() << "Command " << req.name() << " succeeded after " << req.retries() << str_retries << " (BadChecksum)." << std::endl;
	}
}


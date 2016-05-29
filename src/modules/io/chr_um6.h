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

#ifndef XEFIS__MODULES__IO__CHR_UM6_H__INCLUDED
#define XEFIS__MODULES__IO__CHR_UM6_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>
#include <memory>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/property.h>
#include <xefis/core/module.h>
#include <xefis/support/bus/serial_port.h>
#include <xefis/support/devices/chr_um6.h>


/**
 * Warning: this module uses I/O in main thread, which may block.
 *
 * CH-Robotics UM6 sensor driver.
 * Uses UART for communication.
 */
class CHRUM6:
	public QObject,
	public xf::Module
{
	Q_OBJECT

	static constexpr Time	RestartDelay			= 200_ms;
	static constexpr Time	AliveCheckInterval		= 500_ms;
	static constexpr Time	StatusCheckInterval		= 200_ms;
	static constexpr Time	InitializationDelay		= 3_s;

	typedef xf::CHRUM6::ConfigurationAddress	ConfigurationAddress;
	typedef xf::CHRUM6::DataAddress				DataAddress;
	typedef xf::CHRUM6::CommandAddress			CommandAddress;

	enum class Stage
	{
		Initialize,
		Run,
	};

  public:
	// Ctor
	CHRUM6 (xf::ModuleManager* module_manager, QDomElement const& config);

  protected:
	// xf::Module
	void
	data_updated() override;

  private slots:
	/**
	 * Open device and start processing data.
	 */
	void
	open_device();

	/**
	 * Called when device doesn't respond for a while.
	 */
	void
	alive_check_failed();

	/**
	 * Called when initialization takes too long to complete.
	 */
	void
	initialization_timeout();

	/**
	 * Try to restart operation after failure is detected.
	 */
	void
	restart();

	/**
	 * Check device status: read fail bits, check temperature, etc.
	 */
	void
	status_check();

  private:
	/**
	 * Start setting up the device. It's asynchronous, and will
	 * issue several commands. When it's finished, initialization_complete()
	 * will be called.
	 */
	void
	initialize();

	/**
	 * Initialization chain: setup Communication register.
	 */
	void
	setup_communication();

	/**
	 * Initialization chain: setup MiscConfig register.
	 */
	void
	setup_misc_config();

	/**
	 * Initialization chain: log firmware version.
	 */
	void
	log_firmware_version();

	/**
	 * Initialization chain: set EKF process variance.
	 */
	void
	set_ekf_process_variance();

	/**
	 * Initialization chain: reset EKF.
	 */
	void
	reset_ekf();

	/**
	 * Restore XY gyro biases after a failure.
	 */
	void
	restore_gyro_bias_xy();

	/**
	 * Restore Z gyro bias after a failure.
	 */
	void
	restore_gyro_bias_z();

	/**
	 * Initialization chain: set gyro bias.
	 */
	void
	align_gyros();

	/**
	 * Called when initialization is complete.
	 */
	void
	initialization_complete();

	/**
	 * Reset buffer and state. A must after a failure of some sort.
	 */
	void
	reset();

	/**
	 * Indicate failure. Try to reopen device, perhaps
	 * with other baud-rate setting.
	 */
	void
	failure (std::string const& message);

	/**
	 * Called by CHRUM6 sensor object to indicate that
	 * the sensor is alive.
	 */
	void
	alive_check();

	/**
	 * Called when failure is detected by CHRUM6 sensor.
	 */
	void
	communication_failure();

	/**
	 * Process incoming messages from UM6 (everything that wasn't explicitly requested).
	 */
	void
	process_message (xf::CHRUM6::Read);

	/**
	 * Checks status bits and sets status/serviceable properties.
	 */
	void
	status_verify (xf::CHRUM6::Read);

	/**
	 * If command has failed, log error information.
	 */
	void
	describe_errors (xf::CHRUM6::Request const&);

  private:
	Unique<QTimer>				_restart_timer;
	Unique<QTimer>				_alive_check_timer;
	Unique<QTimer>				_status_check_timer;
	Unique<QTimer>				_initialization_timer;
	Shared<xf::SerialPort>		_serial_port;
	Unique<xf::CHRUM6>			_sensor;
	int							_failure_count			= 0;
	Optional<float>				_ekf_process_variance	= 0.5f;
	Frequency					_sample_rate			= 20_Hz;
	unsigned int				_baud_rate				= 115200;
	Stage						_stage					= Stage::Initialize;
	// Backup gyro bias values:
	Optional<uint32_t>			_gyro_bias_xy;
	Optional<uint32_t>			_gyro_bias_z;

	// Input:
	xf::PropertyAcceleration	_input_centrifugal_x;
	xf::PropertyAcceleration	_input_centrifugal_y;
	xf::PropertyAcceleration	_input_centrifugal_z;
	// Output:
	xf::PropertyBoolean			_serviceable;
	xf::PropertyBoolean			_caution;
	xf::PropertyInteger			_failures;
	xf::PropertyTemperature		_internal_temperature;
	xf::PropertyAngle			_orientation_pitch;
	xf::PropertyAngle			_orientation_roll;
	xf::PropertyAngle			_orientation_magnetic_heading;
	xf::PropertyAcceleration	_acceleration_x;
	xf::PropertyAcceleration	_acceleration_y;
	xf::PropertyAcceleration	_acceleration_z;
	xf::Property<AngularVelocity>
								_rotation_x;
	xf::Property<AngularVelocity>
								_rotation_y;
	xf::Property<AngularVelocity>
								_rotation_z;
	// What's the unit?
	xf::PropertyFloat			_magnetic_x;
	xf::PropertyFloat			_magnetic_y;
	xf::PropertyFloat			_magnetic_z;
};

#endif


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
#include <xefis/hardware/chr_um6.h>
#include <xefis/utility/serial_port.h>


/**
 * CH-Robotics UM6 sensor driver.
 * Uses UART for communication.
 */
class CHRUM6:
	public QObject,
	public Xefis::Module
{
	Q_OBJECT

	static constexpr Time	RestartDelay			= 200_ms;
	static constexpr Time	AliveCheckInterval		= 500_ms;
	static constexpr Time	StatusCheckInterval		= 200_ms;
	static constexpr Time	InitializationDelay		= 3_s;

	typedef Xefis::CHRUM6::ConfigurationAddress		ConfigurationAddress;
	typedef Xefis::CHRUM6::DataAddress				DataAddress;
	typedef Xefis::CHRUM6::CommandAddress			CommandAddress;

	enum class Stage
	{
		Initialize,
		Run,
	};

  public:
	// Ctor
	CHRUM6 (Xefis::ModuleManager* module_manager, QDomElement const& config);

  protected:
	// Xefis::Module
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
	process_message (Xefis::CHRUM6::Read);

	/**
	 * Checks status bits and sets status/serviceable properties.
	 */
	void
	status_verify (Xefis::CHRUM6::Read);

	/**
	 * If command has failed, log error information.
	 */
	void
	describe_errors (Xefis::CHRUM6::Request const&);

  private:
	Unique<QTimer>				_restart_timer;
	Unique<QTimer>				_alive_check_timer;
	Unique<QTimer>				_status_check_timer;
	Unique<QTimer>				_initialization_timer;
	Unique<Xefis::SerialPort>	_serial_port;
	Unique<Xefis::CHRUM6>		_sensor;
	int							_failure_count			= 0;
	Optional<float>				_ekf_process_variance	= 0.5f;
	Frequency					_sample_rate			= 20_Hz;
	std::string					_baud_rate				= "115200";
	Stage						_stage					= Stage::Initialize;
	// Backup gyro bias values:
	Optional<uint32_t>			_gyro_bias_xy;
	Optional<uint32_t>			_gyro_bias_z;

	// Input:
	Xefis::PropertyAcceleration	_input_centrifugal_x;
	Xefis::PropertyAcceleration	_input_centrifugal_y;
	Xefis::PropertyAcceleration	_input_centrifugal_z;
	// Output:
	Xefis::PropertyBoolean		_serviceable;
	Xefis::PropertyBoolean		_caution;
	Xefis::PropertyInteger		_failures;
	Xefis::PropertyTemperature	_internal_temperature;
	Xefis::PropertyAngle		_orientation_pitch;
	Xefis::PropertyAngle		_orientation_roll;
	Xefis::PropertyAngle		_orientation_magnetic_heading;
	Xefis::PropertyAcceleration	_acceleration_x;
	Xefis::PropertyAcceleration	_acceleration_y;
	Xefis::PropertyAcceleration	_acceleration_z;
	Xefis::PropertyFrequency	_rotation_x;
	Xefis::PropertyFrequency	_rotation_y;
	Xefis::PropertyFrequency	_rotation_z;
	// What's the unit?
	Xefis::PropertyFloat		_magnetic_x;
	Xefis::PropertyFloat		_magnetic_y;
	Xefis::PropertyFloat		_magnetic_z;
};

#endif


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
#include <xefis/core/v2/module.h>
#include <xefis/core/v2/property.h>
#include <xefis/core/v2/setting.h>
#include <xefis/support/bus/serial_port.h>
#include <xefis/support/devices/chr_um6.h>
#include <xefis/utility/v2/actions.h>


/**
 * Warning: this module uses I/O in main thread, which may block.
 *
 * CH-Robotics UM6 sensor driver.
 * Uses UART for communication.
 */
class CHRUM6:
	public QObject,
	public x2::Module
{
  public:
	/*
	 * Settings
	 */

	x2::Setting<si::Frequency>				setting_sample_rate					{ this, 20_Hz };
	x2::Setting<float>						setting_ekf_process_variance		{ this, 0.5f };

	/*
	 * Input
	 */

	x2::PropertyIn<si::Acceleration>		input_centripetal_x					{ this, "/centripetal-acceleration/x" };
	x2::PropertyIn<si::Acceleration>		input_centripetal_y					{ this, "/centripetal-acceleration/y" };
	x2::PropertyIn<si::Acceleration>		input_centripetal_z					{ this, "/centripetal-acceleration/z" };

	/*
	 * Output
	 */

	x2::PropertyOut<bool>					output_serviceable					{ this, "/serviceable" };
	x2::PropertyOut<bool>					output_caution						{ this, "/caution" };
	x2::PropertyOut<int64_t>				output_failures						{ this, "/failures" };
	x2::PropertyOut<si::Temperature>		output_internal_temperature			{ this, "/internal-temperature" };
	x2::PropertyOut<si::Angle>				output_orientation_pitch			{ this, "/orientation/pitch" };
	x2::PropertyOut<si::Angle>				output_orientation_roll				{ this, "/orientation/roll" };
	x2::PropertyOut<si::Angle>				output_orientation_heading_magnetic	{ this, "/orientation/heading.magnetic" };
	x2::PropertyOut<si::Acceleration>		output_acceleration_x				{ this, "/acceleration/x" };
	x2::PropertyOut<si::Acceleration>		output_acceleration_y				{ this, "/acceleration/y" };
	x2::PropertyOut<si::Acceleration>		output_acceleration_z				{ this, "/acceleration/z" };
	x2::PropertyOut<si::AngularVelocity>	output_rotation_x					{ this, "/rotation/x" };
	x2::PropertyOut<si::AngularVelocity>	output_rotation_y					{ this, "/rotation.y" };
	x2::PropertyOut<si::AngularVelocity>	output_rotation_z					{ this, "/rotation.z" };
	// Note: it's _assumed_ that magnetic field strength returned by the device is in Teslas.
	// TODO check that assumption!
	x2::PropertyOut<si::MagneticField>		output_magnetic_x					{ this, "/magnetic/x" };
	x2::PropertyOut<si::MagneticField>		output_magnetic_y					{ this, "/magnetic/y" };
	x2::PropertyOut<si::MagneticField>		output_magnetic_z					{ this, "/magnetic/z" };

  private:
	Q_OBJECT

	static constexpr si::Time	RestartDelay			= 200_ms;
	static constexpr si::Time	AliveCheckInterval		= 500_ms;
	static constexpr si::Time	StatusCheckInterval		= 200_ms;
	static constexpr si::Time	InitializationDelay		= 3_s;

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
	explicit
	CHRUM6 (xf::SerialPort&& serial_port, std::string const& instance = {});

	// Module API
	void
	process (x2::Cycle const&) override;

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
	Unique<QTimer>						_restart_timer;
	Unique<QTimer>						_alive_check_timer;
	Unique<QTimer>						_status_check_timer;
	Unique<QTimer>						_initialization_timer;
	xf::SerialPort						_serial_port;
	Unique<xf::CHRUM6>					_sensor;
	int									_failure_count						{ 0 };
	Stage								_stage								{ Stage::Initialize };
	x2::PropChanged<si::Acceleration>	_input_centripetal_x_changed		{ input_centripetal_x };
	x2::PropChanged<si::Acceleration>	_input_centripetal_y_changed		{ input_centripetal_y };
	x2::PropChanged<si::Acceleration>	_input_centripetal_z_changed		{ input_centripetal_z };
	x2::PropChanged<si::Acceleration>	_output_acceleration_x_changed		{ output_acceleration_x };
	x2::PropChanged<si::Acceleration>	_output_acceleration_y_changed		{ output_acceleration_y };
	x2::PropChanged<si::Acceleration>	_output_acceleration_z_changed		{ output_acceleration_z };
	// Backup gyro bias values:
	Optional<uint32_t>					_gyro_bias_xy;
	Optional<uint32_t>					_gyro_bias_z;
};

#endif


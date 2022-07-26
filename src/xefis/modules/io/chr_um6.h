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

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/setting.h>
#include <xefis/core/sockets/module_socket.h>
#include <xefis/support/devices/chr_um6.h>
#include <xefis/support/sockets/socket_value_changed.h>

// Neutrino:
#include <neutrino/bus/serial_port.h>
#include <neutrino/logger.h>

// Qt:
#include <QTimer>

// Standard:
#include <cstddef>
#include <string>
#include <memory>


namespace si = neutrino::si;
using namespace neutrino::si::literals;


class CHRUM6_IO: public xf::Module
{
  public:
	using xf::Module::Module;

  public:
	xf::Setting<si::Frequency>			sample_rate						{ this, "sample_rate", 20_Hz };
	xf::Setting<float>					ekf_process_variance			{ this, "ekf_process_variance", 0.5f };

	xf::ModuleIn<si::Acceleration>		centripetal_x					{ this, "centripetal-acceleration/x" };
	xf::ModuleIn<si::Acceleration>		centripetal_y					{ this, "centripetal-acceleration/y" };
	xf::ModuleIn<si::Acceleration>		centripetal_z					{ this, "centripetal-acceleration/z" };

	xf::ModuleOut<bool>					serviceable						{ this, "serviceable" };
	xf::ModuleOut<bool>					caution							{ this, "caution" };
	xf::ModuleOut<int64_t>				failures						{ this, "failures" };
	xf::ModuleOut<si::Temperature>		internal_temperature			{ this, "internal-temperature" };
	xf::ModuleOut<si::Angle>			orientation_pitch				{ this, "orientation/pitch" };
	xf::ModuleOut<si::Angle>			orientation_roll				{ this, "orientation/roll" };
	xf::ModuleOut<si::Angle>			orientation_heading_magnetic	{ this, "orientation/heading.magnetic" };
	xf::ModuleOut<si::Acceleration>		acceleration_x					{ this, "acceleration/x" };
	xf::ModuleOut<si::Acceleration>		acceleration_y					{ this, "acceleration/y" };
	xf::ModuleOut<si::Acceleration>		acceleration_z					{ this, "acceleration/z" };
	xf::ModuleOut<si::AngularVelocity>	rotation_x						{ this, "rotation/x" };
	xf::ModuleOut<si::AngularVelocity>	rotation_y						{ this, "rotation.y" };
	xf::ModuleOut<si::AngularVelocity>	rotation_z						{ this, "rotation.z" };
	// Note: it's _assumed_ that magnetic field strength returned by the device is in Teslas.
	// TODO check that assumption!
	xf::ModuleOut<si::MagneticField>	magnetic_x						{ this, "magnetic/x" };
	xf::ModuleOut<si::MagneticField>	magnetic_y						{ this, "magnetic/y" };
	xf::ModuleOut<si::MagneticField>	magnetic_z						{ this, "magnetic/z" };
};


/**
 * Warning: this module uses I/O in main thread, which may block.
 *
 * CH-Robotics UM6 sensor driver.
 * Uses UART for communication.
 */
class CHRUM6:
	public QObject,
	public CHRUM6_IO
{
	Q_OBJECT

  private:
	static constexpr char kLoggerScope[] = "mod::CHRUM6";

  private:
	static constexpr si::Time	kRestartDelay			= 200_ms;
	static constexpr si::Time	kAliveCheckInterval		= 500_ms;
	static constexpr si::Time	kStatusCheckInterval	= 200_ms;
	static constexpr si::Time	kInitializationDelay	= 3_s;

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
	CHRUM6 (xf::SerialPort&& serial_port, xf::Logger const&, std::string_view const& instance = {});

	// Module API
	void
	process (xf::Cycle const&) override;

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
	 * Checks status bits and sets status/serviceable sockets.
	 */
	void
	status_verify (xf::CHRUM6::Read);

	/**
	 * If command has failed, log error information.
	 */
	void
	describe_errors (xf::CHRUM6::Request const&);

  private:
	CHRUM6_IO&									_io									{ *this };
	xf::Logger									_logger;
	std::unique_ptr<QTimer>						_restart_timer;
	std::unique_ptr<QTimer>						_alive_check_timer;
	std::unique_ptr<QTimer>						_status_check_timer;
	std::unique_ptr<QTimer>						_initialization_timer;
	xf::SerialPort								_serial_port;
	std::unique_ptr<xf::CHRUM6>					_sensor;
	int											_failure_count						{ 0 };
	Stage										_stage								{ Stage::Initialize };
	xf::SocketValueChanged<si::Acceleration>	_input_centripetal_x_changed		{ _io.centripetal_x };
	xf::SocketValueChanged<si::Acceleration>	_input_centripetal_y_changed		{ _io.centripetal_y };
	xf::SocketValueChanged<si::Acceleration>	_input_centripetal_z_changed		{ _io.centripetal_z };
	xf::SocketValueChanged<si::Acceleration>	_output_acceleration_x_changed		{ _io.acceleration_x };
	xf::SocketValueChanged<si::Acceleration>	_output_acceleration_y_changed		{ _io.acceleration_y };
	xf::SocketValueChanged<si::Acceleration>	_output_acceleration_z_changed		{ _io.acceleration_z };
	// Backup gyro bias values:
	std::optional<uint32_t>						_gyro_bias_xy;
	std::optional<uint32_t>						_gyro_bias_z;
};

#endif


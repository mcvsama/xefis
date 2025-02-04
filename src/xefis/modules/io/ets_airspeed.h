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
 *
 * This module was based on Paparazzi AirspeedETS module.
 */

#ifndef XEFIS__MODULES__IO__ETS_AIRSPEED_H__INCLUDED
#define XEFIS__MODULES__IO__ETS_AIRSPEED_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/setting.h>
#include <xefis/core/sockets/module_socket.h>
#include <xefis/utility/smoother.h>

// Neutrino:
#include <neutrino/bus/i2c.h>
#include <neutrino/logger.h>

// Qt:
#include <QTimer>

// Standard:
#include <cstddef>


namespace si = neutrino::si;
using namespace neutrino::si::literals;


class ETSAirspeedIO: public xf::Module
{
  public:
	/*
	 * Settings
	 */

	xf::Setting<si::Time>		read_interval		{ this, "read_interval", 100_ms };
	xf::Setting<si::Time>		smoothing_time		{ this, "smoothing_time", 100_ms };

	/*
	 * Output
	 */

	xf::ModuleOut<bool>			serviceable			{ this, "serviceable" };
	xf::ModuleOut<si::Velocity>	airspeed			{ this, "airspeed" };
	xf::ModuleOut<si::Velocity>	airspeed_minimum	{ this, "airspeed.minimum" };
	xf::ModuleOut<si::Velocity>	airspeed_maximum	{ this, "airspeed.maximum" };

  public:
	using xf::Module::Module;
};


/**
 * Warning: this module uses I2C I/O in main thread, which may block.
 * TODO The above has to be fixed.
 *
 * Handles EagleTree Airspeed V3 sensor.
 * The sensor must be in default mode, not in 3-rd party mode.
 */
class ETSAirspeed:
	public QObject,
	public ETSAirspeedIO
{
  private:
	static constexpr char			kLoggerScope[]				= "mod::ETSAirspeed";
	static constexpr uint8_t		kValueRegister				= 0xea;
	static constexpr float			kValueScale					= 1.8f;
	static constexpr si::Time		kInitializationDelay		= 0.2_s;
	static constexpr unsigned int	kOffsetCalculationSamples	= 100;
	static constexpr uint16_t		kRawValueMinimum			= 1450;
	static constexpr uint16_t		kRawValueMaximum			= 1750;

	enum class Stage {
		Calibrating,
		Running,
	};

  public:
	// Ctor
	explicit
	ETSAirspeed (xf::ProcessingLoop&, xf::i2c::Device&&, xf::Logger const&, std::string_view const& instance = {});

	// Module API
	void
	initialize() override;

  private slots:
	/**
	 * Starts module calibration.
	 */
	void
	device_initialize();

	/**
	 * Reinitialize module after failure.
	 * Don't recalibrate.
	 */
	void
	reinitialize();

	/**
	 * Read data from the sensor and update sockets.
	 */
	void
	read();

  private:
	/**
	 * Called when enough initial samples are collected to get
	 * offset value.
	 */
	void
	offset_collected();

	/**
	 * Guard and reinitialize on I2C error.
	 */
	void
	guard (std::function<void()> guarded_code);

  private:
	ETSAirspeedIO&				_io								{ *this };
	xf::Logger					_logger;
	xf::i2c::Device				_device;
	Stage						_stage							{ Stage::Calibrating };
	QTimer*						_device_initialization_timer;
	QTimer*						_periodic_read_timer;
	std::vector<uint16_t>		_calibration_data;
	uint16_t					_offset							{ 0 };
	xf::Smoother<si::Velocity>	_airspeed_smoother				{ 100_ms };
};

#endif


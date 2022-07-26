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

#ifndef XEFIS__MODULES__SIMULATION__VIRTUAL_TEMPERATURE_SENSOR_H__INCLUDED
#define XEFIS__MODULES__SIMULATION__VIRTUAL_TEMPERATURE_SENSOR_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/setting.h>
#include <xefis/core/socket.h>
#include <xefis/support/simulation/aerodynamic.v0/flight_simulation.h>

// Neutrino:
#include <neutrino/math/normal_distribution.h>

// Standard:
#include <cstddef>
#include <random>


class VirtualTemperatureSensorIO: public xf::Module
{
  public:
	/*
	 * Settings
	 */

	xf::Setting<si::Time>								update_interval		{ this, "update_interval" };
	xf::Setting<xf::NormalVariable<si::Temperature>>	noise				{ this, "noise" };
	xf::Setting<si::Temperature>						resolution			{ this, "resolution" };

	/*
	 * Output
	 */

	xf::ModuleOut<bool>									serviceable			{ this, "serviceable" };
	xf::ModuleOut<si::Temperature>						temperature			{ this, "measured-temperature" };

  public:
	using xf::Module::Module;
};


class VirtualTemperatureSensor: public VirtualTemperatureSensorIO
{
  private:
	static constexpr char kLoggerScope[] { "mod::VirtualTemperatureSensor" };

  public:
	// Ctor
	explicit
	VirtualTemperatureSensor (xf::sim::FlightSimulation const&,
							  xf::SpaceVector<si::Length, xf::AirframeFrame> const& mount_location,
							  xf::Logger const&,
							  std::string_view const& instance = {});

	// Module API
	void
	process (xf::Cycle const&) override;

  private:
	VirtualTemperatureSensorIO&						_io					{ *this };
	xf::Logger										_logger;
	xf::sim::FlightSimulation const&				_flight_simulation;
	xf::SpaceVector<si::Length, xf::AirframeFrame>	_mount_location;
	// Device's noise:
	std::default_random_engine						_random_generator;
	xf::NormalDistribution<si::Temperature>			_noise				{ 0_K, 0_K };
	si::Time										_last_measure_time	{ 0_s };
};

#endif

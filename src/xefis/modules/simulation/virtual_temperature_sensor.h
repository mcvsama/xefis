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

// Standard:
#include <cstddef>
#include <random>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/property.h>
#include <xefis/core/setting.h>
#include <xefis/support/simulation/flight_simulation.h>
#include <xefis/utility/normal_distribution.h>


class VirtualTemperatureSensorIO: public xf::ModuleIO
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

	xf::PropertyOut<bool>								serviceable			{ this, "serviceable" };
	xf::PropertyOut<si::Temperature>					temperature			{ this, "measured-temperature" };
};


class VirtualTemperatureSensor: public xf::Module<VirtualTemperatureSensorIO>
{
  private:
	static constexpr char kLoggerScope[] { "mod::VirtualTemperatureSensor" };

  public:
	// Ctor
	explicit
	VirtualTemperatureSensor (xf::sim::FlightSimulation const&,
							  xf::SpaceVector<si::Length, xf::AirframeFrame> const& mount_location,
							  std::unique_ptr<VirtualTemperatureSensorIO>,
							  xf::Logger const&,
							  std::string_view const& instance = {});

	// Module API
	void
	process (xf::Cycle const&) override;

  private:
	xf::Logger										_logger;
	xf::sim::FlightSimulation const&				_flight_simulation;
	xf::SpaceVector<si::Length, xf::AirframeFrame>	_mount_location;
	// Device's noise:
	std::default_random_engine						_random_generator;
	xf::NormalDistribution<si::Temperature>			_noise				{ 0_K, 0_K };
	si::Time										_last_measure_time	{ 0_s };
};

#endif

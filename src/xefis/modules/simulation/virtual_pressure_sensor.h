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

#ifndef XEFIS__MODULES__SIMULATION__VIRTUAL_PRESSURE_SENSOR_H__INCLUDED
#define XEFIS__MODULES__SIMULATION__VIRTUAL_PRESSURE_SENSOR_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/setting.h>
#include <xefis/core/sockets/module_socket.h>
#include <xefis/support/simulation/devices/prandtl_tube.h>

// Neutrino:
#include <neutrino/math/normal_distribution.h>

// Standard:
#include <cstddef>
#include <random>


namespace nu = neutrino;
namespace si = nu::si;
using namespace si::literals;


class VirtualPressureSensorIO: public xf::Module
{
  public:
	/*
	 * Settings
	 */

	xf::Setting<si::Time>								update_interval		{ this, "update_interval" };
	xf::Setting<nu::math::NormalVariable<si::Pressure>>	noise				{ this, "noise" };
	xf::Setting<si::Pressure>							resolution			{ this, "resolution" };

	/*
	 * Output
	 */

	xf::ModuleOut<bool>									serviceable			{ this, "serviceable" };
	xf::ModuleOut<si::Pressure>							pressure			{ this, "measured-pressure" };

  public:
	using xf::Module::Module;
};


class VirtualPressureSensor: public VirtualPressureSensorIO
{
  private:
	static constexpr char kLoggerScope[] { "mod::VirtualPressureSensor" };

  public:
	enum Probe
	{
		Pitot,	// Module will simulate total pressure.
		Static,	// Module will simulate static pressure.
	};

  public:
	// Ctor
	explicit
	VirtualPressureSensor (xf::ProcessingLoop&,
						   xf::sim::PrandtlTube const&,
						   Probe,
						   nu::Logger const&,
						   std::string_view const instance = {});

	// Module API
	void
	process (xf::Cycle const&) override;

  private:
	// TODO Sigmoidal temperature failure
	VirtualPressureSensorIO&					_io					{ *this };
	nu::Logger									_logger;
	xf::sim::PrandtlTube const&					_prandtl_tube;
	Probe										_probe;
	// Device's noise:
	std::default_random_engine					_random_generator;
	nu::math::NormalDistribution<si::Pressure>	_noise				{ 0_Pa, 0_Pa };
	si::Time									_last_measure_time	{ 0_s };
};

#endif

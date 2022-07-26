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

#ifndef XEFIS__MODULES__SYSTEMS__AFCS_ROLL_AUTOTRIM_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__AFCS_ROLL_AUTOTRIM_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/setting.h>
#include <xefis/core/sockets/module_socket.h>

// Standard:
#include <cstddef>


namespace si = neutrino::si;
using namespace neutrino::si::literals;


class AFCS_RollAutotrim_IO: public xf::Module
{
  public:
	/*
	 * Settings
	 */

	xf::Setting<double>			ias_coefficient				{ this, "ias_coefficient" };
	xf::Setting<double>			engine_torque_coefficient	{ this, "engine_torque_coefficient" };
	xf::Setting<double>			total_coefficient			{ this, "total_coefficient", 1.0 };

	/*
	 * Input
	 */

	xf::ModuleIn<si::Velocity>	measured_ias				{ this, "measured-ias" };
	xf::ModuleIn<si::Torque>	measured_engine_torque		{ this, "measured-eng-torque" };

	/*
	 * Output
	 */

	xf::ModuleOut<si::Angle>	ailerons_correction			{ this, "ailerons-correction" };

  public:
	using xf::Module::Module;
};


/**
 * Compute ailerons correction to apply to counter react engine's torque.
 * Depends on airspeed and engine RPM. Factors need to be obtained experimentally.
 *
 * Works only for air speeds well below Mach 1.
 */
class AFCS_RollAutotrim: public AFCS_RollAutotrim_IO
{
  public:
	// Ctor
	explicit
	AFCS_RollAutotrim (std::string_view const& instance = {});

  protected:
	// Module API
	void
	process (xf::Cycle const&) override;

  private:
	AFCS_RollAutotrim_IO& _io { *this };
};

#endif

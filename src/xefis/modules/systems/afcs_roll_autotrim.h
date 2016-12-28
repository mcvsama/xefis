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

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/module.h>
#include <xefis/core/v2/property.h>
#include <xefis/core/v2/setting.h>


/**
 * Compute ailerons correction to apply to counter react engine's torque.
 * Depends on airspeed and engine RPM. Factors need to be obtained experimentally.
 *
 * Works only for air speeds well below Mach 1.
 */
class AFCS_RollAutotrim: public x2::Module
{
  public:
	/*
	 * Settings
	 */

	x2::Setting<double>				setting_ias_coefficient				{ this };
	x2::Setting<double>				setting_engine_torque_coefficient	{ this };
	x2::Setting<double>				setting_total_coefficient			{ this, 1.0 };

	/*
	 * Input
	 */

	x2::PropertyIn<si::Velocity>	input_measured_ias					{ this, "/measured-ias" };
	x2::PropertyIn<si::Torque>		input_measured_engine_torque		{ this, "/measured-eng-torque" };

	/*
	 * Output
	 */

	x2::PropertyOut<si::Angle>		output_ailerons_correction			{ this, "/ailerons-correction" };

  public:
	// Ctor
	explicit AFCS_RollAutotrim (std::string const& instance = {});

  protected:
	// Module API
	void
	process (x2::Cycle const&) override;
};

#endif

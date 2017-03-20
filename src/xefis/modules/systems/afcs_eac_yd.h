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

#ifndef XEFIS__MODULES__SYSTEMS__AFCS_EAC_YD_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__AFCS_EAC_YD_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/module.h>
#include <xefis/core/v2/property.h>
#include <xefis/core/v2/property_observer.h>
#include <xefis/core/v2/setting.h>
#include <xefis/utility/pid_control.h>


/**
 * Controls rudder to obtain zero slip-skid value.
 */
class AFCS_EAC_YD: public x2::Module
{
  public:
	/*
	 * Settings
	 */

	x2::Setting<xf::PIDControl<si::Force, si::Angle>::Settings>
								setting_rudder_pid_settings	{ this };
	x2::Setting<double>			setting_rudder_pid_gain		{ this, 1.0 };
	x2::Setting<si::Angle>		setting_deflection_limit	{ this };

	/*
	 * Input
	 */

	x2::PropertyIn<bool>		input_enabled				{ this, "/enabled" };
	x2::PropertyIn<si::Force>	input_slip_skid				{ this, "/slip-skid" };

	/*
	 * Output
	 */

	x2::PropertyOut<si::Angle>	output_rudder_deflection	{ this, "/rudder-deflection" };

  public:
	// Ctor
	explicit
	AFCS_EAC_YD (std::string const& instance = {});

  private:
	// Module API
	void
	initialize() override;

	// Module API
	void
	process (x2::Cycle const&) override;

	/**
	 * Compute rudder.
	 */
	void
	compute();

  private:
	xf::PIDControl<si::Force, si::Angle>	_rudder_pid;
	x2::PropertyObserver					_rudder_computer;
};

#endif

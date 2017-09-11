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


class AFCS_EAC_YD_IO: public v2::ModuleIO
{
  public:
	/*
	 * Settings
	 */

	v2::Setting<xf::PIDControl<si::Force, si::Angle>::Settings>
								rudder_pid_settings	{ this, "rudder_pid_settings" };
	v2::Setting<double>			rudder_pid_gain		{ this, "rudder_pid_gain", 1.0 };
	v2::Setting<si::Angle>		deflection_limit	{ this, "deflection_limit" };

	/*
	 * Input
	 */

	v2::PropertyIn<bool>		enabled				{ this, "/enabled" };
	v2::PropertyIn<si::Force>	slip_skid			{ this, "/slip-skid" };

	/*
	 * Output
	 */

	v2::PropertyOut<si::Angle>	rudder_deflection	{ this, "/rudder-deflection" };
};


/**
 * Controls rudder to obtain zero slip-skid value.
 */
class AFCS_EAC_YD: public v2::Module<AFCS_EAC_YD_IO>
{
  public:
	// Ctor
	explicit
	AFCS_EAC_YD (std::unique_ptr<AFCS_EAC_YD_IO>, std::string const& instance = {});

  private:
	// Module API
	void
	initialize() override;

	// Module API
	void
	process (v2::Cycle const&) override;

	/**
	 * Compute rudder.
	 */
	void
	compute();

  private:
	xf::PIDControl<si::Force, si::Angle>	_rudder_pid;
	v2::PropertyObserver					_rudder_computer;
};

#endif

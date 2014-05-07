/* vim:ts=4
 *
 * Copyleft 2012…2014  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__MODULES__SYSTEMS__YAW_DAMPER_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__YAW_DAMPER_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/property.h>
#include <xefis/core/property_observer.h>
#include <xefis/utility/pid_control.h>


class YawDamper: public Xefis::Module
{
  public:
	// Ctor
	YawDamper (Xefis::ModuleManager*, QDomElement const& config);

  private:
	// Module API
	void
	data_updated() override;

	/**
	 * Compute rudder.
	 */
	void
	compute();

  private:
	Xefis::PIDControl<double>	_rudder_pid;
	// Settings:
	double						_rudder_p;
	double						_rudder_i;
	double						_rudder_d;
	double						_rudder_gain;
	double						_limit;
	// Input:
	Xefis::PropertyBoolean		_input_enabled;
	Xefis::PropertyFloat		_input_slip_skid_g;
	// Output:
	Xefis::PropertyFloat		_output_rudder;
	// Other:
	Xefis::PropertyObserver		_rudder_computer;
};

#endif

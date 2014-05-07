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

#ifndef XEFIS__MODULES__SYSTEMS__AUTOTHROTTLE_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__AUTOTHROTTLE_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/property.h>
#include <xefis/core/property_observer.h>
#include <xefis/utility/pid_control.h>
#include <xefis/utility/smoother.h>


class Autothrottle: public Xefis::Module
{
	enum class SpeedMode
	{
		None		= 0,
		Thrust		= 1,
		Airspeed	= 2,
		sentinel	= 3,
	};

  public:
	// Ctor
	Autothrottle (Xefis::ModuleManager*, QDomElement const& config);

  protected:
	void
	data_updated() override;

  private:
	void
	compute_thrust();

	void
	speed_mode_changed();

  private:
	double						_computed_output_thrust	= 0.0;
	SpeedMode					_speed_mode				= SpeedMode::None;
	double						_output_thrust_minimum	= 0.0;
	double						_output_thrust_maximum	= 1.0;
	double						_ias_pid_p				= 1.0;
	double						_ias_pid_i				= 0.1;
	double						_ias_pid_d				= 0.0;
	double						_ias_to_thrust_scale	= 1.0;
	Xefis::PIDControl<double>	_ias_pid;
	Xefis::Range<double>		_output_thrust_extent;
	// Input:
	Xefis::PropertyInteger		_cmd_speed_mode;
	Xefis::PropertyFloat		_cmd_thrust;
	Xefis::PropertySpeed		_cmd_ias;
	Xefis::PropertySpeed		_measured_ias;
	Xefis::PropertyFloat		_output_thrust;
	Xefis::PropertyBoolean		_disengage_at;
	// Other:
	Xefis::PropertyObserver		_thrust_computer;
};

#endif

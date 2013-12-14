/* vim:ts=4
 *
 * Copyleft 2012…2013  Michał Gawron
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
	speed_mode_changed();

  private:
	Time						_dt							= 0_s;
	Xefis::PIDControl<float>	_thrust_pid;
	Xefis::PIDControl<float>	_ias_pid;
	Xefis::Smoother<double>		_output_power_smoother		= 250_ms;
	double						_computed_output_power;
	SpeedMode					_speed_mode					= SpeedMode::None;
	// TODO PID params as settings
	// Input:
	Xefis::PropertyBoolean		_enabled;
	Xefis::PropertyFloat		_power_limit_max;
	Xefis::PropertyFloat		_power_limit_min;
	Xefis::PropertyInteger		_cmd_speed_mode;
	Xefis::PropertyFloat		_cmd_thrust;
	Xefis::PropertyFloat		_cmd_ias;
	Xefis::PropertyFloat		_measured_thrust;
	Xefis::PropertyFloat		_measured_ias;
	Xefis::PropertyFloat		_output_power;
	Xefis::PropertyBoolean		_disengage_at;
};

#endif

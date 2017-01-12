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

#ifndef XEFIS__MODULES__SYSTEMS__AUTOTHROTTLE_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__AUTOTHROTTLE_H__INCLUDED

// Standard:
#include <cstddef>
#include <utility>

// Qt:
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/property.h>
#include <xefis/core/property_observer.h>
#include <xefis/utility/pid_control.h>
#include <xefis/utility/smoother.h>


class AFCS_AT: public xf::Module
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
	AFCS_AT (xf::ModuleManager*, QDomElement const& config);

  protected:
	void
	data_updated() override;

  private:
	void
	compute_thrust();

	void
	speed_mode_changed();

  private:
	SpeedMode					_speed_mode				= SpeedMode::None;
	si::Force					_output_thrust_minimum	= 0.0_N;
	si::Force					_output_thrust_maximum	= 1.0_N;
	double						_ias_pid_p				= 1.0;
	double						_ias_pid_i				= 0.1;
	double						_ias_pid_d				= 0.0;
	xf::PIDControl<si::Velocity, si::Force>
								_ias_pid				{ _ias_pid_p, _ias_pid_i, _ias_pid_d, 0.0_kt };
	xf::Smoother<si::Force>		_ias_pid_smoother		{ 250_ms };
	xf::Range<si::Force>		_output_thrust_extent;
	// Input:
	xf::PropertyInteger			_cmd_speed_mode;
	xf::Property<si::Force>		_cmd_thrust;
	xf::PropertySpeed			_cmd_ias;
	xf::PropertySpeed			_measured_ias;
	xf::Property<si::Force>		_output_thrust;
	xf::PropertyBoolean			_disengage_at;
	// Other:
	xf::PropertyObserver		_thrust_computer;
};

#endif

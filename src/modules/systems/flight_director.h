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

#ifndef XEFIS__MODULES__SYSTEMS__FLIGHT_DIRECTOR_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__FLIGHT_DIRECTOR_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/utility/pid.h>


class FlightDirector: public Xefis::Module
{
  public:
	enum LateralMode
	{
		LateralDisabled		= 0,
		FollowHeading		= 1,
		FollowTrack			= 2
	};

	enum VerticalMode
	{
		VerticalDisabled	= 0,
		AltitudeHold		= 1,
		VerticalSpeed		= 2,
		FlightPathAngle		= 3
	};

  public:
	// Ctor
	FlightDirector (Xefis::ModuleManager*, QDomElement const& config);

  protected:
	void
	data_updated() override;

  private:
	Xefis::PID<float>		_magnetic_heading_pid;
	Xefis::PID<float>		_magnetic_track_pid;
	Xefis::PID<float>		_altitude_pid;
	Xefis::PID<float>		_vertical_speed_pid;
	Xefis::PID<float>		_fpa_pid;
	Time					_dt = 0_s;
	// Input:
	// TODO PID params as settings:
	Xefis::PropertyBoolean	_enabled;
	Xefis::PropertyInteger	_lateral_mode;
	Xefis::PropertyInteger	_vertical_mode;
	Xefis::PropertyFloat	_pitch_limit_deg;
	Xefis::PropertyFloat	_roll_limit_deg;
	Xefis::PropertyFloat	_selected_magnetic_heading_deg;
	Xefis::PropertyFloat	_selected_magnetic_track_deg;
	Xefis::PropertyFloat	_selected_altitude_ft;
	Xefis::PropertyFloat	_selected_vertical_speed_fpm;
	Xefis::PropertyFloat	_selected_fpa_deg;
	Xefis::PropertyFloat	_measured_magnetic_heading_deg;
	Xefis::PropertyFloat	_measured_magnetic_track_deg;
	Xefis::PropertyFloat	_measured_altitude_ft;
	Xefis::PropertyFloat	_measured_vertical_speed_fpm;
	Xefis::PropertyFloat	_measured_fpa_deg;
	// Output:
	Xefis::PropertyFloat	_output_pitch_deg;
	Xefis::PropertyFloat	_output_roll_deg;
};

#endif

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
	Xefis::PropertyAngle	_pitch_limit;
	Xefis::PropertyAngle	_roll_limit;
	Xefis::PropertyAngle	_selected_magnetic_heading;
	Xefis::PropertyAngle	_selected_magnetic_track;
	Xefis::PropertyLength	_selected_altitude;
	Xefis::PropertySpeed	_selected_vertical_speed;
	Xefis::PropertyAngle	_selected_fpa;
	Xefis::PropertyAngle	_measured_magnetic_heading;
	Xefis::PropertyAngle	_measured_magnetic_track;
	Xefis::PropertyLength	_measured_altitude;
	Xefis::PropertySpeed	_measured_vertical_speed;
	Xefis::PropertyAngle	_measured_fpa;
	// Output:
	Xefis::PropertyAngle	_output_pitch;
	Xefis::PropertyAngle	_output_roll;
};

#endif

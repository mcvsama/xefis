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

#ifndef XEFIS__MODULES__COMPUTERS__AP_H__INCLUDED
#define XEFIS__MODULES__COMPUTERS__AP_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/utility/timestamp.h>
#include <xefis/utility/pid.h>


class AP: public Xefis::Module
{
  public:
	enum VerticalMode
	{
		AltitudeSet		= 0,
		ClimbRateSet	= 1
	};

  public:
	// Ctor
	AP (Xefis::ModuleManager*, QDomElement const& config);

  protected:
	void
	data_updated() override;

  private:
	void
	compute_ap_settings();

	void
	compute_joystick_input();

	float
	remove_dead_zone (float input, float dead_deflection);

  private:
	Xefis::PID<float>		_heading_pid;
	Xefis::PID<float>		_altitude_pid;
	Xefis::PID<float>		_cbr_pid;
	Xefis::PID<float>		_output_pitch_pid;
	Xefis::PID<float>		_output_roll_pid;
	Angle					_manual_output_pitch;
	Angle					_manual_output_roll;
	Angle					_auto_output_pitch;
	Angle					_auto_output_roll;
	Xefis::Timestamp		_dt;
	// Input:
	// TODO PIDs parameters
	Xefis::PropertyBoolean	_ap_enabled;
	Xefis::PropertyFloat	_bank_limit_deg;
	Xefis::PropertyFloat	_yank_limit_deg;
	Xefis::PropertyFloat	_selected_mag_heading_deg;
	Xefis::PropertyFloat	_selected_altitude_ft;
	Xefis::PropertyFloat	_selected_cbr_fpm;
	Xefis::PropertyInteger	_vertical_mode;
	Xefis::PropertyFloat	_measured_mag_heading_deg;
	Xefis::PropertyFloat	_measured_altitude_ft;
	Xefis::PropertyFloat	_measured_cbr_fpm;
	Xefis::PropertyFloat	_input_pitch_axis;
	Xefis::PropertyFloat	_input_roll_axis;
	Xefis::PropertyFloat	_pitch_axis_dead_zone;
	Xefis::PropertyFloat	_max_pitch_angle_deg;
	Xefis::PropertyFloat	_max_roll_angle_deg;
	Xefis::PropertyFloat	_roll_axis_dead_zone;
	Xefis::PropertyFloat	_orientation_pitch_deg;
	Xefis::PropertyFloat	_orientation_roll_deg;
	// Output:
	Xefis::PropertyFloat	_output_control_stick_pitch;
	Xefis::PropertyFloat	_output_control_stick_roll;
	Xefis::PropertyFloat	_output_pitch_deg;
	Xefis::PropertyFloat	_output_roll_deg;
};

#endif

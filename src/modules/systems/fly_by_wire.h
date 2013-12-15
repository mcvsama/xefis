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

#ifndef XEFIS__MODULES__SYSTEMS__FLY_BY_WIRE_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__FLY_BY_WIRE_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/utility/pid_control.h>


class FlyByWire: public Xefis::Module
{
	enum Mode
	{
		ManualMode			= 0,
		StabilizedMode		= 1,
		FlightDirectorMode	= 2
	};

  public:
	// Ctor
	FlyByWire (Xefis::ModuleManager*, QDomElement const& config);

  private:
	void
	data_updated() override;

	void
	integrate_manual_input();

  private:
	// Used with joystick input:
	Xefis::PIDControl<float>	_manual_pitch_pid;
	Xefis::PIDControl<float>	_manual_roll_pid;
	Angle						_computed_output_pitch	= 0_deg;
	Angle						_computed_output_roll	= 0_deg;
	// Stabilizer PIDs:
	Xefis::PIDControl<float>	_elevator_pid;
	Xefis::PIDControl<float>	_ailerons_pid;
	Xefis::PIDControl<float>	_rudder_pid;
	Time						_dt						= 0_s;
	// Input:
	// TODO different stabilization parameters for joystick input and for F/D input.
	// TODO PID params as settings, not properties.
	Xefis::PropertyInteger		_mode;
	Xefis::PropertyAngle		_pitch_extent;
	Xefis::PropertyAngle		_roll_extent;
	Xefis::PropertyFloat		_stabilization_gain;
	Xefis::PropertyFloat		_pitch_gain;
	Xefis::PropertyFloat		_pitch_p;
	Xefis::PropertyFloat		_pitch_i;
	Xefis::PropertyFloat		_pitch_d;
	Xefis::PropertyFloat		_pitch_error_power;
	Xefis::PropertyFloat		_roll_gain;
	Xefis::PropertyFloat		_roll_p;
	Xefis::PropertyFloat		_roll_i;
	Xefis::PropertyFloat		_roll_d;
	Xefis::PropertyFloat		_roll_error_power;
	Xefis::PropertyFloat		_yaw_gain;
	Xefis::PropertyFloat		_yaw_p;
	Xefis::PropertyFloat		_yaw_i;
	Xefis::PropertyFloat		_yaw_d;
	Xefis::PropertyFloat		_yaw_error_power;
	Xefis::PropertyFloat		_input_pitch_axis;
	Xefis::PropertyFloat		_input_roll_axis;
	Xefis::PropertyFloat		_input_yaw_axis;
	Xefis::PropertyAngle		_input_pitch;
	Xefis::PropertyAngle		_input_roll;
	Xefis::PropertyAngle		_measured_pitch;
	Xefis::PropertyAngle		_measured_roll;
	Xefis::PropertyFloat		_measured_slip_skid_g;
	Xefis::PropertyFloat		_elevator_minimum;
	Xefis::PropertyFloat		_elevator_maximum;
	Xefis::PropertyFloat		_ailerons_minimum;
	Xefis::PropertyFloat		_ailerons_maximum;
	Xefis::PropertyFloat		_rudder_minimum;
	Xefis::PropertyFloat		_rudder_maximum;
	// Output:
	Xefis::PropertyAngle		_output_control_stick_pitch;
	Xefis::PropertyAngle		_output_control_stick_roll;
	Xefis::PropertyAngle		_output_pitch;
	Xefis::PropertyAngle		_output_roll;
	Xefis::PropertyFloat		_output_elevator;
	Xefis::PropertyFloat		_output_ailerons;
	Xefis::PropertyFloat		_output_rudder;
};

#endif

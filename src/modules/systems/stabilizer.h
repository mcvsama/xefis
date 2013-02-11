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

#ifndef XEFIS__MODULES__SYSTEMS__STABILIZER_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__STABILIZER_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/utility/pid.h>


class Stabilizer: public Xefis::Module
{
  public:
	// Ctor
	Stabilizer (Xefis::ModuleManager*, QDomElement const& config);

  protected:
	void
	data_updated() override;

  private:
	Xefis::PID<float>		_elevator_pid;
	Xefis::PID<float>		_ailerons_pid;
	Xefis::PID<float>		_rudder_pid;
	Xefis::Timestamp		_dt;
	// Input:
	Xefis::PropertyFloat	_stabilization_gain;
	Xefis::PropertyFloat	_pitch_gain;
	Xefis::PropertyFloat	_pitch_p;
	Xefis::PropertyFloat	_pitch_i;
	Xefis::PropertyFloat	_pitch_d;
	Xefis::PropertyFloat	_pitch_error_power;
	Xefis::PropertyFloat	_roll_gain;
	Xefis::PropertyFloat	_roll_p;
	Xefis::PropertyFloat	_roll_i;
	Xefis::PropertyFloat	_roll_d;
	Xefis::PropertyFloat	_roll_error_power;
	Xefis::PropertyFloat	_yaw_gain;
	Xefis::PropertyFloat	_yaw_p;
	Xefis::PropertyFloat	_yaw_i;
	Xefis::PropertyFloat	_yaw_d;
	Xefis::PropertyFloat	_yaw_error_power;
	Xefis::PropertyFloat	_input_pitch_deg;
	Xefis::PropertyFloat	_input_roll_deg;
	Xefis::PropertyFloat	_input_yaw_axis;
	Xefis::PropertyFloat	_measured_pitch_deg;
	Xefis::PropertyFloat	_measured_roll_deg;
	Xefis::PropertyFloat	_measured_slip_skid_g;
	Xefis::PropertyFloat	_elevator_minimum;
	Xefis::PropertyFloat	_elevator_maximum;
	Xefis::PropertyFloat	_ailerons_minimum;
	Xefis::PropertyFloat	_ailerons_maximum;
	Xefis::PropertyFloat	_rudder_minimum;
	Xefis::PropertyFloat	_rudder_maximum;
	// Output:
	Xefis::PropertyFloat	_output_elevator;
	Xefis::PropertyFloat	_output_ailerons;
	Xefis::PropertyFloat	_output_rudder;
};

#endif

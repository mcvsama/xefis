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

#ifndef XEFIS__MODULES__COMPUTERS__AP_MULTIPLEXER_H__INCLUDED
#define XEFIS__MODULES__COMPUTERS__AP_MULTIPLEXER_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/utility/timestamp.h>
#include <xefis/utility/pid.h>


class APMultiplexer: public Xefis::Module
{
  public:
	// Ctor
	APMultiplexer (Xefis::ModuleManager*, QDomElement const& config);

  protected:
	void
	data_updated() override;

	float
	remove_dead_zone (float input, float dead_deflection);

  private:
	Xefis::PID<float>		_output_pitch_pid;
	Xefis::PID<float>		_output_roll_pid;
	Angle					_output_pitch;
	Angle					_output_roll;
	Xefis::Timestamp		_dt;
	// Input:
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

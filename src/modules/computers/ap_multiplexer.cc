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

// Standard:
#include <cstddef>

// Qt:
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/numeric.h>

// Local:
#include "ap_multiplexer.h"


APMultiplexer::APMultiplexer (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager),
	_output_pitch_pid (1.0, 0.1, 0.0, 0.0),
	_output_roll_pid (1.0, 0.1, 0.0, 0.0)
{
	for (QDomElement& e: config)
	{
		if (e == "properties")
		{
			parse_properties (e, {
				{ "input-pitch-axis", _input_pitch_axis, true },
				{ "input-roll-axis", _input_roll_axis, true },
				{ "pitch-axis-dead-zone", _pitch_axis_dead_zone, false },
				{ "roll-axis-dead-zone", _roll_axis_dead_zone, false },
				{ "max-pitch-angle", _max_pitch_angle_deg, true },
				{ "max-roll-angle", _max_roll_angle_deg, true },
				{ "orientation-pitch", _orientation_pitch_deg, true },
				{ "orientation-roll", _orientation_roll_deg, true },
				{ "output-control-stick-pitch", _output_control_stick_pitch, false },
				{ "output-control-stick-roll", _output_control_stick_roll, false },
				{ "output-pitch", _output_pitch_deg, true },
				{ "output-roll", _output_roll_deg, true },
			});
		}
	}

	_output_pitch_pid.set_i_limit ({ -0.05f, +0.05f });
	_output_pitch_pid.set_winding (true);
	_output_roll_pid.set_i_limit ({ -0.05f, +0.05f });
	_output_roll_pid.set_winding (true);
}


void
APMultiplexer::data_updated()
{
	// Don't process if dt is too small:
	_dt += update_dt();
	if (_dt.seconds() < 0.005)
		return;

	// Shortcuts:
	Angle target_pitch_limit = *_max_pitch_angle_deg * 1_deg;
	Angle target_roll_limit = *_max_roll_angle_deg * 1_deg;
	float axis_pitch = remove_dead_zone (*_input_pitch_axis, _pitch_axis_dead_zone.valid() ? *_pitch_axis_dead_zone : 0);
	float axis_roll = remove_dead_zone (*_input_roll_axis, _roll_axis_dead_zone.valid() ? *_roll_axis_dead_zone : 0);
	Angle orientation_pitch = *_orientation_pitch_deg * 1_deg;
	Angle orientation_roll = *_orientation_roll_deg * 1_deg;
	// Target attitude - computed from current orientation and joystick deflection:
	Angle target_pitch = orientation_pitch + std::cos (orientation_roll.rad()) * axis_pitch * target_pitch_limit;
	Angle target_roll = orientation_roll + axis_roll * target_roll_limit;
	target_pitch = floored_mod<float> (target_pitch.deg(), -180.0, +180.0) * 1_deg;
	target_roll = floored_mod<float> (target_roll.deg(), -180.0, +180.0) * 1_deg;

	// Update output attitude:
	_output_pitch_pid.set_target (target_pitch.deg() / 180.f);
	_output_roll_pid.set_target (target_roll.deg() / 180.f);
	_output_pitch_pid.process (_output_pitch.deg() / 180.f, _dt.seconds());
	_output_roll_pid.process (_output_roll.deg() / 180.f, _dt.seconds());
	_output_pitch += std::abs (axis_pitch) * _output_pitch_pid.output() * 360_deg;
	_output_roll += std::abs (axis_roll) * _output_roll_pid.output() * 360_deg;
	_output_pitch = floored_mod<float> (_output_pitch.deg(), -180.0, +180.0) * 1_deg;
	_output_roll = floored_mod<float> (_output_roll.deg(), -180.0, +180.0) * 1_deg;

	_output_pitch_deg.write (_output_pitch.deg());
	_output_roll_deg.write (_output_roll.deg());

	if (!_output_control_stick_pitch.is_singular())
		_output_control_stick_pitch.write ((axis_pitch * target_pitch_limit).deg());
	if (!_output_control_stick_roll.is_singular())
		_output_control_stick_roll.write ((axis_roll * target_roll_limit).deg());

	_dt = Xefis::Timestamp::from_epoch (0);
}


inline float
APMultiplexer::remove_dead_zone (float input, float dead_deflection)
{
	if (std::abs (input) < dead_deflection)
		return 0.0;
	return input - sgn (input) * dead_deflection;
}


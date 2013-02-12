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
#include "ap.h"


AP::AP (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager),
	_heading_pid (1.0, 0.1, 0.0, 0.0),
	_altitude_pid (1.0, 0.1, 0.0, 0.0),
	_cbr_pid (1.0, 0.1, 0.0, 0.0),
	_output_pitch_pid (1.0, 0.1, 0.0, 0.0),
	_output_roll_pid (1.0, 0.1, 0.0, 0.0)
{
	for (QDomElement& e: config)
	{
		if (e == "properties")
		{
			parse_properties (e, {
				{ "autopilot-enabled", _autopilot_enabled, true },
				{ "bank-limit", _bank_limit_deg, true },
				{ "yank-limit", _yank_limit_deg, true },
				{ "selected-mag-heading", _selected_mag_heading_deg, true },
				{ "selected-altitude", _selected_altitude_ft, true },
				{ "selected-climb-rate", _selected_cbr_fpm, true },
				{ "vertical-mode", _vertical_mode, true },
				{ "measured-mag-heading", _measured_mag_heading_deg, true },
				{ "measured-altitude", _measured_altitude_ft, true },
				{ "measured-climb-rate", _measured_cbr_fpm, true },
				{ "measured-pitch", _orientation_pitch_deg, true },
				{ "measured-roll", _orientation_roll_deg, true },
				{ "input-pitch-axis", _input_pitch_axis, true },
				{ "input-roll-axis", _input_roll_axis, true },
				{ "pitch-axis-dead-zone", _pitch_axis_dead_zone, false },
				{ "roll-axis-dead-zone", _roll_axis_dead_zone, false },
				{ "max-pitch-angle", _max_pitch_angle_deg, true },
				{ "max-roll-angle", _max_roll_angle_deg, true },
				{ "output-control-stick-pitch", _output_control_stick_pitch, false },
				{ "output-control-stick-roll", _output_control_stick_roll, false },
				{ "output-pitch", _output_pitch_deg, true },
				{ "output-roll", _output_roll_deg, true },
			});
		}
	}

	for (auto pid: { &_heading_pid, &_output_pitch_pid, &_output_roll_pid })
	{
		pid->set_i_limit ({ -0.05f, +0.05f });
		pid->set_winding (true);
	}

	for (auto* pid: { &_altitude_pid, &_cbr_pid })
		pid->set_i_limit ({ -0.05f, +0.05f });
}


void
AP::data_updated()
{
	// Don't process if dt is too small:
	_dt += update_dt();
	if (_dt.seconds() < 0.005)
		return;

	compute_ap_settings();
	compute_joystick_input();

	if (*_autopilot_enabled)
	{
		_output_pitch_deg.write (_auto_output_pitch.deg());
		_output_roll_deg.write (_auto_output_roll.deg());
		_manual_output_pitch = _auto_output_pitch;
		_manual_output_roll = _auto_output_roll;
	}
	else
	{
		_output_pitch_deg.write (_manual_output_pitch.deg());
		_output_roll_deg.write (_manual_output_roll.deg());
	}

	_dt = Xefis::Timestamp::from_epoch (0);
}


void
AP::compute_ap_settings()
{
	double const alt_output_scale = 0.1;
	double const cbr_output_scale = 0.01;

	float bank_limit = *_bank_limit_deg;
	float yank_limit = *_yank_limit_deg;

	_heading_pid.set_target (renormalize (*_selected_mag_heading_deg, 0.0f, 360.f, -1.f, +1.f));
	_heading_pid.process (renormalize (*_measured_mag_heading_deg, 0.0f, 360.f, -1.f, +1.f), _dt.seconds());
	_auto_output_roll = limit<float> (_heading_pid.output() * 180.0, -bank_limit, +bank_limit) * 1_deg;

	_altitude_pid.set_target (*_selected_altitude_ft);
	_altitude_pid.process (*_measured_altitude_ft, _dt.seconds());

	_cbr_pid.set_target (*_selected_cbr_fpm);
	_cbr_pid.process (*_measured_cbr_fpm, _dt.seconds());

	switch (static_cast<VerticalMode> (*_vertical_mode))
	{
		case AltitudeSet:
			_auto_output_pitch = limit<float> (alt_output_scale * _altitude_pid.output(), -yank_limit, +yank_limit) * 1_deg;
			break;

		case ClimbRateSet:
			_auto_output_pitch = limit<float> (cbr_output_scale * _cbr_pid.output(), -yank_limit, +yank_limit) * 1_deg;
			break;
	}
}


void
AP::compute_joystick_input()
{
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
	_output_pitch_pid.process (_manual_output_pitch.deg() / 180.f, _dt.seconds());
	_output_roll_pid.process (_manual_output_roll.deg() / 180.f, _dt.seconds());
	_manual_output_pitch += std::abs (axis_pitch) * _output_pitch_pid.output() * 360_deg;
	_manual_output_roll += std::abs (axis_roll) * _output_roll_pid.output() * 360_deg;
	_manual_output_pitch = floored_mod<float> (_manual_output_pitch.deg(), -180.0, +180.0) * 1_deg;
	_manual_output_roll = floored_mod<float> (_manual_output_roll.deg(), -180.0, +180.0) * 1_deg;

	// Joystick visualisation on EFIS:
	if (!_output_control_stick_pitch.is_singular())
		_output_control_stick_pitch.write ((axis_pitch * target_pitch_limit).deg());
	if (!_output_control_stick_roll.is_singular())
		_output_control_stick_roll.write ((axis_roll * target_roll_limit).deg());
}


inline float
AP::remove_dead_zone (float input, float dead_deflection)
{
	if (std::abs (input) < dead_deflection)
		return 0.0;
	return input - sgn (input) * dead_deflection;
}


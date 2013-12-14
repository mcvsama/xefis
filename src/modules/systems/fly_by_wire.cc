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

// Local:
#include "fly_by_wire.h"


XEFIS_REGISTER_MODULE_CLASS ("systems/fly-by-wire", FlyByWire);


FlyByWire::FlyByWire (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config),
	_manual_pitch_pid (1.0, 0.1, 0.0, 0.0),
	_manual_roll_pid (1.0, 0.1, 0.0, 0.0),
	_elevator_pid (0.0, 0.0, 0.0, 0.0),
	_ailerons_pid (0.0, 0.0, 0.0, 0.0),
	_rudder_pid (0.0, 0.0, 0.0, 0.0)
{
	parse_properties (config, {
		{ "mode", _mode, true },
		{ "stabilization-gain", _stabilization_gain, true },
		{ "pitch-gain", _pitch_gain, true },
		{ "pitch-p", _pitch_p, true },
		{ "pitch-i", _pitch_i, true },
		{ "pitch-d", _pitch_d, true },
		{ "pitch-error-power", _pitch_error_power, true },
		{ "roll-gain", _roll_gain, true },
		{ "roll-p", _roll_p, true },
		{ "roll-i", _roll_i, true },
		{ "roll-d", _roll_d, true },
		{ "roll-error-power", _roll_error_power, true },
		{ "yaw-gain", _yaw_gain, true },
		{ "yaw-p", _yaw_p, true },
		{ "yaw-i", _yaw_i, true },
		{ "yaw-d", _yaw_d, true },
		{ "yaw-error-power", _yaw_error_power, true },
		{ "input-pitch-axis", _input_pitch_axis, true },
		{ "input-roll-axis", _input_roll_axis, true },
		{ "input-yaw-axis", _input_yaw_axis, true },
		{ "pitch-extent", _pitch_extent, true },
		{ "roll-extent", _roll_extent, true },
		{ "input-pitch", _input_pitch, true },
		{ "input-roll", _input_roll, true },
		{ "measured-pitch", _measured_pitch, true },
		{ "measured-roll", _measured_roll, true },
		{ "measured-slip-skid", _measured_slip_skid_g, true },
		{ "elevator-minimum", _elevator_minimum, true },
		{ "elevator-maximum", _elevator_maximum, true },
		{ "ailerons-minimum", _ailerons_minimum, true },
		{ "ailerons-maximum", _ailerons_maximum, true },
		{ "rudder-minimum", _rudder_minimum, true },
		{ "rudder-maximum", _rudder_maximum, true },
		{ "output-control-stick-pitch", _output_control_stick_pitch, false },
		{ "output-control-stick-roll", _output_control_stick_roll, false },
		{ "output-pitch", _output_pitch, true },
		{ "output-roll", _output_roll, true },
		{ "output-elevator", _output_elevator, true },
		{ "output-ailerons", _output_ailerons, true },
		{ "output-rudder", _output_rudder, true },
	});

	_elevator_pid.set_i_limit ({ -0.1f, +0.1f });
	_elevator_pid.set_winding (true);
	_ailerons_pid.set_i_limit ({ -0.1f, +0.1f });
	_ailerons_pid.set_winding (true);
	_rudder_pid.set_i_limit ({ -0.1f, +0.1f });

	for (auto pid: { &_manual_pitch_pid, &_manual_roll_pid })
	{
		pid->set_i_limit ({ -0.05f, +0.05f });
		pid->set_winding (true);
	}
}


void
FlyByWire::data_updated()
{
	// Don't process if dt is too small:
	_dt += update_dt();
	if (_dt < 0.005_s)
		return;

	switch (static_cast<Mode> (*_mode))
	{
		case ManualMode:
			_output_elevator.write (-*_input_pitch_axis);
			_output_ailerons.write (*_input_roll_axis);
			_output_rudder.write (*_input_yaw_axis);

			_pre_output_pitch = *_measured_pitch;
			_pre_output_roll = *_measured_roll;
			break;

		case StabilizedMode:
		case FlightDirectorMode:
		{
			using Xefis::Range;

			// Should always be computed:
			integrate_manual_input();

			if (static_cast<Mode> (*_mode) == FlightDirectorMode)
			{
				_pre_output_pitch = *_input_pitch;
				_pre_output_roll = *_input_roll;
			}

			float stabilization_gain = *_stabilization_gain;

			_elevator_pid.set_pid (*_pitch_p, *_pitch_i, *_pitch_d);
			_elevator_pid.set_gain (*_pitch_gain * stabilization_gain);
			_elevator_pid.set_error_power (*_pitch_error_power);
			_elevator_pid.set_output_limit (Range<float> (*_elevator_minimum, *_elevator_maximum));

			_ailerons_pid.set_pid (*_roll_p, *_roll_i, *_roll_d);
			_ailerons_pid.set_gain (*_roll_gain * stabilization_gain);
			_ailerons_pid.set_error_power (*_roll_error_power);
			_ailerons_pid.set_output_limit (Range<float> (*_ailerons_minimum, *_ailerons_maximum));

			_rudder_pid.set_pid (*_yaw_p, *_yaw_i, *_yaw_d);
			_rudder_pid.set_gain (*_yaw_gain * stabilization_gain);
			_rudder_pid.set_error_power (*_yaw_error_power);
			_rudder_pid.set_output_limit (Range<float> (*_rudder_minimum, *_rudder_maximum));

			_elevator_pid.set_target (_pre_output_pitch / 180_deg);
			_elevator_pid.process (*_measured_pitch / 180_deg, _dt.s());

			_ailerons_pid.set_target (_pre_output_roll / 180_deg);
			_ailerons_pid.process (*_measured_roll / 180_deg, _dt.s());

			_rudder_pid.set_target (0.0);
			_rudder_pid.process (*_measured_slip_skid_g, _dt.s());

			_output_elevator.write (-std::cos (*_measured_roll) * _elevator_pid.output());
			_output_ailerons.write (_ailerons_pid.output());

			float yaw_axis = *_input_yaw_axis;
			_output_rudder.write (yaw_axis + (1.0f - yaw_axis) * _rudder_pid.output());
			break;
		}

		default:
			log() << "stabilizer: unknown mode: " << *_mode << std::endl;
			break;
	}

	// Output:
	if (_output_pitch.configured())
		_output_pitch.write (_pre_output_pitch);
	if (_output_roll.configured())
		_output_roll.write (_pre_output_roll);

	_dt = 0_s;
}


void
FlyByWire::integrate_manual_input()
{
	using Xefis::floored_mod;

	// Shortcuts:

	Angle target_pitch_extent = *_pitch_extent;
	Angle target_roll_extent = *_roll_extent;
	float axis_pitch = *_input_pitch_axis;
	float axis_roll = *_input_roll_axis;
	Angle measured_pitch = *_measured_pitch;
	Angle measured_roll = *_measured_roll;

	// Target attitude - computed from current orientation and joystick deflection:

	Angle target_pitch = measured_pitch + std::cos (measured_roll) * axis_pitch * target_pitch_extent;
	Angle target_roll = measured_roll + axis_roll * target_roll_extent;
	target_pitch = floored_mod<float> (target_pitch.deg(), -180.0, +180.0) * 1_deg;
	target_roll = floored_mod<float> (target_roll.deg(), -180.0, +180.0) * 1_deg;

	// Update output attitude:

	_manual_pitch_pid.set_target (target_pitch.deg() / 180.f);
	_manual_pitch_pid.process (_pre_output_pitch.deg() / 180.f, _dt.s());
	_pre_output_pitch += std::abs (axis_pitch) * _manual_pitch_pid.output() * 360_deg;
	_pre_output_pitch = floored_mod<float> (_pre_output_pitch.deg(), -180.0, +180.0) * 1_deg;

	_manual_roll_pid.set_target (target_roll.deg() / 180.f);
	_manual_roll_pid.process (_pre_output_roll.deg() / 180.f, _dt.s());
	_pre_output_roll += std::abs (axis_roll) * _manual_roll_pid.output() * 360_deg;
	_pre_output_roll = floored_mod<float> (_pre_output_roll.deg(), -180.0, +180.0) * 1_deg;

	// Joystick visualisation on EFIS:
	if (_output_control_stick_pitch.configured())
		_output_control_stick_pitch.write (axis_pitch * target_pitch_extent);

	if (_output_control_stick_roll.configured())
		_output_control_stick_roll.write (axis_roll * target_roll_extent);
}


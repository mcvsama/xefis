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
#include <xefis/utility/timestamp.h>

// Local:
#include "stabilizer.h"


Stabilizer::Stabilizer (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager),
	_elevator_pid (0.0, 0.0, 0.0, 0.0),
	_ailerons_pid (0.0, 0.0, 0.0, 0.0),
	_rudder_pid (0.0, 0.0, 0.0, 0.0)
{
	for (QDomElement& e: config)
	{
		if (e == "properties")
		{
			parse_properties (e, {
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
				{ "input-pitch", _input_pitch_deg, true },
				{ "input-roll", _input_roll_deg, true },
				{ "input-yaw-axis", _input_yaw_axis, true },
				{ "measured-pitch", _measured_pitch_deg, true },
				{ "measured-roll", _measured_roll_deg, true },
				{ "measured-slip-skid", _measured_slip_skid_g, true },
				{ "elevator-minimum", _elevator_minimum, true },
				{ "elevator-maximum", _elevator_maximum, true },
				{ "ailerons-minimum", _ailerons_minimum, true },
				{ "ailerons-maximum", _ailerons_maximum, true },
				{ "rudder-minimum", _rudder_minimum, true },
				{ "rudder-maximum", _rudder_maximum, true },
				{ "output-elevator", _output_elevator, true },
				{ "output-ailerons", _output_ailerons, true },
				{ "output-rudder", _output_rudder, true },
			});
		}
	}

	_elevator_pid.set_i_limit ({ -0.1f, +0.1f });
	_elevator_pid.set_winding (true);
	_ailerons_pid.set_i_limit ({ -0.1f, +0.1f });
	_ailerons_pid.set_winding (true);
	_rudder_pid.set_i_limit ({ -0.1f, +0.1f });
}


void
Stabilizer::data_updated()
{
	// Don't process if dt is too small:
	_dt += update_dt();
	if (_dt.seconds() < 0.005)
		return;

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

	_elevator_pid.set_target (*_input_pitch_deg / 180.f);
	_elevator_pid.process (*_measured_pitch_deg / 180.f, _dt.seconds());

	_ailerons_pid.set_target (*_input_roll_deg / 180.f);
	_ailerons_pid.process (*_measured_roll_deg / 180.f, _dt.seconds());

	_rudder_pid.set_target (0.0);
	_rudder_pid.process (*_measured_slip_skid_g, _dt.seconds());

	_output_elevator.write (-std::cos ((*_measured_roll_deg * 1_deg).rad()) * _elevator_pid.output());
	_output_ailerons.write (_ailerons_pid.output());

	float yaw_axis = *_input_yaw_axis;
	_output_rudder.write (yaw_axis + (1.0f - yaw_axis) * _rudder_pid.output());

	_dt = Xefis::Timestamp::from_epoch (0);
}



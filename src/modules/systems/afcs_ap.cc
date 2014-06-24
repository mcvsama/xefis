/* vim:ts=4
 *
 * Copyleft 2012…2014  Michał Gawron
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
#include "afcs_ap.h"


XEFIS_REGISTER_MODULE_CLASS ("systems/afcs-ap", AFCS_AP);


AFCS_AP::AFCS_AP (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config),
	_elevator_pid (0.0, 0.0, 0.0, 0.0),
	_ailerons_pid (0.0, 0.0, 0.0, 0.0)
{
	parse_settings (config, {
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
	});

	parse_properties (config, {
		{ "input.pitch", _input_pitch, true },
		{ "input.roll", _input_roll, true },
		{ "measured.pitch", _measured_pitch, true },
		{ "measured.roll", _measured_roll, true },
		{ "elevator.minimum", _elevator_minimum, true },
		{ "elevator.maximum", _elevator_maximum, true },
		{ "ailerons.minimum", _ailerons_minimum, true },
		{ "ailerons.maximum", _ailerons_maximum, true },
		{ "output.serviceable", _serviceable, true },
		{ "output.elevator", _output_elevator, true },
		{ "output.ailerons", _output_ailerons, true },
	});

	_elevator_pid.set_pid (_pitch_p, _pitch_i, _pitch_d);
	_elevator_pid.set_gain (_pitch_gain * _stabilization_gain);
	_elevator_pid.set_i_limit ({ -0.1f, +0.1f });
	_elevator_pid.set_error_power (_pitch_error_power);
	_elevator_pid.set_winding (true);

	_ailerons_pid.set_pid (_roll_p, _roll_i, _roll_d);
	_ailerons_pid.set_gain (_roll_gain * _stabilization_gain);
	_ailerons_pid.set_i_limit ({ -0.1f, +0.1f });
	_ailerons_pid.set_error_power (_roll_error_power);
	_ailerons_pid.set_winding (true);

	_ap_computer.set_minimum_dt (5_ms);
	_ap_computer.set_callback (std::bind (&AFCS_AP::compute_ap, this));
	_ap_computer.add_depending_smoothers ({
		&_elevator_smoother,
		&_ailerons_smoother,
	});
	_ap_computer.observe ({
		&_input_pitch,
		&_input_roll,
		&_measured_pitch,
		&_measured_roll,
		&_elevator_minimum,
		&_elevator_maximum,
		&_ailerons_minimum,
		&_ailerons_maximum,
	});
}


void
AFCS_AP::data_updated()
{
	_ap_computer.data_updated (update_time());
}


void
AFCS_AP::rescue()
{
	_serviceable.write (false);
}


void
AFCS_AP::compute_ap()
{
	Time update_dt = _ap_computer.update_dt();

	double computed_elevator = 0.0;
	double computed_ailerons = 0.0;

	using Xefis::Range;

	if (_measured_pitch.is_nil() || _measured_roll.is_nil())
	{
		diagnose();
		_serviceable.write (false);
	}
	else
	{
		_elevator_pid.set_output_limit (Range<double> (_elevator_minimum.read (-1.0), _elevator_maximum.read (1.0)));
		_elevator_pid.set_target (_input_pitch.read (0_deg) / 180_deg);
		_elevator_pid.process (*_measured_pitch / 180_deg, update_dt);

		_ailerons_pid.set_output_limit (Range<double> (_ailerons_minimum.read (-1.0), _ailerons_maximum.read (1.0)));
		_ailerons_pid.set_target (_input_roll.read (0_deg) / 180_deg);
		_ailerons_pid.process (*_measured_roll / 180_deg, update_dt);

		computed_elevator = -std::cos (*_measured_roll) * _elevator_pid.output();
		computed_elevator = _elevator_smoother.process (computed_elevator, update_dt);

		computed_ailerons = _ailerons_pid.output();
		computed_ailerons = _ailerons_smoother.process (computed_ailerons, update_dt);

		_serviceable.write (true);
	}

	// Output:
	if (_output_elevator.configured())
		_output_elevator.write (computed_elevator);
	if (_output_ailerons.configured())
		_output_ailerons.write (computed_ailerons);
}


void
AFCS_AP::diagnose()
{
	if (_measured_pitch.is_nil())
		log() << "Measured pitch is nil!" << std::endl;
	if (_measured_roll.is_nil())
		log() << "Measured roll is nil!" << std::endl;
}


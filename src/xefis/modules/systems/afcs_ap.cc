/* vim:ts=4
 *
 * Copyleft 2012…2016  Michał Gawron
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

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/range.h>

// Local:
#include "afcs_ap.h"


AFCS_AP::AFCS_AP (std::string const& instance):
	Module (instance)
{
	constexpr auto radian_second = 1.0_rad * 1.0_s;

	_elevator_pid.set_integral_limit ({ -0.1 * radian_second, +0.1 * radian_second });
	_elevator_pid.set_winding (true);

	_ailerons_pid.set_integral_limit ({ -0.1 * radian_second, +0.1 * radian_second });
	_ailerons_pid.set_winding (true);

	_ap_computer.set_minimum_dt (5_ms);
	_ap_computer.set_callback (std::bind (&AFCS_AP::compute_ap, this));
	_ap_computer.add_depending_smoothers ({
		&_elevator_smoother,
		&_ailerons_smoother,
	});
	_ap_computer.observe ({
		&input_cmd_pitch,
		&input_cmd_roll,
		&input_measured_pitch,
		&input_measured_roll,
		&input_elevator_minimum,
		&input_elevator_maximum,
		&input_ailerons_minimum,
		&input_ailerons_maximum,
	});
}


void
AFCS_AP::initialize()
{
	_elevator_pid.set_pid (*setting_pitch_pid_settings);
	_elevator_pid.set_gain (*setting_overall_gain * *setting_pitch_gain);

	_ailerons_pid.set_pid (*setting_roll_pid_settings);
	_ailerons_pid.set_gain (*setting_overall_gain * *setting_roll_gain);
}


void
AFCS_AP::process (v2::Cycle const& cycle)
{
	_ap_computer.process (cycle.update_time());
}


void
AFCS_AP::rescue (std::exception_ptr)
{
	diagnose();
	output_serviceable = false;
	output_elevator = 0_deg;
	output_ailerons = 0_deg;
}


void
AFCS_AP::compute_ap()
{
	si::Time update_dt = _ap_computer.update_dt();

	si::Angle computed_elevator = 0.0_deg;
	si::Angle computed_ailerons = 0.0_deg;

	if (input_measured_pitch && input_measured_roll &&
		input_elevator_minimum && input_elevator_maximum &&
		input_ailerons_minimum && input_ailerons_maximum)
	{
		_elevator_pid.set_output_limit ({ *input_elevator_minimum, *input_elevator_maximum });
		_elevator_pid (input_cmd_pitch.value_or (*input_measured_pitch), *input_measured_pitch, update_dt);

		_ailerons_pid.set_output_limit ({ *input_ailerons_minimum, *input_ailerons_maximum });
		_ailerons_pid (input_cmd_roll.value_or (*input_measured_roll), *input_measured_roll, update_dt);

		computed_elevator = _elevator_smoother (-si::cos (*input_measured_roll) * _elevator_pid.output(), update_dt);
		computed_ailerons = _ailerons_smoother (_ailerons_pid.output(), update_dt);

		output_serviceable = true;
	}
	else
	{
		diagnose();
		output_serviceable = false;
	}

	output_elevator = computed_elevator;
	output_ailerons = computed_ailerons;
}


void
AFCS_AP::diagnose()
{
	if (!input_measured_pitch)
		log() << "Measured pitch is nil!" << std::endl;

	if (!input_measured_roll)
		log() << "Measured roll is nil!" << std::endl;
}


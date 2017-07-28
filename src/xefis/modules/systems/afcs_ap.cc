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


AFCS_AP::AFCS_AP (std::unique_ptr<AFCS_AP_IO> module_io, std::string const& instance):
	Module (std::move (module_io), instance)
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
		&io.input_cmd_pitch,
		&io.input_cmd_roll,
		&io.input_measured_pitch,
		&io.input_measured_roll,
		&io.input_elevator_minimum,
		&io.input_elevator_maximum,
		&io.input_ailerons_minimum,
		&io.input_ailerons_maximum,
	});
}


void
AFCS_AP::initialize()
{
	_elevator_pid.set_pid (*io.setting_pitch_pid_settings);
	_elevator_pid.set_gain (*io.setting_overall_gain * *io.setting_pitch_gain);

	_ailerons_pid.set_pid (*io.setting_roll_pid_settings);
	_ailerons_pid.set_gain (*io.setting_overall_gain * *io.setting_roll_gain);
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
	io.output_serviceable = false;
	io.output_elevator = 0_deg;
	io.output_ailerons = 0_deg;
}


void
AFCS_AP::compute_ap()
{
	si::Time update_dt = _ap_computer.update_dt();

	si::Angle computed_elevator = 0.0_deg;
	si::Angle computed_ailerons = 0.0_deg;

	if (io.input_measured_pitch && io.input_measured_roll &&
		io.input_elevator_minimum && io.input_elevator_maximum &&
		io.input_ailerons_minimum && io.input_ailerons_maximum)
	{
		_elevator_pid.set_output_limit ({ *io.input_elevator_minimum, *io.input_elevator_maximum });
		_elevator_pid (io.input_cmd_pitch.value_or (*io.input_measured_pitch), *io.input_measured_pitch, update_dt);

		_ailerons_pid.set_output_limit ({ *io.input_ailerons_minimum, *io.input_ailerons_maximum });
		_ailerons_pid (io.input_cmd_roll.value_or (*io.input_measured_roll), *io.input_measured_roll, update_dt);

		computed_elevator = _elevator_smoother (-si::cos (*io.input_measured_roll) * _elevator_pid.output(), update_dt);
		computed_ailerons = _ailerons_smoother (_ailerons_pid.output(), update_dt);

		io.output_serviceable = true;
	}
	else
	{
		diagnose();
		io.output_serviceable = false;
	}

	io.output_elevator = computed_elevator;
	io.output_ailerons = computed_ailerons;
}


void
AFCS_AP::diagnose()
{
	if (!io.input_measured_pitch)
		log() << "Measured pitch is nil!" << std::endl;

	if (!io.input_measured_roll)
		log() << "Measured roll is nil!" << std::endl;
}


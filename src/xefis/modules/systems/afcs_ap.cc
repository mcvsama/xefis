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

// Local:
#include "afcs_ap.h"

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/qt/qdom.h>
#include <neutrino/range.h>

// Standard:
#include <cstddef>


AFCS_AP::AFCS_AP (xf::Logger const& logger, std::string_view const& instance):
	AFCS_AP_IO (instance),
	_logger (logger.with_scope (std::string (kLoggerScope) + "#" + instance))
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
		&_io.cmd_pitch,
		&_io.cmd_roll,
		&_io.measured_pitch,
		&_io.measured_roll,
		&_io.elevator_minimum,
		&_io.elevator_maximum,
		&_io.ailerons_minimum,
		&_io.ailerons_maximum,
	});
}


void
AFCS_AP::initialize()
{
	_elevator_pid.set_pid (*_io.pitch_pid_settings);
	_elevator_pid.set_gain (*_io.overall_gain * *_io.pitch_gain);

	_ailerons_pid.set_pid (*_io.roll_pid_settings);
	_ailerons_pid.set_gain (*_io.overall_gain * *_io.roll_gain);
}


void
AFCS_AP::process (xf::Cycle const& cycle)
{
	_ap_computer.process (cycle.update_time());
}


void
AFCS_AP::rescue (xf::Cycle const& cycle, std::exception_ptr eptr)
{
	using namespace xf::exception_ops;

	diagnose();
	_io.serviceable = false;
	_io.elevator = 0_deg;
	_io.ailerons = 0_deg;
	(cycle.logger() + _logger) << "" << eptr << std::endl;
}


void
AFCS_AP::compute_ap()
{
	si::Time update_dt = _ap_computer.update_dt();

	si::Angle computed_elevator = 0.0_deg;
	si::Angle computed_ailerons = 0.0_deg;

	if (_io.measured_pitch && _io.measured_roll &&
		_io.elevator_minimum && _io.elevator_maximum &&
		_io.ailerons_minimum && _io.ailerons_maximum)
	{
		_elevator_pid.set_output_limit ({ *_io.elevator_minimum, *_io.elevator_maximum });
		_elevator_pid (_io.cmd_pitch.value_or (*_io.measured_pitch), *_io.measured_pitch, update_dt);

		_ailerons_pid.set_output_limit ({ *_io.ailerons_minimum, *_io.ailerons_maximum });
		_ailerons_pid (_io.cmd_roll.value_or (*_io.measured_roll), *_io.measured_roll, update_dt);

		computed_elevator = _elevator_smoother (-si::cos (*_io.measured_roll) * _elevator_pid.output(), update_dt);
		computed_ailerons = _ailerons_smoother (_ailerons_pid.output(), update_dt);

		_io.serviceable = true;
	}
	else
	{
		diagnose();
		_io.serviceable = false;
	}

	_io.elevator = computed_elevator;
	_io.ailerons = computed_ailerons;
}


void
AFCS_AP::diagnose()
{
	if (!_io.measured_pitch)
		_logger << "Measured pitch is nil!" << std::endl;

	if (!_io.measured_roll)
		_logger << "Measured roll is nil!" << std::endl;
}


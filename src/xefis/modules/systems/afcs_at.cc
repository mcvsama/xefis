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
#include "afcs_at.h"

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/numeric.h>
#include <neutrino/qt/qdom.h>
#include <neutrino/range.h>

// Standard:
#include <cstddef>


AFCS_AT::AFCS_AT (std::string_view const& instance):
	AFCS_AT_IO (instance)
{
	_ias_pid.set_integral_limit ({ -5.0_m, +5.0_m });

	_thrust_computer.set_minimum_dt (5_ms);
	_thrust_computer.set_callback (std::bind (&AFCS_AT::compute_thrust, this));
	_thrust_computer.add_depending_smoothers ({
		&_ias_pid_smoother,
	});
	_thrust_computer.observe ({
		&_io.cmd_speed_mode,
		&_io.cmd_thrust,
		&_io.cmd_ias,
		&_io.measured_ias,
	});
}


void
AFCS_AT::initialize()
{
	_ias_pid.set_pid (*_io.ias_pid_settings);
	_ias_pid.set_gain (*_io.ias_pid_gain);
}


void
AFCS_AT::process (xf::Cycle const& cycle)
{
	_thrust_computer.process (cycle.update_time());
}


void
AFCS_AT::compute_thrust()
{
	bool disengage = false;
	si::Force computed_thrust = 0.0_N;
	si::Time dt = _thrust_computer.update_dt();

	if (_io.cmd_speed_mode)
	{
		switch (*_io.cmd_speed_mode)
		{
			case afcs::SpeedMode::Thrust:
				if (_io.cmd_thrust)
					computed_thrust = *_io.cmd_thrust;
				else
					disengage = true;
				break;

			case afcs::SpeedMode::Airspeed:
				if (_io.cmd_ias && _io.measured_ias)
				{
					// This is more tricky, since we measure IAS, but control thrust.
					// There's no 1:1 correlaction between them.
					// TODO use _ias_pid.set_output_limit (...);
					xf::Range output_extents { *_io.output_thrust_minimum, *_io.output_thrust_maximum };
					computed_thrust = xf::clamped (_ias_pid_smoother (_ias_pid (*_io.cmd_ias, *_io.measured_ias, dt), dt),
												   output_extents);
					// TODO make PID control the change rate of thrust, not the thrust directly. Maybe incorporate
					// something into the PIDController object itself? Or create another function-like class.
				}
				else
				{
					_ias_pid.reset();
					_ias_pid_smoother.reset();
					disengage = true;
				}
				break;

			case afcs::SpeedMode::None:
				// Don't change current state:
				break;

			case afcs::SpeedMode::xf_nil_value:
				// Should not happen.
				break;
		}

		_io.thrust = computed_thrust;
	}

	if (disengage || !_io.disengage_at)
		_io.disengage_at = disengage;
}


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
#include <xefis/utility/numeric.h>

// Local:
#include "afcs_fd_roll.h"
#include "afcs_api.h"


AFCS_FD_Roll::AFCS_FD_Roll (std::string const& instance):
	Module (instance)
{
	constexpr auto sec = 1.0_s;

	for (auto* pid: { &_magnetic_hdg_pid, &_magnetic_trk_pid })
	{
		pid->set_integral_limit ({ -5.0_deg * sec, +5.0_deg * sec });
		pid->set_winding (true);
	}

	_roll_computer.set_minimum_dt (5_ms);
	_roll_computer.set_callback (std::bind (static_cast<void (AFCS_FD_Roll::*)()> (&AFCS_FD_Roll::compute_roll), this));
	_roll_computer.add_depending_smoothers ({
		&_output_roll_smoother,
	});
	_roll_computer.observe ({
		&input_autonomous,
		&input_roll_limits,
		&input_cmd_roll_mode,
		&input_cmd_magnetic_hdg,
		&input_cmd_magnetic_trk,
		&input_measured_magnetic_hdg,
		&input_measured_magnetic_trk,
	});
}


void
AFCS_FD_Roll::initialize()
{
	_magnetic_hdg_pid.set_pid (*setting_hdg_pid_settings);
	_magnetic_trk_pid.set_pid (*setting_trk_pid_settings);
}


void
AFCS_FD_Roll::process (x2::Cycle const& cycle)
{
	_roll_computer.process (cycle.update_time());
	check_autonomous();
}


void
AFCS_FD_Roll::rescue (std::exception_ptr)
{
	if (!input_autonomous.value_or (true))
		output_operative = false;

	check_autonomous();
}


void
AFCS_FD_Roll::compute_roll()
{
	si::Time update_dt = _roll_computer.update_dt();
	bool disengage = false;
	Optional<si::Angle> roll;

	// Always compute both PIDs. Use their output only when it's needed.
	Optional<si::Angle> roll_for_hdg = compute_roll (_magnetic_hdg_pid, input_cmd_magnetic_hdg, input_measured_magnetic_hdg, update_dt);
	Optional<si::Angle> roll_for_trk = compute_roll (_magnetic_trk_pid, input_cmd_magnetic_trk, input_measured_magnetic_trk, update_dt);

	// TODO use transistor for output

	auto use_result = [&](Optional<si::Angle> const& use_roll) {
		if (use_roll)
			roll = use_roll;
		else
			disengage = true;
	};

	using afcs_api::RollMode;

	switch (*input_cmd_roll_mode)
	{
		case RollMode::None:
			roll.reset();
			break;

		case RollMode::Heading:
			use_result (roll_for_hdg);
			break;

		case RollMode::Track:
			use_result (roll_for_trk);
			break;

		case RollMode::WingsLevel:
			use_result (0_rad);
			break;

		case RollMode::Localizer:
			// TODO
			roll.reset();
			disengage = true;
			break;

		case RollMode::LNAV:
			// TODO
			roll.reset();
			disengage = true;
			break;

		default:
			roll.reset();
			disengage = true;
			break;
	}

	if (roll)
		output_roll = _output_roll_smoother (*roll, update_dt);
	else
	{
		output_roll.set_nil();
		_output_roll_smoother.reset();
	}

	if (disengage || output_operative.is_nil())
		output_operative = !disengage;

	check_autonomous();
}


Optional<si::Angle>
AFCS_FD_Roll::compute_roll (xf::PIDControl<si::Angle, si::Angle>& pid,
							x2::PropertyIn<si::Angle> const& cmd_direction,
							x2::PropertyIn<si::Angle> const& measured_direction,
							si::Time update_dt) const
{
	xf::Range<si::Angle> roll_limits { -*input_roll_limits, +*input_roll_limits };

	if (cmd_direction && measured_direction)
		return xf::clamped (pid (*cmd_direction, *measured_direction, update_dt), roll_limits);
	else
	{
		pid.reset();
		return {};
	}
}


void
AFCS_FD_Roll::check_autonomous()
{
	if (input_autonomous.value_or (true))
		output_operative = true;
}


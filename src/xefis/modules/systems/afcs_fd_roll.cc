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
#include "afcs_fd_roll.h"
#include "afcs_api.h"

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/numeric.h>
#include <neutrino/qt/qdom.h>

// Standard:
#include <cstddef>


AFCS_FD_Roll::AFCS_FD_Roll (xf::Logger const& logger, std::string_view const& instance):
	AFCS_FD_Roll_IO (instance),
	_logger (logger.with_scope (std::string (kLoggerScope) + "#" + instance))
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
		&_io.autonomous,
		&_io.roll_limits,
		&_io.cmd_roll_mode,
		&_io.cmd_magnetic_hdg,
		&_io.cmd_magnetic_trk,
		&_io.measured_magnetic_hdg,
		&_io.measured_magnetic_trk,
	});
}


void
AFCS_FD_Roll::initialize()
{
	_magnetic_hdg_pid.set_pid (*_io.hdg_pid_settings);
	_magnetic_trk_pid.set_pid (*_io.trk_pid_settings);
}


void
AFCS_FD_Roll::process (xf::Cycle const& cycle)
{
	_roll_computer.process (cycle.update_time());
	check_autonomous();
}


void
AFCS_FD_Roll::rescue (xf::Cycle const& cycle, std::exception_ptr eptr)
{
	using namespace xf::exception_ops;

	if (!_io.autonomous.value_or (true))
		_io.operative = false;

	(cycle.logger() + _logger) << "" << eptr << std::endl;
	check_autonomous();
}


void
AFCS_FD_Roll::compute_roll()
{
	si::Time update_dt = _roll_computer.update_dt();
	bool disengage = false;
	std::optional<si::Angle> roll;

	// Always compute both PIDs. Use their output only when it's needed.
	std::optional<si::Angle> roll_for_hdg = compute_roll (_magnetic_hdg_pid, _io.cmd_magnetic_hdg, _io.measured_magnetic_hdg, update_dt);
	std::optional<si::Angle> roll_for_trk = compute_roll (_magnetic_trk_pid, _io.cmd_magnetic_trk, _io.measured_magnetic_trk, update_dt);

	// TODO use transistor for output

	auto use_result = [&](std::optional<si::Angle> const& use_roll) {
		if (use_roll)
			roll = use_roll;
		else
			disengage = true;
	};

	using afcs::RollMode;

	switch (*_io.cmd_roll_mode)
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
		_io.roll = _output_roll_smoother (*roll, update_dt);
	else
	{
		_io.roll = xf::nil;
		_output_roll_smoother.reset();
	}

	if (disengage || _io.operative.is_nil())
		_io.operative = !disengage;

	check_autonomous();
}


std::optional<si::Angle>
AFCS_FD_Roll::compute_roll (xf::PIDController<si::Angle, si::Angle>& pid,
							xf::ModuleIn<si::Angle> const& cmd_direction,
							xf::ModuleIn<si::Angle> const& measured_direction,
							si::Time update_dt) const
{
	xf::Range roll_limits { -*_io.roll_limits, +*_io.roll_limits };

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
	if (_io.autonomous.value_or (true))
		_io.operative = true;
}


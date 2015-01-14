/* vim:ts=4
 *
 * Copyleft 2012…2015  Michał Gawron
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
#include "afcs_fd_roll.h"
#include "afcs_api.h"


XEFIS_REGISTER_MODULE_CLASS ("systems/afcs-fd-roll", AFCS_FD_Roll);


AFCS_FD_Roll::AFCS_FD_Roll (xf::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	for (auto* pid: { &_magnetic_hdg_pid, &_magnetic_trk_pid })
	{
		pid->set_i_limit ({ -0.05f, +0.05f });
		pid->set_winding (true);
	}

	_output_roll_smoother.set_winding ({ -180.0, +180.0 });

	parse_settings (config, {
		{ "heading.magnetic.pid.p", _hdg_pid_settings.p, false },
		{ "heading.magnetic.pid.i", _hdg_pid_settings.i, false },
		{ "heading.magnetic.pid.d", _hdg_pid_settings.d, false },
		{ "track.magnetic.pid.p", _trk_pid_settings.p, false },
		{ "track.magnetic.pid.i", _trk_pid_settings.i, false },
		{ "track.magnetic.pid.d", _trk_pid_settings.d, false },
	});

	parse_properties (config, {
		{ "autonomous", _autonomous, true },
		{ "roll-limit", _roll_limit, true },
		{ "cmd.roll-mode", _cmd_roll_mode, true },
		{ "cmd.heading.magnetic", _cmd_magnetic_hdg, true },
		{ "cmd.track.magnetic", _cmd_magnetic_trk, true },
		{ "measured.heading.magnetic", _measured_magnetic_hdg, true },
		{ "measured.track.magnetic", _measured_magnetic_trk, true },
		{ "output.roll", _output_roll, true },
		{ "output.operative", _operative, true },
	});

	// Update PID params according to settings:
	_magnetic_hdg_pid.set_pid (_hdg_pid_settings);
	_magnetic_trk_pid.set_pid (_trk_pid_settings);

	roll_mode_changed();

	_roll_computer.set_minimum_dt (5_ms);
	_roll_computer.set_callback (std::bind (static_cast<void (AFCS_FD_Roll::*)()> (&AFCS_FD_Roll::compute_roll), this));
	_roll_computer.add_depending_smoothers ({
		&_output_roll_smoother,
	});
	_roll_computer.observe ({
		&_autonomous,
		&_roll_limit,
		&_cmd_roll_mode,
		&_cmd_magnetic_hdg,
		&_cmd_magnetic_trk,
		&_measured_magnetic_hdg,
		&_measured_magnetic_trk,
	});
}


void
AFCS_FD_Roll::data_updated()
{
	_roll_computer.data_updated (update_time());
	check_autonomous();
}


void
AFCS_FD_Roll::rescue()
{
	if (!_autonomous.read (true))
		_operative.write (false);

	check_autonomous();
}


void
AFCS_FD_Roll::compute_roll()
{
	Time update_dt = _roll_computer.update_dt();
	bool disengage = false;
	Optional<Angle> output_roll;

	if (_cmd_roll_mode.fresh())
		roll_mode_changed();

	// Always compute both PIDs. Use their output only when it's needed.
	Optional<Angle> roll_for_hdg = compute_roll (_magnetic_hdg_pid, _cmd_magnetic_hdg, _measured_magnetic_hdg, update_dt);
	Optional<Angle> roll_for_trk = compute_roll (_magnetic_trk_pid, _cmd_magnetic_trk, _measured_magnetic_trk, update_dt);

	// TODO use transistor for output

	using namespace afcs_api;

	switch (_roll_mode)
	{
		case RollMode::None:
			output_roll.reset();
			break;

		case RollMode::Heading:
			if (roll_for_hdg)
				output_roll = roll_for_hdg;
			else
				disengage = true;
			break;

		case RollMode::Track:
			if (roll_for_trk)
				output_roll = roll_for_trk;
			else
				disengage = true;
			break;

		case RollMode::WingsLevel:
			output_roll = 0_deg;
			break;

		case RollMode::Localizer:
			// TODO
			output_roll.reset();
			disengage = true;
			break;

		case RollMode::LNAV:
			// TODO
			output_roll.reset();
			disengage = true;
			break;

		default:
			output_roll.reset();
			disengage = true;
			break;
	}

	if (output_roll)
		_output_roll.write (1_deg * _output_roll_smoother.process (output_roll->deg(), update_dt));
	else
	{
		_output_roll.set_nil();
		_output_roll_smoother.reset();
	}

	if (disengage || _operative.is_nil())
		_operative.write (!disengage);

	check_autonomous();
}


Optional<Angle>
AFCS_FD_Roll::compute_roll (xf::PIDControl<double>& pid,
							xf::Property<Angle> const& cmd_direction,
							xf::Property<Angle> const& measured_direction,
							Time const& update_dt) const
{
	xf::Range<double> roll_limit { -_roll_limit->deg(), +_roll_limit->deg() };

	if (cmd_direction.is_nil() || measured_direction.is_nil())
	{
		pid.reset();
		return { };
	}
	else
	{
		constexpr xf::Range<double> input_range = { 0.0, 360.0 };
		constexpr xf::Range<double> artificial_range = { -1.0, +1.0 };
		pid.set_target (xf::renormalize (cmd_direction->deg(), input_range, artificial_range));
		pid.process (xf::renormalize (measured_direction->deg(), input_range, artificial_range), update_dt);
		return 1_deg * xf::limit<double> (pid.output() * input_range.mid(), roll_limit);
	}
}


void
AFCS_FD_Roll::roll_mode_changed()
{
	_roll_mode = static_cast<afcs_api::RollMode> (*_cmd_roll_mode);
}


void
AFCS_FD_Roll::check_autonomous()
{
	if (_autonomous.read (true))
		_operative.write (true);
}


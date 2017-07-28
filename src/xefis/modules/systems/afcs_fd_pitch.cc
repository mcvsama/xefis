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
#include "afcs_fd_pitch.h"
#include "afcs_api.h"


AFCS_FD_Pitch::AFCS_FD_Pitch (std::unique_ptr<AFCS_FD_Pitch_IO> module_io, std::string const& instance):
	Module (std::move (module_io), instance)
{
	constexpr auto sec = 1.0_s;

	_ias_pid.set_integral_limit ({ -0.05_m, +0.05_m });
	_mach_pid.set_integral_limit ({ -0.05_s, +0.05_s });
	_altitude_pid.set_integral_limit ({ -0.05_m * sec, +0.05_m * sec });
	_vs_pid.set_integral_limit ({ -0.05_m, +0.05_m });
	_fpa_pid.set_integral_limit ({ -5.0_deg * sec, +5.0_deg * sec });

	_pitch_computer.set_minimum_dt (5_ms);
	_pitch_computer.set_callback (std::bind (static_cast<void (AFCS_FD_Pitch::*)()> (&AFCS_FD_Pitch::compute_pitch), this));
	_pitch_computer.add_depending_smoothers ({
		&_output_pitch_smoother,
	});
	_pitch_computer.observe ({
		&io.input_autonomous,
		&io.input_pitch_limits,
		&io.input_cmd_pitch_mode,
		&io.input_cmd_ias,
		&io.input_cmd_mach,
		&io.input_cmd_alt,
		&io.input_cmd_vs,
		&io.input_cmd_fpa,
		&io.input_measured_ias,
		&io.input_measured_mach,
		&io.input_measured_alt,
		&io.input_measured_vs,
		&io.input_measured_fpa,
	});
}


void
AFCS_FD_Pitch::initialize()
{
	_ias_pid.set_pid (*io.setting_ias_pid_settings);
	_mach_pid.set_pid (*io.setting_mach_pid_settings);
	_altitude_pid.set_pid (*io.setting_altitude_pid_settings);
	_vs_pid.set_pid (*io.setting_vs_pid_settings);
	_fpa_pid.set_pid (*io.setting_fpa_pid_settings);
}


void
AFCS_FD_Pitch::process (v2::Cycle const& cycle)
{
	_pitch_computer.process (cycle.update_time());
	check_autonomous();
}


void
AFCS_FD_Pitch::rescue (std::exception_ptr)
{
	if (!io.input_autonomous.value_or (true))
		io.output_operative = false;

	check_autonomous();
}


void
AFCS_FD_Pitch::compute_pitch()
{
	si::Time update_dt = _pitch_computer.update_dt();
	bool disengage = false;
	std::optional<si::Angle> pitch;

	// Always compute all PIDs. Use their output only when it's needed.

	// TODO Change all Ranges here to settings.
	std::optional<si::Angle> pitch_for_ias = compute_pitch (_ias_pid, io.input_cmd_ias, io.input_measured_ias, update_dt);
	std::optional<si::Angle> pitch_for_mach = compute_pitch (_mach_pid, io.input_cmd_mach, io.input_measured_mach, update_dt);
	std::optional<si::Angle> pitch_for_alt = compute_pitch (_altitude_pid, io.input_cmd_alt, io.input_measured_alt, update_dt);
	std::optional<si::Angle> pitch_for_vs = compute_pitch (_vs_pid, io.input_cmd_vs, io.input_measured_vs, update_dt);
	std::optional<si::Angle> pitch_for_fpa = compute_pitch (_fpa_pid, io.input_cmd_fpa, io.input_measured_fpa, update_dt);
	std::optional<si::Angle> pitch_for_vnavpath;//TODO
	std::optional<si::Angle> pitch_for_gs;//TODO
	std::optional<si::Angle> pitch_for_flare;//TODO

	// TODO use transistor

	auto use_result = [&](std::optional<si::Angle> const& use_pitch) {
		if (use_pitch)
			pitch = use_pitch;
		else
			disengage = true;
	};

	using afcs_api::PitchMode;

	switch (*io.input_cmd_pitch_mode)
	{
		case PitchMode::None:
			pitch.reset();
			break;

		case PitchMode::KIAS:
			use_result (pitch_for_ias);
			break;

		case PitchMode::Mach:
			use_result (pitch_for_mach);
			break;

		case PitchMode::Altitude:
			use_result (pitch_for_alt);
			break;

		case PitchMode::VS:
			use_result (pitch_for_vs);
			break;

		case PitchMode::FPA:
			use_result (pitch_for_fpa);
			break;

		case PitchMode::VNAVPath:
			use_result (pitch_for_vnavpath);
			break;

		case PitchMode::GS:
			use_result (pitch_for_gs);
			break;

		case PitchMode::Flare:
			use_result (pitch_for_flare);
			break;

		default:
			pitch.reset();
			disengage = true;
			break;
	}

	if (pitch)
		io.output_pitch = _output_pitch_smoother (*pitch, update_dt);
	else
	{
		io.output_pitch.set_nil();
		_output_pitch_smoother.reset();
	}

	if (disengage || io.output_operative.is_nil())
		io.output_operative = !disengage;

	check_autonomous();
}


template<class Input, class Control>
	std::optional<si::Angle>
	AFCS_FD_Pitch::compute_pitch (xf::PIDControl<Input, Control>& pid,
								  v2::PropertyIn<Input> const& cmd_param,
								  v2::PropertyIn<Input> const& measured_param,
								  si::Time update_dt) const
	{
		xf::Range<si::Angle> pitch_limits { -*io.input_pitch_limits, +*io.input_pitch_limits };

		if (cmd_param && measured_param)
			return xf::clamped (pid (*cmd_param, *measured_param, update_dt), pitch_limits);
		else
		{
			pid.reset();
			return {};
		}
	}


void
AFCS_FD_Pitch::check_autonomous()
{
	if (io.input_autonomous.value_or (true))
		io.output_operative = true;
}


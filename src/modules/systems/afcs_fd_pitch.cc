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

// Qt:
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/numeric.h>

// Local:
#include "afcs_fd_pitch.h"
#include "afcs_api.h"


XEFIS_REGISTER_MODULE_CLASS ("systems/afcs-fd-pitch", AFCS_FD_Pitch)


AFCS_FD_Pitch::AFCS_FD_Pitch (xf::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	for (auto* pid: { &_ias_pid, &_mach_pid, &_alt_pid, &_vs_pid, &_fpa_pid })
		pid->set_i_limit ({ -0.05f, +0.05f });

	parse_settings (config, {
		{ "ias.pid.p", _ias_pid_settings.p, false },
		{ "ias.pid.i", _ias_pid_settings.i, false },
		{ "ias.pid.d", _ias_pid_settings.d, false },
		{ "mach.pid.p", _mach_pid_settings.p, false },
		{ "mach.pid.i", _mach_pid_settings.i, false },
		{ "mach.pid.d", _mach_pid_settings.d, false },
		{ "altitude.pid.p", _alt_pid_settings.p, false },
		{ "altitude.pid.i", _alt_pid_settings.i, false },
		{ "altitude.pid.d", _alt_pid_settings.d, false },
		{ "vertical-speed.pid.p", _vs_pid_settings.p, false },
		{ "vertical-speed.pid.i", _vs_pid_settings.i, false },
		{ "vertical-speed.pid.d", _vs_pid_settings.d, false },
		{ "fpa.pid.p", _fpa_pid_settings.p, false },
		{ "fpa.pid.i", _fpa_pid_settings.i, false },
		{ "fpa.pid.d", _fpa_pid_settings.d, false },
	});

	parse_properties (config, {
		{ "autonomous", _autonomous, true },
		{ "pitch-limit", _pitch_limit, true },
		{ "cmd.pitch-mode", _cmd_pitch_mode, true },
		{ "cmd.ias", _cmd_ias, true },
		{ "cmd.mach", _cmd_mach, true },
		{ "cmd.altitude", _cmd_alt, true },
		{ "cmd.vertical-speed", _cmd_vs, true },
		{ "cmd.fpa", _cmd_fpa, true },
		{ "measured.ias", _measured_ias, true },
		{ "measured.mach", _measured_mach, true },
		{ "measured.altitude", _measured_alt, true },
		{ "measured.vertical-speed", _measured_vs, true },
		{ "measured.fpa", _measured_fpa, true },
		{ "output.pitch", _output_pitch, true },
		{ "output.operative", _operative, true },
	});

	// Update PID params according to settings:
	_ias_pid.set_pid (_ias_pid_settings);
	_mach_pid.set_pid (_mach_pid_settings);
	_alt_pid.set_pid (_alt_pid_settings);
	_vs_pid.set_pid (_vs_pid_settings);
	_fpa_pid.set_pid (_fpa_pid_settings);

	pitch_mode_changed();

	_pitch_computer.set_minimum_dt (5_ms);
	_pitch_computer.set_callback (std::bind (static_cast<void (AFCS_FD_Pitch::*)()> (&AFCS_FD_Pitch::compute_pitch), this));
	_pitch_computer.add_depending_smoothers ({
		&_output_pitch_smoother,
	});
	_pitch_computer.observe ({
		&_autonomous,
		&_pitch_limit,
		&_cmd_pitch_mode,
		&_cmd_ias,
		&_cmd_mach,
		&_cmd_alt,
		&_cmd_vs,
		&_cmd_fpa,
		&_measured_ias,
		&_measured_mach,
		&_measured_alt,
		&_measured_vs,
		&_measured_fpa,
	});
}


void
AFCS_FD_Pitch::data_updated()
{
	_pitch_computer.data_updated (update_time());
	check_autonomous();
}


void
AFCS_FD_Pitch::rescue()
{
	if (!_autonomous.read (true))
		_operative.write (false);

	check_autonomous();
}


void
AFCS_FD_Pitch::compute_pitch()
{
	Time update_dt = _pitch_computer.update_dt();
	bool disengage = false;
	Optional<Angle> output_pitch;

	if (_cmd_pitch_mode.fresh())
		pitch_mode_changed();

	// Always compute all PIDs. Use their output only when it's needed.
	Optional<Angle> pitch_for_ias = compute_pitch (_ias_pid, _cmd_ias, _measured_ias, { 0.0, 1000.0 }, update_dt);
	Optional<Angle> pitch_for_mach = compute_pitch (_mach_pid, _cmd_mach, _measured_mach, { 0.0, 10.0 }, update_dt);
	Optional<Angle> pitch_for_alt = compute_pitch (_alt_pid, _cmd_alt, _measured_alt, { -10'000.0, +10'000.0 }, update_dt);
	Optional<Angle> pitch_for_vs = compute_pitch (_vs_pid, _cmd_vs, _measured_vs, { -10'000.0, +10'000.0 }, update_dt);
	Optional<Angle> pitch_for_fpa = compute_pitch (_fpa_pid, _cmd_fpa, _measured_fpa, { -90.0, +90.0 }, update_dt);
	Optional<Angle> pitch_for_vnavpath;//TODO
	Optional<Angle> pitch_for_gs;//TODO
	Optional<Angle> pitch_for_flare;//TODO

	// TODO use transistor

	auto use_result = [&](Optional<Angle> const& pitch) {
		if (pitch)
			output_pitch = pitch;
		else
			disengage = true;
	};

	using namespace afcs_api;

	switch (_pitch_mode)
	{
		case PitchMode::None:
			output_pitch.reset();
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
			output_pitch.reset();
			disengage = true;
			break;
	}

	if (output_pitch)
		_output_pitch.write (_output_pitch_smoother (*output_pitch, update_dt));
	else
	{
		_output_pitch.set_nil();
		_output_pitch_smoother.reset();
	}

	if (disengage || _operative.is_nil())
		_operative.write (!disengage);

	check_autonomous();
}


template<class P>
	Optional<Angle>
	AFCS_FD_Pitch::compute_pitch (xf::PIDControl<double>& pid,
								  xf::Property<P> const& cmd_param,
								  xf::Property<P> const& measured_param,
								  xf::Range<double> const& input_range,
								  Time const& update_dt) const
	{
		xf::Range<Angle> pitch_limit { -*_pitch_limit, +*_pitch_limit };

		if (cmd_param.is_nil() || measured_param.is_nil())
		{
			pid.reset();
			return { };
		}
		else
		{
			constexpr xf::Range<double> artificial_range = { -1.0, +1.0 };
			pid.set_target (xf::renormalize (base_quantity (*cmd_param), input_range, artificial_range));
			pid.process (xf::renormalize (base_quantity (*measured_param), input_range, artificial_range), update_dt);

			return xf::clamped<Angle> (1_deg * pid.output() * input_range.mid(), pitch_limit);
		}
	}


void
AFCS_FD_Pitch::pitch_mode_changed()
{
	_pitch_mode = static_cast<afcs_api::PitchMode> (*_cmd_pitch_mode);
}


void
AFCS_FD_Pitch::check_autonomous()
{
	if (_autonomous.read (true))
		_operative.write (true);
}


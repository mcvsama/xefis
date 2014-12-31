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
#include "afcs_fd.h"


XEFIS_REGISTER_MODULE_CLASS ("systems/afcs-fd", AFCS_FD);


AFCS_FD::AFCS_FD (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config),
	_magnetic_heading_pid (_magnetic_heading_pid_p, _magnetic_heading_pid_i, _magnetic_heading_pid_d, 0.0),
	_magnetic_track_pid (_magnetic_track_pid_p, _magnetic_track_pid_i, _magnetic_track_pid_d, 0.0),
	_altitude_pid (_altitude_pid_p, _altitude_pid_i, _altitude_pid_d, 0.0),
	_ias_pid (_ias_pid_p, _ias_pid_i, _ias_pid_d, 0.0),
	_vertical_speed_pid (_vertical_speed_pid_p, _vertical_speed_pid_i, _vertical_speed_pid_d, 0.0),
	_fpa_pid (_fpa_pid_p, _fpa_pid_i, _fpa_pid_d, 0.0)
{
	for (auto* pid: { &_magnetic_heading_pid, &_magnetic_track_pid })
	{
		pid->set_i_limit ({ -0.05f, +0.05f });
		pid->set_winding (true);
	}

	for (auto* pid: { &_altitude_pid, &_ias_pid, &_vertical_speed_pid, &_fpa_pid })
		pid->set_i_limit ({ -0.05f, +0.05f });

	_output_pitch_smoother.set_winding ({ -180.0, +180.0 });
	_output_roll_smoother.set_winding ({ -180.0, +180.0 });

	parse_settings (config, {
		{ "magnetic-heading.pid.p", _magnetic_heading_pid_p, false },
		{ "magnetic-heading.pid.i", _magnetic_heading_pid_i, false },
		{ "magnetic-heading.pid.d", _magnetic_heading_pid_d, false },
		{ "magnetic-track.pid.p", _magnetic_track_pid_p, false },
		{ "magnetic-track.pid.i", _magnetic_track_pid_i, false },
		{ "magnetic-track.pid.d", _magnetic_track_pid_d, false },
		{ "altitude.pid.p", _altitude_pid_p, false },
		{ "altitude.pid.i", _altitude_pid_i, false },
		{ "altitude.pid.d", _altitude_pid_d, false },
		{ "ias.pid.p", _ias_pid_p, false },
		{ "ias.pid.i", _ias_pid_i, false },
		{ "ias.pid.d", _ias_pid_d, false },
		{ "vertical-speed.pid.p", _vertical_speed_pid_p, false },
		{ "vertical-speed.pid.i", _vertical_speed_pid_i, false },
		{ "vertical-speed.pid.d", _vertical_speed_pid_d, false },
		{ "fpa.pid.p", _fpa_pid_p, false },
		{ "fpa.pid.i", _fpa_pid_i, false },
		{ "fpa.pid.d", _fpa_pid_d, false },
	});

	parse_properties (config, {
		{ "orientation.pitch-limit.maximum", _pitch_limit_max, true },
		{ "orientation.pitch-limit.minimum", _pitch_limit_min, true },
		{ "orientation.roll-limit", _roll_limit, true },
		{ "cmd.roll-mode", _cmd_roll_mode, true },
		{ "cmd.pitch-mode", _cmd_pitch_mode, true },
		{ "cmd.heading.magnetic", _cmd_magnetic_heading, true },
		{ "cmd.track.magnetic", _cmd_magnetic_track, true },
		{ "cmd.altitude", _cmd_altitude, true },
		{ "cmd.ias", _cmd_ias, true },
		{ "cmd.vertical-speed", _cmd_vertical_speed, true },
		{ "cmd.fpa", _cmd_fpa, true },
		{ "measured.heading.magnetic", _measured_magnetic_heading, true },
		{ "measured.track.magnetic", _measured_magnetic_track, true },
		{ "measured.altitude", _measured_altitude, true },
		{ "measured.ias", _measured_ias, true },
		{ "measured.vertical-speed", _measured_vertical_speed, true },
		{ "measured.fpa", _measured_fpa, true },
		{ "output.pitch", _output_pitch, true },
		{ "output.roll", _output_roll, true },
		{ "output.operative", _operative, true },
	});

	// Update PID params according to settings:
	_magnetic_heading_pid.set_pid (_magnetic_heading_pid_p, _magnetic_heading_pid_i, _magnetic_heading_pid_d);
	_magnetic_track_pid.set_pid (_magnetic_track_pid_p, _magnetic_track_pid_i, _magnetic_track_pid_d);
	_altitude_pid.set_pid (_altitude_pid_p, _altitude_pid_i, _altitude_pid_d);
	_ias_pid.set_pid (_ias_pid_p, _ias_pid_i, _ias_pid_d);
	_vertical_speed_pid.set_pid (_vertical_speed_pid_p, _vertical_speed_pid_i, _vertical_speed_pid_d);
	_fpa_pid.set_pid (_fpa_pid_p, _fpa_pid_i, _fpa_pid_d);

	roll_mode_changed();
	pitch_mode_changed();

	_fd_computer.set_minimum_dt (5_ms);
	_fd_computer.set_callback (std::bind (&AFCS_FD::compute_fd, this));
	_fd_computer.add_depending_smoothers ({
		&_output_pitch_smoother,
		&_output_roll_smoother,
	});
	_fd_computer.observe ({
		&_pitch_limit_max,
		&_pitch_limit_min,
		&_roll_limit,
		&_cmd_roll_mode,
		&_cmd_pitch_mode,
		&_cmd_magnetic_heading,
		&_cmd_magnetic_track,
		&_cmd_altitude,
		&_cmd_ias,
		&_cmd_vertical_speed,
		&_cmd_fpa,
		&_measured_magnetic_heading,
		&_measured_magnetic_track,
		&_measured_altitude,
		&_measured_ias,
		&_measured_vertical_speed,
		&_measured_fpa,
	});
}


void
AFCS_FD::data_updated()
{
	_fd_computer.data_updated (update_time());
}


void
AFCS_FD::rescue()
{
	_operative.write (false);
}


void
AFCS_FD::compute_fd()
{
	Time update_dt = _fd_computer.update_dt();
	bool disengage = false;

	if (_cmd_roll_mode.fresh())
		roll_mode_changed();

	if (_cmd_pitch_mode.fresh())
		pitch_mode_changed();

	double const altitude_output_scale = 0.10;
	double const vertical_speed_output_scale = 0.01;
	double const rl = _roll_limit->deg();
	double const pl_max = _pitch_limit_max->deg();
	double const pl_min = _pitch_limit_min->deg();
	Xefis::Range<double> roll_limit (-rl, +rl);
	Xefis::Range<double> pitch_limit (pl_min, pl_max);

	switch (_roll_mode)
	{
		case RollMode::Heading:
			if (_cmd_magnetic_heading.is_nil() || _measured_magnetic_heading.is_nil())
			{
				_magnetic_heading_pid.reset();
				disengage = true;
			}
			else
			{
				_magnetic_heading_pid.set_target (Xefis::renormalize (_cmd_magnetic_heading->deg(), 0.0, 360.0, -1.0, +1.0));
				_magnetic_heading_pid.process (Xefis::renormalize (_measured_magnetic_heading->deg(), 0.0, 360.0, -1.0, +1.0), update_dt);
				_computed_output_roll = 1_deg * Xefis::limit<double> (_magnetic_heading_pid.output() * 180.0, roll_limit);
			}
			break;

		case RollMode::Track:
			if (_cmd_magnetic_track.is_nil() || _measured_magnetic_track.is_nil())
			{
				_magnetic_track_pid.reset();
				disengage = true;
			}
			else
			{
				_magnetic_track_pid.set_target (Xefis::renormalize (_cmd_magnetic_track->deg(), 0.0, 360.0, -1.0, +1.0));
				_magnetic_track_pid.process (Xefis::renormalize (_measured_magnetic_track->deg(), 0.0, 360.0, -1.0, +1.0), update_dt);
				_computed_output_roll = 1_deg * Xefis::limit<double> (_magnetic_track_pid.output() * 180.0, roll_limit);
			}
			break;

		case RollMode::None:
		case RollMode::sentinel:
			_computed_output_roll = 0_deg;
			break;
	}

	switch (_pitch_mode)
	{
		case PitchMode::Altitude:
			if (_cmd_altitude.is_nil() || _measured_altitude.is_nil())
			{
				_altitude_pid.reset();
				disengage = true;
			}
			else
			{
				_altitude_pid.set_target (_cmd_altitude->ft());
				_altitude_pid.process (_measured_altitude->ft(), update_dt);
				_computed_output_pitch = 1_deg * Xefis::limit<double> (altitude_output_scale * _altitude_pid.output(), pitch_limit);
			}
			break;

		case PitchMode::Airspeed:
			if (_cmd_ias.is_nil() || _measured_ias.is_nil())
			{
				_ias_pid.reset();
				disengage = true;
			}
			else
			{
				_ias_pid.set_target (_cmd_ias->kt());
				_ias_pid.process (_measured_ias->kt(), update_dt);
				_computed_output_pitch = 1_deg * Xefis::limit<double> (_ias_pid.output(), pitch_limit);
			}
			break;

		case PitchMode::VerticalSpeed:
			if (_cmd_vertical_speed.is_nil() || _measured_vertical_speed.is_nil())
			{
				_vertical_speed_pid.reset();
				disengage = true;
			}
			else
			{
				_vertical_speed_pid.set_target (_cmd_vertical_speed->fpm());
				_vertical_speed_pid.process (_measured_vertical_speed->fpm(), update_dt);
				_computed_output_pitch = 1_deg * Xefis::limit<double> (vertical_speed_output_scale * _vertical_speed_pid.output(), pitch_limit);
			}
			break;

		case PitchMode::FPA:
			if (_cmd_fpa.is_nil() || _measured_fpa.is_nil())
			{
				_fpa_pid.reset();
				disengage = true;
			}
			else
			{
				_fpa_pid.set_target (_cmd_fpa->deg());
				_fpa_pid.process (_measured_fpa->deg(), update_dt);
				_computed_output_pitch = 1_deg * Xefis::limit<double> (_fpa_pid.output(), pitch_limit);
			}
			break;

		case PitchMode::None:
		case PitchMode::sentinel:
			_computed_output_roll = 0_deg;
			break;
	}

	_output_pitch.write (1_deg * _output_pitch_smoother.process (_computed_output_pitch.deg(), update_dt));
	_output_roll.write (1_deg * _output_roll_smoother.process (_computed_output_roll.deg(), update_dt));

	if (disengage || _operative.is_nil())
		_operative.write (!disengage);
}


void
AFCS_FD::roll_mode_changed()
{
	Xefis::PropertyInteger::Type m = Xefis::limit<decltype (m)> (_cmd_roll_mode.read (-1), 0, static_cast<decltype (m)> (RollMode::sentinel) - 1);
	_roll_mode = static_cast<RollMode> (m);
}


void
AFCS_FD::pitch_mode_changed()
{
	Xefis::PropertyInteger::Type m = Xefis::limit<decltype (m)> (_cmd_pitch_mode.read (-1), 0, static_cast<decltype (m)> (PitchMode::sentinel) - 1);
	_pitch_mode = static_cast<PitchMode> (m);
}


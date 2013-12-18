/* vim:ts=4
 *
 * Copyleft 2012…2013  Michał Gawron
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
#include "flight_director.h"


XEFIS_REGISTER_MODULE_CLASS ("systems/flight-director", FlightDirector);


FlightDirector::FlightDirector (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config),
	_magnetic_heading_pid (1.0, 0.1, 0.0, 0.0),
	_magnetic_track_pid (1.0, 0.1, 0.0, 0.0),
	_altitude_pid (1.0, 0.1, 0.0, 0.0),
	_ias_pid (1.0, 0.1, 0.0, 0.0),
	_vertical_speed_pid (1.0, 0.1, 0.0, 0.0),
	_fpa_pid (1.0, 0.1, 0.0, 0.0)
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

	parse_properties (config, {
		{ "orientation.pitch-limit.max", _pitch_limit_max, true },
		{ "orientation.pitch-limit.min", _pitch_limit_min, true },
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
		{ "disengage-ap", _disengage_ap, true },
	});

	roll_mode_changed();
	pitch_mode_changed();
}


void
FlightDirector::data_updated()
{
	bool disengage = false;

	// Don't process if dt is too small:
	_dt += update_dt();
	if (_dt < 5_ms)
		return;

	if (_cmd_roll_mode.fresh())
		roll_mode_changed();

	if (_cmd_pitch_mode.fresh())
		pitch_mode_changed();

	double const altitude_output_scale = 0.10;
	double const vertical_speed_output_scale = 0.01;
	double const rl = (*_roll_limit).deg();
	double const pl_max = (*_pitch_limit_max).deg();
	double const pl_min = (*_pitch_limit_min).deg();
	Xefis::Range<float> roll_limit (-rl, +rl);
	Xefis::Range<float> pitch_limit (pl_min, pl_max);

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
				_magnetic_heading_pid.set_target (Xefis::renormalize ((*_cmd_magnetic_heading).deg(), 0.f, 360.f, -1.f, +1.f));
				_magnetic_heading_pid.process (Xefis::renormalize ((*_measured_magnetic_heading).deg(), 0.f, 360.f, -1.f, +1.f), _dt.s());
				_computed_output_roll = 1_deg * Xefis::limit<float> (_magnetic_heading_pid.output() * 180.f, roll_limit);
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
				_magnetic_track_pid.set_target (Xefis::renormalize ((*_cmd_magnetic_track).deg(), 0.f, 360.f, -1.f, +1.f));
				_magnetic_track_pid.process (Xefis::renormalize ((*_measured_magnetic_track).deg(), 0.f, 360.f, -1.f, +1.f), _dt.s());
				_computed_output_roll = 1_deg * Xefis::limit<float> (_magnetic_track_pid.output() * 180.f, roll_limit);
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
				_altitude_pid.set_target ((*_cmd_altitude).ft());
				_altitude_pid.process ((*_measured_altitude).ft(), _dt.s());
				_computed_output_pitch = 1_deg * Xefis::limit<float> (altitude_output_scale * _altitude_pid.output(), pitch_limit);
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
				_ias_pid.set_target ((*_cmd_ias).kt());
				_ias_pid.process ((*_measured_ias).kt(), _dt.s());
				_computed_output_pitch = 1_deg * Xefis::limit<float> (_ias_pid.output(), pitch_limit);
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
				_vertical_speed_pid.set_target ((*_cmd_vertical_speed).fpm());
				_vertical_speed_pid.process ((*_measured_vertical_speed).fpm(), _dt.s());
				_computed_output_pitch = 1_deg * Xefis::limit<float> (vertical_speed_output_scale * _vertical_speed_pid.output(), pitch_limit);
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
				_fpa_pid.set_target ((*_cmd_fpa).deg());
				_fpa_pid.process ((*_measured_fpa).deg(), _dt.s());
				_computed_output_pitch = 1_deg * Xefis::limit<float> (_fpa_pid.output(), pitch_limit);
			}
			break;

		case PitchMode::None:
		case PitchMode::sentinel:
			_computed_output_roll = 0_deg;
			break;
	}

	_output_pitch.write (1_deg * _output_pitch_smoother.process (_computed_output_pitch.deg(), _dt));
	_output_roll.write (1_deg * _output_roll_smoother.process (_computed_output_roll.deg(), _dt));
	_disengage_ap.write (disengage);

	_dt = 0_s;
}


void
FlightDirector::roll_mode_changed()
{
	Xefis::PropertyInteger::Type m = Xefis::limit<decltype (m)> (_cmd_roll_mode.read (-1), 0, static_cast<decltype (m)> (RollMode::sentinel) - 1);
	_roll_mode = static_cast<RollMode> (m);
}


void
FlightDirector::pitch_mode_changed()
{
	Xefis::PropertyInteger::Type m = Xefis::limit<decltype (m)> (_cmd_pitch_mode.read (-1), 0, static_cast<decltype (m)> (PitchMode::sentinel) - 1);
	_pitch_mode = static_cast<PitchMode> (m);
}


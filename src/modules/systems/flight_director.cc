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


FlightDirector::FlightDirector (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager),
	_magnetic_heading_pid (1.0, 0.1, 0.0, 0.0),
	_magnetic_track_pid (1.0, 0.1, 0.0, 0.0),
	_altitude_pid (1.0, 0.1, 0.0, 0.0),
	_vertical_speed_pid (1.0, 0.1, 0.0, 0.0),
	_fpa_pid (1.0, 0.1, 0.0, 0.0)
{
	for (QDomElement& e: config)
	{
		if (e == "properties")
		{
			parse_properties (e, {
				{ "enabled", _enabled, true },
				{ "lateral-mode", _lateral_mode, true },
				{ "vertical-mode", _vertical_mode, true },
				{ "pitch-limit", _pitch_limit_deg, true },
				{ "roll-limit", _roll_limit_deg, true },
				{ "selected-magnetic-heading", _selected_magnetic_heading_deg, true },
				{ "selected-magnetic-track", _selected_magnetic_track_deg, true },
				{ "selected-altitude", _selected_altitude_ft, true },
				{ "selected-vertical-speed", _selected_vertical_speed_fpm, true },
				{ "selected-fpa", _selected_fpa_deg, true },
				{ "measured-magnetic-heading", _measured_magnetic_heading_deg, true },
				{ "measured-magnetic-track", _measured_magnetic_track_deg, true },
				{ "measured-altitude", _measured_altitude_ft, true },
				{ "measured-vertical-speed", _measured_vertical_speed_fpm, true },
				{ "measured-fpa", _measured_fpa_deg, true },
				{ "output-pitch", _output_pitch_deg, true },
				{ "output-roll", _output_roll_deg, true },
			});
		}
	}

	for (auto* pid: { &_magnetic_heading_pid, &_magnetic_track_pid })
	{
		pid->set_i_limit ({ -0.05f, +0.05f });
		pid->set_winding (true);
	}
	for (auto* pid: { &_altitude_pid, &_vertical_speed_pid, &_fpa_pid })
		pid->set_i_limit ({ -0.05f, +0.05f });
}


void
FlightDirector::data_updated()
{
	// Don't process if dt is too small:
	_dt += update_dt();
	if (_dt.seconds() < 0.005)
		return;

	if (*_enabled)
	{
		double const altitude_output_scale = 0.10;
		double const vertical_speed_output_scale = 0.01;
		double const rl = *_roll_limit_deg;
		double const pl = *_pitch_limit_deg;
		Range<float> roll_limit (-rl, +rl);
		Range<float> pitch_limit (-pl, +pl);

		_magnetic_heading_pid.set_target (renormalize (*_selected_magnetic_heading_deg, 0.f, 360.f, -1.f, +1.f));
		_magnetic_heading_pid.process (renormalize (*_measured_magnetic_heading_deg, 0.f, 360.f, -1.f, +1.f), _dt.seconds());

		_magnetic_track_pid.set_target (renormalize (*_selected_magnetic_track_deg, 0.f, 360.f, -1.f, +1.f));
		_magnetic_track_pid.process (renormalize (*_measured_magnetic_track_deg, 0.f, 360.f, -1.f, +1.f), _dt.seconds());

		_altitude_pid.set_target (*_selected_altitude_ft);
		_altitude_pid.process (*_measured_altitude_ft, _dt.seconds());

		_vertical_speed_pid.set_target (*_selected_vertical_speed_fpm);
		_vertical_speed_pid.process (*_measured_vertical_speed_fpm, _dt.seconds());

		_fpa_pid.set_target (*_selected_fpa_deg);
		_fpa_pid.process (*_measured_fpa_deg, _dt.seconds());

		switch (static_cast<VerticalMode> (*_vertical_mode))
		{
			case VerticalDisabled:
				_output_pitch_deg.write (0.f);
				break;

			case AltitudeHold:
				_output_pitch_deg.write (limit<float> (altitude_output_scale * _altitude_pid.output(), pitch_limit));
				break;

			case VerticalSpeed:
				_output_pitch_deg.write (limit<float> (vertical_speed_output_scale * _vertical_speed_pid.output(), pitch_limit));
				break;

			case FlightPathAngle:
				_output_pitch_deg.write (limit<float> (_fpa_pid.output(), pitch_limit));
				break;
		}

		switch (static_cast<LateralMode> (*_lateral_mode))
		{
			case LateralDisabled:
				_output_roll_deg.write (0.f);
				break;

			case FollowHeading:
				_output_roll_deg.write (limit<float> (_magnetic_heading_pid.output() * 180.f, roll_limit));
				break;

			case FollowTrack:
				_output_roll_deg.write (limit<float> (_magnetic_track_pid.output() * 180.f, roll_limit));
				break;
		}
	}
	else
	{
		_output_pitch_deg.write (0.f);
		_output_roll_deg.write (0.f);
	}

	_dt = Xefis::Timestamp::from_epoch (0);
}


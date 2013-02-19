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
	_heading_pid (1.0, 0.1, 0.0, 0.0),
	_altitude_pid (1.0, 0.1, 0.0, 0.0),
	_cbr_pid (1.0, 0.1, 0.0, 0.0)
{
	for (QDomElement& e: config)
	{
		if (e == "properties")
		{
			parse_properties (e, {
				{ "enabled", _enabled, true },
				{ "pitch-limit", _pitch_limit_deg, true },
				{ "roll-limit", _roll_limit_deg, true },
				{ "selected-mag-heading", _selected_mag_heading_deg, true },
				{ "selected-altitude", _selected_altitude_ft, true },
				{ "selected-climb-rate", _selected_cbr_fpm, true },
				{ "vertical-mode", _vertical_mode, true },
				{ "measured-mag-heading", _measured_mag_heading_deg, true },
				{ "measured-altitude", _measured_altitude_ft, true },
				{ "measured-climb-rate", _measured_cbr_fpm, true },
				{ "output-pitch", _output_pitch_deg, true },
				{ "output-roll", _output_roll_deg, true },
			});
		}
	}

	_heading_pid.set_i_limit ({ -0.05f, +0.05f });
	_heading_pid.set_winding (true);
	for (auto* pid: { &_altitude_pid, &_cbr_pid })
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
		double const alt_output_scale = 0.10;
		double const cbr_output_scale = 0.01;

		float roll_limit = *_roll_limit_deg;
		float pitch_limit = *_pitch_limit_deg;

		_heading_pid.set_target (renormalize (*_selected_mag_heading_deg, 0.0f, 360.f, -1.f, +1.f));
		_heading_pid.process (renormalize (*_measured_mag_heading_deg, 0.0f, 360.f, -1.f, +1.f), _dt.seconds());

		_altitude_pid.set_target (*_selected_altitude_ft);
		_altitude_pid.process (*_measured_altitude_ft, _dt.seconds());

		_cbr_pid.set_target (*_selected_cbr_fpm);
		_cbr_pid.process (*_measured_cbr_fpm, _dt.seconds());

		switch (static_cast<VerticalMode> (*_vertical_mode))
		{
			case AltitudeSet:
				_output_pitch_deg.write (limit<float> (alt_output_scale * _altitude_pid.output(), -pitch_limit, +pitch_limit));
				break;

			case ClimbRateSet:
				_output_pitch_deg.write (limit<float> (cbr_output_scale * _cbr_pid.output(), -pitch_limit, +pitch_limit));
				break;
		}
		_output_roll_deg.write (limit<float> (_heading_pid.output() * 180.0, -roll_limit, +roll_limit));
	}
	else
	{
		_output_pitch_deg.write (0.f);
		_output_roll_deg.write (0.f);
	}

	_dt = Xefis::Timestamp::from_epoch (0);
}


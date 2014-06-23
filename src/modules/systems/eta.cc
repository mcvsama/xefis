/* vim:ts=4
 *
 * Copyleft 2012…2014  Michał Gawron
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
#include <xefis/utility/numeric.h>

// Local:
#include "eta.h"


XEFIS_REGISTER_MODULE_CLASS ("systems/eta", ETA);


ETA::ETA (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	parse_properties (config, {
		{ "input.station.latitude", _input_station_latitude, true },
		{ "input.station.longitude", _input_station_longitude, true },
		{ "input.aircraft.latitude", _input_aircraft_latitude, true },
		{ "input.aircraft.longitude", _input_aircraft_longitude, true },
		{ "input.track.lateral.true", _input_track_lateral_true, true },
		{ "output.eta", _output_eta, true },
	});

	_eta_computer.set_minimum_dt (1_s);
	_eta_computer.set_callback (std::bind (&ETA::compute, this));
	_eta_computer.add_depending_smoothers ({
		&_smoother,
	});
	_eta_computer.observe ({
		&_input_station_latitude,
		&_input_station_longitude,
		&_input_aircraft_latitude,
		&_input_aircraft_longitude,
	});
}


void
ETA::data_updated()
{
	_eta_computer.data_updated (update_time());
}


void
ETA::compute()
{
	struct SetNil
	{
		SetNil (bool reset_prev_distance):
			reset_prev_distance (reset_prev_distance)
		{ }

		bool reset_prev_distance = false;
	};

	Time dt = _eta_computer.update_dt();
	Length distance;

	try {
		if (!_input_station_latitude.valid() || !_input_station_longitude.valid() ||
			!_input_aircraft_latitude.valid() || !_input_aircraft_longitude.valid() ||
			!_input_track_lateral_true.valid())
		{
			throw SetNil (true);
		}

		LonLat station (*_input_station_longitude, *_input_station_latitude);
		LonLat aircraft (*_input_aircraft_longitude, *_input_aircraft_latitude);
		distance = station.haversine_earth (aircraft);

		Angle station_bearing = Xefis::floored_mod (aircraft.initial_bearing (station), 0_deg, 360_deg);
		Angle angle_diff = station_bearing - *_input_track_lateral_true;
		angle_diff = Xefis::floored_mod (angle_diff, -180_deg, +180_deg);

		if (std::abs (angle_diff) > 30_deg)
			throw SetNil (true);

		if (_prev_distance)
		{
			Length distance_diff = distance - *_prev_distance;
			if (distance_diff >= 0_nmi)
			{
				_prev_distance = distance;
				throw SetNil (false);
			}

			Time eta = dt * (-distance / distance_diff);
			_output_eta.write (1_s * _smoother.process (eta.s(), dt));
		}
		else
			_output_eta.set_nil();

		_prev_distance = distance;
	}
	catch (SetNil const& set_nil)
	{
		if (set_nil.reset_prev_distance)
			_prev_distance.reset();

		_output_eta.set_nil();
	}
}


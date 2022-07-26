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
#include "vor.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/earth/earth.h>

// Neutrino:
#include <neutrino/numeric.h>

// Standard:
#include <cstddef>


VOR::VOR (std::string_view const& instance):
	VOR_IO (instance)
{
	_vor_computer.set_callback (std::bind (&VOR::compute, this));
	_vor_computer.add_depending_smoothers ({
		&_deviation_smoother,
	});
	_vor_computer.observe ({
		&_io.input_magnetic_declination,
		&_io.input_station_latitude,
		&_io.input_station_longitude,
		&_io.input_aircraft_latitude,
		&_io.input_aircraft_longitude,
		&_io.input_radial_magnetic,
	});
}


void
VOR::process (xf::Cycle const& cycle)
{
	_vor_computer.process (cycle.update_time());
}


void
VOR::compute()
{
	using std::abs;

	si::Time dt = _vor_computer.update_dt();

	if (_io.input_magnetic_declination &&
		_io.input_station_latitude && _io.input_station_longitude &&
		_io.input_aircraft_latitude && _io.input_aircraft_longitude &&
		_io.input_radial_magnetic)
	{
		// Convert magnetic radial to true radial:
		si::Angle const declination = *_io.input_magnetic_declination;
		si::Angle const input_radial = *_io.input_radial_magnetic + declination;

		si::LonLat const station_position (*_io.input_station_longitude, *_io.input_station_latitude);
		si::LonLat const aircraft_position (*_io.input_aircraft_longitude, *_io.input_aircraft_latitude);

		si::Angle current_radial = normalize (xf::initial_bearing (station_position, aircraft_position));
		si::Angle deviation = xf::floored_mod<si::Angle> (input_radial - current_radial, -180_deg, +180_deg);
		if (abs (deviation) > 90_deg)
			deviation = -denormalize (deviation + 180_deg);

		_io.output_radial_magnetic = normalize (current_radial - declination);
		_io.output_reciprocal_magnetic = normalize (current_radial + 180_deg - declination);
		_io.output_initial_bearing_magnetic = normalize (xf::initial_bearing (aircraft_position, station_position) - declination);
		_io.output_to_flag = abs (denormalize (current_radial - input_radial)) > 90_deg;
		_io.output_deviation = _deviation_smoother (deviation, dt);
		_io.output_distance = xf::haversine_earth (station_position, aircraft_position);
	}
	else
	{
		_io.output_deviation = xf::nil;
		_io.output_to_flag = xf::nil;
	}
}


inline si::Angle
VOR::normalize (si::Angle const a)
{
	return xf::floored_mod<si::Angle> (a, 0_deg, 360_deg);
}


inline si::Angle
VOR::denormalize (si::Angle const a)
{
	return xf::floored_mod<si::Angle> (a, -180_deg, +180_deg);
}


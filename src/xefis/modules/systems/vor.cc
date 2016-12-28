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
#include <xefis/support/navigation/earth.h>
#include <xefis/utility/numeric.h>

// Local:
#include "vor.h"


VOR::VOR (std::string const& instance):
	Module (instance)
{
	_vor_computer.set_callback (std::bind (&VOR::compute, this));
	_vor_computer.add_depending_smoothers ({
		&_deviation_smoother,
	});
	_vor_computer.observe ({
		&input_magnetic_declination,
		&input_station_latitude,
		&input_station_longitude,
		&input_aircraft_latitude,
		&input_aircraft_longitude,
		&input_radial_magnetic,
	});
}


void
VOR::process (x2::Cycle const& cycle)
{
	_vor_computer.process (cycle.update_time());
}


void
VOR::compute()
{
	using std::abs;

	si::Time dt = _vor_computer.update_dt();

	if (input_magnetic_declination &&
		input_station_latitude && input_station_longitude &&
		input_aircraft_latitude && input_aircraft_longitude &&
		input_radial_magnetic)
	{
		// Convert magnetic radial to true radial:
		si::Angle const declination = *input_magnetic_declination;
		si::Angle const input_radial = *input_radial_magnetic + declination;

		si::LonLat const station_position (*input_station_longitude, *input_station_latitude);
		si::LonLat const aircraft_position (*input_aircraft_longitude, *input_aircraft_latitude);

		si::Angle current_radial = normalize (xf::initial_bearing (station_position, aircraft_position));
		si::Angle deviation = xf::floored_mod<Angle> (input_radial - current_radial, -180_deg, +180_deg);
		if (abs (deviation) > 90_deg)
			deviation = -denormalize (deviation + 180_deg);

		output_radial_magnetic = normalize (current_radial - declination);
		output_reciprocal_magnetic = normalize (current_radial + 180_deg - declination);
		output_initial_bearing_magnetic = normalize (xf::initial_bearing (aircraft_position, station_position) - declination);
		output_to_flag = abs (denormalize (current_radial - input_radial)) > 90_deg;
		output_deviation = _deviation_smoother (deviation, dt);
		output_distance = xf::haversine_earth (station_position, aircraft_position);
	}
	else
	{
		output_deviation.set_nil();
		output_to_flag.set_nil();
	}
}


inline si::Angle
VOR::normalize (Angle a)
{
	return xf::floored_mod<si::Angle> (a, 0_deg, 360_deg);
}


inline si::Angle
VOR::denormalize (Angle a)
{
	return xf::floored_mod<si::Angle> (a, -180_deg, +180_deg);
}


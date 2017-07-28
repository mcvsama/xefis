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


VOR::VOR (std::unique_ptr<VOR_IO> module_io, std::string const& instance):
	Module (std::move (module_io), instance)
{
	_vor_computer.set_callback (std::bind (&VOR::compute, this));
	_vor_computer.add_depending_smoothers ({
		&_deviation_smoother,
	});
	_vor_computer.observe ({
		&io.input_magnetic_declination,
		&io.input_station_latitude,
		&io.input_station_longitude,
		&io.input_aircraft_latitude,
		&io.input_aircraft_longitude,
		&io.input_radial_magnetic,
	});
}


void
VOR::process (v2::Cycle const& cycle)
{
	_vor_computer.process (cycle.update_time());
}


void
VOR::compute()
{
	using std::abs;

	si::Time dt = _vor_computer.update_dt();

	if (io.input_magnetic_declination &&
		io.input_station_latitude && io.input_station_longitude &&
		io.input_aircraft_latitude && io.input_aircraft_longitude &&
		io.input_radial_magnetic)
	{
		// Convert magnetic radial to true radial:
		si::Angle const declination = *io.input_magnetic_declination;
		si::Angle const input_radial = *io.input_radial_magnetic + declination;

		si::LonLat const station_position (*io.input_station_longitude, *io.input_station_latitude);
		si::LonLat const aircraft_position (*io.input_aircraft_longitude, *io.input_aircraft_latitude);

		si::Angle current_radial = normalize (xf::initial_bearing (station_position, aircraft_position));
		si::Angle deviation = xf::floored_mod<Angle> (input_radial - current_radial, -180_deg, +180_deg);
		if (abs (deviation) > 90_deg)
			deviation = -denormalize (deviation + 180_deg);

		io.output_radial_magnetic = normalize (current_radial - declination);
		io.output_reciprocal_magnetic = normalize (current_radial + 180_deg - declination);
		io.output_initial_bearing_magnetic = normalize (xf::initial_bearing (aircraft_position, station_position) - declination);
		io.output_to_flag = abs (denormalize (current_radial - input_radial)) > 90_deg;
		io.output_deviation = _deviation_smoother (deviation, dt);
		io.output_distance = xf::haversine_earth (station_position, aircraft_position);
	}
	else
	{
		io.output_deviation.set_nil();
		io.output_to_flag.set_nil();
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


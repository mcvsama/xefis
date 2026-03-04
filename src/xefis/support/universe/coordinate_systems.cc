/* vim:ts=4
 *
 * Copyleft 2026  Michał Gawron
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
#include "coordinate_systems.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/universe/julian_calendar.h>
#include <xefis/support/universe/earth/utility.h>

// Neutrino:
#include <neutrino/numeric.h>

// Standard:
#include <cstddef>
#include <cmath>


namespace xf {

SpaceLength<ECEFSpace>
EclipticCoordinates::to_ecef (double const days_since_j2000) const
{
	auto const obliquity = 23.4393_deg - 0.0000003563_deg * days_since_j2000;

	auto const cos_lat = cos (this->latitude);
	auto const x_ecl = cos (this->longitude) * cos_lat;
	auto const y_ecl = sin (this->longitude) * cos_lat;
	auto const z_ecl = sin (this->latitude);

	auto const x_eq = x_ecl;
	auto const y_eq = y_ecl * cos (obliquity) - z_ecl * sin (obliquity);
	auto const z_eq = y_ecl * sin (obliquity) + z_ecl * cos (obliquity);

	auto const julian_date = days_since_j2000 + kJ2000Epoch;
	auto const gmst = compute_greenwich_mean_sidereal_time_at_0h_ut (julian_date);
	auto const x = +cos (gmst) * x_eq + sin (gmst) * y_eq;
	auto const y = -sin (gmst) * x_eq + cos (gmst) * y_eq;
	auto const z = z_eq;

	return { this->distance_from_earth * x, this->distance_from_earth * y, this->distance_from_earth * z };
}


SpaceLength<ECEFSpace>
EclipticCoordinates::to_ecef_from_unix_time (si::Time const time) const
{
	auto const julian_date = unix_time_to_julian_date (time);
	auto const days_since_j2000 = julian_date - kJ2000Epoch;
	return to_ecef (days_since_j2000);
}

} // namespace xf

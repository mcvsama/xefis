/* vim:ts=4
 *
 * Copyleft 2025  Michał Gawron
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
#include "sun_position.h"

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/numeric.h>

// Standard:
#include <cstddef>


namespace xf {

EclipticCoordinates
calculate_sun_ecliptic_position (double const days_since_J2000)
{
	// These calculations are based on <https://en.wikipedia.org/wiki/Position_of_the_Sun#Approximate_position>.

	auto const n = days_since_J2000;
	// Starting with Ecliptic coordinate system:
	// The mean longitude of the Sun, corrected for the aberration of light:
	auto const L_full_range = 280.460_deg + 0.9856474_deg * n;
	// The mean anomaly of the Sun:
	auto const g_full_range = 357.528_deg + 0.9856003_deg * n;
	// Put L and g in range 0…360°:
	auto const L = floored_mod (L_full_range, 360_deg);
	auto const g = floored_mod (g_full_range, 360_deg);

	return {
		.longitude = L + 1.915_deg * sin (g) + 0.020_deg * sin (2 * g),
		.latitude = 0_deg,
		.distance_from_earth = 1.00014 - 0.01671 * cos (g) - 0.00014 * cos (2 * g),
	};
}


EquatorialCoordinates
calculate_sun_equatorial_position (si::Angle const& ecliptic_longitude, double const days_since_J2000)
{
	// These calculations are based on <https://en.wikipedia.org/wiki/Position_of_the_Sun#Approximate_position>.

	auto const lambda = ecliptic_longitude;
	auto const sin_lambda = sin (lambda);
	// Approximate obliquity of the ecliptic:
	auto const ecliptic_obliquity = 23.439_deg - 0.0000004_deg * days_since_J2000;
	// Convert to equatorial coordinates:
	return {
		.right_ascension = 1_rad * atan2 (cos (ecliptic_obliquity) * sin_lambda, cos (lambda)),
		.declination = 1_rad * asin (sin (ecliptic_obliquity) * sin_lambda),
	};
}

} // namespace xf


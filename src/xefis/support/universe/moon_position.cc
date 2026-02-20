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
#include "moon_position.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/nature/constants.h>
#include <xefis/support/universe/earth/utility.h>
#include <xefis/support/universe/julian_calendar.h>

// Neutrino:
#include <neutrino/numeric.h>

// Standard:
#include <cstddef>


namespace xf {

[[nodiscard]]
constexpr si::Angle
normalize_angle_360 (si::Angle const angle)
{
	return nu::floored_mod (angle, 360_deg);
}


[[nodiscard]]
EclipticCoordinates
compute_moon_ecliptic_position (si::Time const time)
{
	// Low-cost analytical model with major periodic corrections.
	// Formula set follows the classic Schlyter approximation, anchored to Julian date.
	auto const julian_date = unix_time_to_julian_date (time);
	auto const d = julian_date - 2451543.5;

	auto const ascending_node = normalize_angle_360 (125.1228_deg - 0.0529538083_deg * d);
	auto const inclination = 5.1454_deg;
	auto const arg_perigee = normalize_angle_360 (318.0634_deg + 0.1643573223_deg * d);
	auto constexpr a_earth_radii = 60.2666;
	auto constexpr eccentricity = 0.054900;
	auto const mean_anomaly = normalize_angle_360 (115.3654_deg + 13.0649929509_deg * d);

	auto const eccentric_anomaly = mean_anomaly + 1_rad * eccentricity * sin (mean_anomaly) * (1.0 + eccentricity * cos (mean_anomaly));
	auto const x_orbit = a_earth_radii * (cos (eccentric_anomaly) - eccentricity);
	auto const y_orbit = a_earth_radii * std::sqrt (1.0 - eccentricity * eccentricity) * sin (eccentric_anomaly);
	auto const true_anomaly = 1_rad * std::atan2 (y_orbit, x_orbit);
	auto distance_earth_radii = std::sqrt (x_orbit * x_orbit + y_orbit * y_orbit);

	auto const argument_of_latitude = true_anomaly + arg_perigee;
	auto const x_ecliptic = distance_earth_radii * (cos (ascending_node) * cos (argument_of_latitude) - sin (ascending_node) * sin (argument_of_latitude) * cos (inclination));
	auto const y_ecliptic = distance_earth_radii * (sin (ascending_node) * cos (argument_of_latitude) + cos (ascending_node) * sin (argument_of_latitude) * cos (inclination));
	auto const z_ecliptic = distance_earth_radii * sin (argument_of_latitude) * sin (inclination);
	auto longitude = 1_rad * std::atan2 (y_ecliptic, x_ecliptic);
	auto latitude = 1_rad * std::atan2 (z_ecliptic, std::hypot (x_ecliptic, y_ecliptic));

	auto const sun_mean_anomaly = normalize_angle_360 (356.0470_deg + 0.9856002585_deg * d);
	auto const sun_mean_longitude = normalize_angle_360 (280.460_deg + 0.98564736_deg * d);
	auto const moon_mean_longitude = normalize_angle_360 (ascending_node + arg_perigee + mean_anomaly);
	auto const elongation = normalize_angle_360 (moon_mean_longitude - sun_mean_longitude);
	auto const latitude_argument = normalize_angle_360 (moon_mean_longitude - ascending_node);

	longitude +=
		-1.274_deg * sin (mean_anomaly - 2 * elongation)
		+0.658_deg * sin (2 * elongation)
		-0.186_deg * sin (sun_mean_anomaly)
		-0.059_deg * sin (2 * mean_anomaly - 2 * elongation)
		-0.057_deg * sin (mean_anomaly - 2 * elongation + sun_mean_anomaly)
		+0.053_deg * sin (mean_anomaly + 2 * elongation)
		+0.046_deg * sin (2 * elongation - sun_mean_anomaly)
		+0.041_deg * sin (mean_anomaly - sun_mean_anomaly)
		-0.035_deg * sin (elongation)
		-0.031_deg * sin (mean_anomaly + sun_mean_anomaly)
		-0.015_deg * sin (2 * latitude_argument - 2 * elongation)
		+0.011_deg * sin (mean_anomaly - 4 * elongation);

	latitude +=
		-0.173_deg * sin (latitude_argument - 2 * elongation)
		-0.055_deg * sin (mean_anomaly - latitude_argument - 2 * elongation)
		-0.046_deg * sin (mean_anomaly + latitude_argument - 2 * elongation)
		+0.033_deg * sin (latitude_argument + 2 * elongation)
		+0.017_deg * sin (2 * mean_anomaly + latitude_argument);

	distance_earth_radii +=
		-0.58 * cos (mean_anomaly - 2 * elongation)
		-0.46 * cos (2 * elongation);

	return {
		.longitude = normalize_angle_360 (longitude),
		.latitude = latitude,
		.distance_from_earth = distance_earth_radii * kEarthMeanRadius,
	};
}


[[nodiscard]]
SpaceLength<WorldSpace>
compute_moon_position_in_ecef (si::Time const time)
{
	auto const moon = compute_moon_ecliptic_position (time);
	auto const julian_date = unix_time_to_julian_date (time);
	auto const days_since_j2000 = julian_date - kJ2000Epoch;
	auto const obliquity = 23.4393_deg - 0.0000003563_deg * days_since_j2000;

	auto const cos_lat = cos (moon.latitude);
	auto const x_ecl = cos (moon.longitude) * cos_lat;
	auto const y_ecl = sin (moon.longitude) * cos_lat;
	auto const z_ecl = sin (moon.latitude);

	auto const x_eq = x_ecl;
	auto const y_eq = y_ecl * cos (obliquity) - z_ecl * sin (obliquity);
	auto const z_eq = y_ecl * sin (obliquity) + z_ecl * cos (obliquity);

	auto const gmst = compute_greenwich_mean_sidereal_time_at_0h_ut (julian_date);
	auto const x = +cos (gmst) * x_eq + sin (gmst) * y_eq;
	auto const y = -sin (gmst) * x_eq + cos (gmst) * y_eq;
	auto const z = z_eq;

	return { moon.distance_from_earth * x, moon.distance_from_earth * y, moon.distance_from_earth * z };
}


} // namespace xf

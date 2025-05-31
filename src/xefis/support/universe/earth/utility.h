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

#ifndef XEFIS__SUPPORT__UNIVERSE__EARTH__UTILITY_H__INCLUDED
#define XEFIS__SUPPORT__UNIVERSE__EARTH__UTILITY_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/geometry.h>
#include <xefis/support/nature/constants.h>
#include <xefis/support/universe/julian_calendar.h>

// Standard:
#include <cstddef>


namespace xf {

/**
 * Compute distance between two sets of coordinates on Earth.
 * Result is in sphere radius units.
 */
[[nodiscard]]
si::Angle::Value
haversine (si::LonLat const& a, si::LonLat const& b);


/**
 * Convenience function.
 * Compute distance between two sets of coordinates on Earth.
 * Result is in nautical miles.
 */
[[nodiscard]]
inline si::Length
haversine_earth (si::LonLat const& a, si::LonLat const& b)
{
	return haversine (a, b) * kEarthMeanRadius;
}


/**
 * Initial bearing when flying orthodrome (great circle path)
 * to another point. For final bearing, reverse the arguments.
 * Result is in range [-180_deg, +180_deg].
 */
[[nodiscard]]
si::Angle
initial_bearing (si::LonLat const& a, si::LonLat const& b);


/**
 * Compute angle between two great arcs on a sphere.
 * Arcs are given by three points, the second one lies on the intersection.
 * Result is in degrees.
 */
[[nodiscard]]
si::Angle
great_arcs_angle (si::LonLat const& a, si::LonLat const& common, si::LonLat const& b);


[[nodiscard]]
std::string
to_dms (si::Angle, bool three_digits);


[[nodiscard]]
std::string
to_latitude_dms (si::Angle);


[[nodiscard]]
std::string
to_longitude_dms (si::Angle);


/**
 * Mean value for two angles on a circle.
 */
[[nodiscard]]
si::Angle
mean (si::Angle a, si::Angle b);


/**
 * Calculate Greenwich Mean Sidereal Time (GMST) at 0h UT.
 */
[[nodiscard]]
constexpr si::Angle
calculate_greenwich_mean_sidereal_time_at_0h_ut (double const julian_date)
{
	auto const midnight = std::floor (julian_date) + 0.5;
	auto const days_since_midnight = julian_date - midnight;
	auto const hours_since_midnight = 24.0 * days_since_midnight;
	auto const days_since_epoch = julian_date - kJ2000Epoch;
	auto const centuries_since_epoch = days_since_epoch / 36525.0;
	auto const whole_days_since_epoch = midnight - kJ2000Epoch;
	// See <https://aa.usno.navy.mil/faq/GAST>.
	auto const gmst_hours = 6.697374558
		+ 0.065707485828 * whole_days_since_epoch
		+ 1.00273790935 * hours_since_midnight
		+ 0.0854103 * centuries_since_epoch
		+ 0.0000258 * neutrino::square (centuries_since_epoch);
	return 15_deg * floored_mod (gmst_hours, 24.0);
}


/**
 * Calculate Greenwich Mean Sidereal Time (GMST) at 0h UT.
 */
[[nodiscard]]
constexpr si::Angle
unix_time_to_greenwich_mean_sidereal_time_at_0h_ut (si::Time const unix_time)
{
	auto const jd = unix_time_to_julian_date (unix_time);
	return calculate_greenwich_mean_sidereal_time_at_0h_ut (jd);
}


/**
 * Calculate Greenwich Mean Sidereal Time (GMST).
 */
[[nodiscard]]
constexpr si::Angle
unix_time_to_local_sidereal_time (si::Time const unix_time, si::Angle const observer_longitude)
{
	return unix_time_to_greenwich_mean_sidereal_time_at_0h_ut (unix_time) + observer_longitude;
}


/**
 * Build quaternion for ECEF→Celestial rotation.
 */
[[nodiscard]]
RotationQuaternion<WorldSpace>
calculate_ecef_to_celestial_rotation (double const julian_date);

} // namespace xf

#endif


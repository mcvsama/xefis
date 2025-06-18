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

#ifndef XEFIS__SUPPORT__UNIVERSE__SUN_POSITION_H__INCLUDED
#define XEFIS__SUPPORT__UNIVERSE__SUN_POSITION_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/universe/coordinate_systems.h>

// Neutrino:
#include <neutrino/numeric.h>

// Standard:
#include <cstddef>
#include <tuple>


namespace xf {

[[nodiscard]]
EclipticCoordinates
compute_sun_ecliptic_position (double days_since_J2000);


[[nodiscard]]
EquatorialCoordinates
compute_sun_equatorial_position (si::Angle const& ecliptic_longitude, double days_since_J2000);


[[nodiscard]]
HorizontalCoordinates
compute_sun_horizontal_position (si::Angle sun_declination, si::Angle observer_latitude, si::Angle hour_angle);


[[nodiscard]]
si::Angle
compute_sun_altitude (si::Angle sun_declination, si::Angle observer_latitude, si::Angle hour_angle);


[[nodiscard]]
si::Angle
compute_sun_azimuth (si::Angle sun_declination, si::Angle observer_latitude, si::Angle hour_angle, si::Angle sun_altitude);


/**
 * Calculate sunrise and sunset hour-angles.
 * If the function returns NaN, the Sun never rises (polar night) or never sets (midnight Sun).
 *
 * \returns	[sunrise_hour_angle, sunset_hour_angle]
 *			sunset_hour_angle is negative, sunset_hour_angle is positive.
 */
[[nodiscard]]
constexpr std::tuple<si::Angle, si::Angle>
compute_sunrise_and_sunset_hour_angles (si::Angle const sun_declination, si::Angle const observer_latitude)
{
	auto const arg = -tan (observer_latitude) * tan (sun_declination);
	auto const ha = 1_rad * acos (std::clamp (arg, -1.0, +1.0));
	return { -ha, +ha };
}


/**
 * Calculate maximum altitude of the Sun which occurs at solar noon (when hour angle is 0°).
 */
[[nodiscard]]
constexpr si::Angle
compute_solar_noon_altitude (si::Angle const sun_declination, si::Angle const observer_latitude)
{
	return 90_deg - abs (observer_latitude - sun_declination);
}


/**
 * Calculate hour angle.
 */
[[nodiscard]]
constexpr si::Angle
compute_hour_angle (si::Angle const local_sidereal_time, si::Angle const sun_right_ascension)
{
	return floored_mod (local_sidereal_time - sun_right_ascension, 360_deg);
}

} // namespace xf

#endif


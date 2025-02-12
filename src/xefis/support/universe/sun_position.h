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

// Neutrino:
#include <neutrino/numeric.h>

// Standard:
#include <cstddef>


namespace xf {

// This is the standard Julian Date for the Unix epoch (1970-01-01 00:00:00 UTC):
constexpr auto kJulianDateForUnixEpoch = 2440587.5;

// This is the standard Julian Date for the J2000.0 epoch:
constexpr auto kJ2000Epoch = 2451545.0;


struct EclipticCoordinates
{
	// Ecliptic longitude of the Sun (also known as lambda):
	si::Angle	longitude;

	// Ecliptic latitude of the Sun (always nearly 0°) (AKA beta):
	si::Angle	latitude;

	// Distance of the Sun from the Earth (in astronomical units) (also known as R):
	double		distance_from_earth;
};


struct EquatorialCoordinates
{
	// The angular distance measured eastward along the celestial equator from the vernal equinox:
	si::Angle	right_ascension;

	// The angular distance north (positive) or south (negative) of the celestial equator
	// (represents the Sun's position relative to Earth's equator):
	si::Angle	declination;
};


struct HorizontalCoordinates
{
	// The angle between the object (e.g., the Sun) and the observer's local horizon.
	// Positive values indicate the object is above the horizon.
	si::Angle	altitude;

	// The direction of the object along the horizon, typically measured clockwise from true north:
	si::Angle	azimuth;
};


[[nodiscard]]
EclipticCoordinates
calculate_sun_ecliptic_position (double days_since_J2000);


[[nodiscard]]
EquatorialCoordinates
calculate_sun_equatorial_position (si::Angle const& ecliptic_longitude, double days_since_J2000);


[[nodiscard]]
HorizontalCoordinates
calculate_sun_horizontal_position (si::Angle sun_declination, si::Angle observer_latitude, si::Angle hour_angle);


[[nodiscard]]
si::Angle
calculate_sun_altitude (si::Angle sun_declination, si::Angle observer_latitude, si::Angle hour_angle);


[[nodiscard]]
si::Angle
calculate_sun_azimuth (si::Angle sun_declination, si::Angle observer_latitude, si::Angle hour_angle, si::Angle sun_altitude);


/**
 * Convert Unix time to Julian Date.
 */
[[nodiscard]]
constexpr double
unix_time_to_julian_date (si::Time const unix_time)
{
	return unix_time / 86400_s + kJulianDateForUnixEpoch;
}


/**
 * Calculate Julian centuries elapsed since the J2000.0 epoch.
 */
[[nodiscard]]
constexpr double
unix_time_to_julian_century (si::Time const unix_time)
{
	return (unix_time_to_julian_date (unix_time) - kJ2000Epoch) / 36525.0;
}


/**
 * Calculate number of days since the J2000 epoch
 * (Greenwich noon, Terrestrial Time, on 1 January 2000).
 */
[[nodiscard]]
constexpr double
unix_time_to_days_since_J2000 (si::Time const unix_time)
{
	return unix_time_to_julian_date (unix_time) - kJ2000Epoch;
}


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
 * Calculate hour angle.
 */
[[nodiscard]]
constexpr si::Angle
calculate_hour_angle (si::Angle const local_sidereal_time, si::Angle const sun_right_ascension)
{
	return floored_mod (local_sidereal_time - sun_right_ascension, 360_deg);
}

} // namespace xf

#endif


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

#ifndef XEFIS__SUPPORT__UNIVERSE__JULIAN_CALENDAR_H__INCLUDED
#define XEFIS__SUPPORT__UNIVERSE__JULIAN_CALENDAR_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>


namespace xf {

// This is the standard Julian Date for the Unix epoch (1970-01-01 00:00:00 UTC):
constexpr auto kJulianDateForUnixEpoch = 2440587.5;

// This is the standard Julian Date for the J2000.0 epoch:
constexpr auto kJ2000Epoch = 2451545.0;


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

} // namespace xf

#endif


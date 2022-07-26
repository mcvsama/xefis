/* vim:ts=4
 *
 * Copyleft 2008…2013  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__EARTH__AIR_H__INCLUDED
#define XEFIS__SUPPORT__EARTH__AIR_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/numeric.h>

// Standard:
#include <cstddef>


namespace xf {

inline si::Velocity
speed_of_sound (si::Temperature static_air_temperature)
{
	return 38.967854_kt * std::sqrt (static_air_temperature.in<si::Kelvin>());
}


inline si::Length
density_altitude (si::Length pressure_altitude, si::Temperature static_air_temperature)
{
	float t_s = 273.15 + (15.0 - (0.0019812 * pressure_altitude.in<si::Foot>()));
	return pressure_altitude + 1_ft * (t_s / 0.0019812) * (1.0 - std::pow (t_s / static_air_temperature.in<si::Kelvin>(), 0.2349690));
}


inline si::Velocity
true_airspeed (si::Velocity indicated_airspeed, si::Length density_altitude)
{
	return indicated_airspeed / std::pow (1.0 - 6.8755856 * 1e-6 * density_altitude.in<si::Foot>(), 2.127940);
}


inline si::Velocity
indicated_airspeed (si::Velocity true_airspeed, si::Length density_altitude)
{
	return true_airspeed * std::pow (1.0 - 6.8755856 * 1e-6 * density_altitude.in<si::Foot>(), 2.127940);
}

} // namespace xf

#endif


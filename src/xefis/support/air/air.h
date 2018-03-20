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

#ifndef XEFIS__SUPPORT__AIR__AIR_H__INCLUDED
#define XEFIS__SUPPORT__AIR__AIR_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/datatable2d.h>


namespace xf {

inline Speed
compute_sound_speed (Temperature static_air_temperature)
{
	return 38.967854_kt * std::sqrt (static_air_temperature.in<Kelvin>());
}


inline Length
compute_density_altitude (Length pressure_altitude, Temperature static_air_temperature)
{
	float t_s = 273.15 + (15.0 - (0.0019812 * pressure_altitude.in<Foot>()));
	return pressure_altitude + 1_ft * (t_s / 0.0019812) * (1.0 - std::pow (t_s / static_air_temperature.in<Kelvin>(), 0.2349690));
}


inline Speed
compute_true_airspeed (Speed indicated_airspeed, Length density_altitude)
{
	return indicated_airspeed / std::pow (1.0 - 6.8755856 * 1e-6 * density_altitude.in<Foot>(), 2.127940);
}


inline Speed
compute_indicated_airspeed (Speed true_airspeed, Length density_altitude)
{
	return true_airspeed * std::pow (1.0 - 6.8755856 * 1e-6 * density_altitude.in<Foot>(), 2.127940);
}


xf::Datatable2D<si::Temperature, si::DynamicViscosity> const&
temperature_to_dynamic_viscosity();

} // namespace xf

#endif


/* vim:ts=4
 *
 * Copyleft 2024  Michał Gawron
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
#include <xefis/support/math/placement.h>

// Neutrino:
#include <neutrino/math/math.h>
#include <neutrino/numeric.h>

// Standard:
#include <cstddef>


namespace xf {

class Atmosphere;


template<class Space>
	struct Air
	{
		si::Density							density;
		si::Pressure						pressure;
		si::Temperature						temperature;
		si::DynamicViscosity				dynamic_viscosity;
		si::Velocity						speed_of_sound;
		SpaceVector<si::Velocity, Space>	velocity;
	};


template<class TargetSpace, class SourceSpace>
	constexpr Air<TargetSpace>
	operator* (RotationQuaternion<TargetSpace, SourceSpace> const& rotation, Air<SourceSpace> const& air)
	{
		return {
			.density = air.density,
			.pressure = air.pressure,
			.temperature = air.temperature,
			.dynamic_viscosity = air.dynamic_viscosity,
			.speed_of_sound = air.speed_of_sound,
			.velocity = rotation * air.velocity,
		};
	}


[[nodiscard]]
constexpr si::Velocity
speed_of_sound (si::Temperature static_air_temperature)
{
	return 38.967854_kt * std::sqrt (static_air_temperature.in<si::Kelvin>());
}


[[nodiscard]]
constexpr si::Length
density_altitude (si::Length pressure_altitude, si::Temperature static_air_temperature)
{
	float t_s = 273.15 + (15.0 - (0.0019812 * pressure_altitude.in<si::Foot>()));
	return pressure_altitude + 1_ft * (t_s / 0.0019812) * (1.0 - std::pow (t_s / static_air_temperature.in<si::Kelvin>(), 0.2349690));
}


[[nodiscard]]
constexpr si::Velocity
true_airspeed (si::Velocity indicated_airspeed, si::Length density_altitude)
{
	return indicated_airspeed / std::pow (1.0 - 6.8755856 * 1e-6 * density_altitude.in<si::Foot>(), 2.127940);
}


[[nodiscard]]
constexpr si::Velocity
indicated_airspeed (si::Velocity true_airspeed, si::Length density_altitude)
{
	return true_airspeed * std::pow (1.0 - 6.8755856 * 1e-6 * density_altitude.in<si::Foot>(), 2.127940);
}


[[nodiscard]]
constexpr si::Pressure
dynamic_pressure (si::Density const fluid_density, si::Velocity const fluid_speed)
{
	return 0.5 * fluid_density * square (fluid_speed);
}


[[nodiscard]]
si::Pressure
total_pressure (Air<ECEFSpace> const& air,
				SpaceLength<ECEFSpace> const& sensor_normal_vector,
				SpaceVector<si::Velocity, ECEFSpace> sensor_velocity);

/**
 * Normal vector to the sensor's surface is the X axis in the Placement object.
 */
[[nodiscard]]
si::Pressure
total_pressure (Atmosphere const& atmosphere,
				Placement<ECEFSpace> const& placement,
				SpaceVector<si::Velocity, ECEFSpace> const sensor_velocity);

} // namespace xf

#endif


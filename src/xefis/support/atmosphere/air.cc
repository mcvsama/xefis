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

// Local:
#include "air.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/atmosphere/air.h>
#include <xefis/support/atmosphere/atmosphere.h>
#include <xefis/support/math/geometry.h>

// Neutrino:
#include <neutrino/math/field.h>

// Standard:
#include <cstddef>


namespace xf {
namespace {

// Map of temperature <-> dynamic viscosity taken from
// <http://www.engineeringtoolbox.com/air-absolute-kinematic-viscosity-d_601.html>
math::Field<si::Temperature, si::DynamicViscosity> const&
air_temperature_to_dynamic_viscosity()
{
	static auto const kAirTemperatureToDynamicViscosity = math::Field<si::Temperature, si::DynamicViscosity> {
		{  -40_degF, 157.591e-7_Pas },
		{  -20_degF, 159.986e-7_Pas },
		{    0_degF, 157.591e-7_Pas },
		{   10_degF, 164.776e-7_Pas },
		{   20_degF, 167.650e-7_Pas },
		{   30_degF, 171.482e-7_Pas },
		{   40_degF, 172.440e-7_Pas },
		{   50_degF, 176.272e-7_Pas },
		{   60_degF, 179.625e-7_Pas },
		{   70_degF, 182.978e-7_Pas },
		{   80_degF, 184.894e-7_Pas },
		{   90_degF, 186.810e-7_Pas },
		{  100_degF, 188.726e-7_Pas },
		{  120_degF, 192.558e-7_Pas },
		{  140_degF, 197.827e-7_Pas },
		{  160_degF, 202.138e-7_Pas },
		{  180_degF, 207.886e-7_Pas },
		{  200_degF, 215.071e-7_Pas },
		{  300_degF, 238.063e-7_Pas },
		{  400_degF, 250.996e-7_Pas },
		{  500_degF, 277.820e-7_Pas },
		{  750_degF, 326.199e-7_Pas },
		{ 1000_degF, 376.015e-7_Pas },
		{ 1500_degF, 455.050e-7_Pas },
	};

	return kAirTemperatureToDynamicViscosity;
}

} // namespace


si::DynamicViscosity
dynamic_air_viscosity (si::Temperature const temperature)
{
	return air_temperature_to_dynamic_viscosity().extrapolated_value (temperature);
}


si::Pressure
total_pressure (si::Pressure const air_pressure,
				si::Density const air_density,
				SpaceVector<si::Velocity, ECEFSpace> const air_velocity,
				SpaceVector<double, ECEFSpace> const& sensor_normal_vector,
				SpaceVector<si::Velocity, ECEFSpace> const sensor_velocity)
{
	// P_total is Pitot pressure (one that is measured byt the Pitot probe).
	// P_total = P_static + air_density * velocity² / 2
	// The velocity here is True Air Speed.
	auto const sensor_velocity_relative_to_air = sensor_velocity - air_velocity;
	// Assuming sensor_normal_vector is normalized:
	auto const effective_sensor_velocity = projection_onto_normalized (sensor_velocity_relative_to_air, sensor_normal_vector);
	return air_pressure + dynamic_pressure (air_density, abs (effective_sensor_velocity));
}


si::Pressure
total_pressure (Air<ECEFSpace> const& air,
				SpaceVector<double, ECEFSpace> const& sensor_normal_vector,
				SpaceVector<si::Velocity, ECEFSpace> const sensor_velocity)
{
	return total_pressure (air.pressure, air.density, air.velocity, sensor_normal_vector, sensor_velocity);
}


si::Pressure
total_pressure (Atmosphere const& atmosphere,
				Placement<ECEFSpace> const& placement,
				SpaceVector<si::Velocity, ECEFSpace> const sensor_velocity)
{
	auto const air = atmosphere.air_at (placement.position());
	auto const sensor_normal_vector = placement.body_coordinates().x_axis();
	return total_pressure (air, sensor_normal_vector, sensor_velocity);
}

} // namespace xf


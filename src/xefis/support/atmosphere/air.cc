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

// Standard:
#include <cstddef>


namespace xf {

si::Pressure
total_pressure (Air<ECEFSpace> const& air,
				SpaceVector<double, ECEFSpace> const& sensor_normal_vector,
				SpaceVector<si::Velocity, ECEFSpace> const sensor_velocity)
{
	// P_total is Pitot pressure (one that is measured byt the Pitot probe).
	// P_total = P_static + air_density * velocity² / 2
	// The velocity here is True Air Speed.
	auto const velocity_relative_to_air = sensor_velocity - air.velocity;
	// Assuming sensor_normal_vector is normalized:
	auto const velocity_on_sensor = projection_onto_normalized (velocity_relative_to_air, sensor_normal_vector);
	return air.pressure + dynamic_pressure (air.density, abs (velocity_on_sensor));
}


si::Pressure
total_pressure (Atmosphere const& atmosphere,
				Placement<ECEFSpace> const& placement,
				SpaceVector<si::Velocity, ECEFSpace> const sensor_velocity)
{
	auto const air = atmosphere.air_at (placement.position());
	auto const sensor_normal_vector = placement.body_coordinates().column (0);
	return total_pressure (air, sensor_normal_vector, sensor_velocity);
}

} // namespace xf


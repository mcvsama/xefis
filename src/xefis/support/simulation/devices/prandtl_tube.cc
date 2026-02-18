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
#include "prandtl_tube.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/atmosphere/air.h>
#include <xefis/support/nature/various_inertia_tensors.h>
#include <xefis/support/shapes/various_materials.h>
#include <xefis/support/shapes/various_shapes.h>

// Standard:
#include <cstddef>


namespace xf::sim {

PrandtlTube::PrandtlTube (Atmosphere const& atmosphere, PrandtlTubeParameters const& params):
	Body (MassMoments<BodyCOM>()),
	_atmosphere (atmosphere)
{
	auto const material = make_material ({ 0xff, 0xaa, 0x00 });
	auto shape = make_cylinder_shape ({
		.length { params.length },
		.radius { 0.5 * params.diameter },
		.num_faces { 7 },
		.with_bottom = true,
		.with_top = true,
		.material = material,
	});
	// Make X point into the wind:
	auto const orientation = xf::y_rotation<BodyOrigin> (90_deg);
	shape.rotate (orientation);
	set_shape (shape);

	auto const inertia_tensor_at_com = make_centered_solid_cylinder_inertia_tensor<BodyCOM> ({
		.mass = params.mass,
		.radius = 0.5 * params.diameter,
		.length = params.length,
	});
	// Build mass moments for a Z-oriented cylinder and rotate to match shape orientation.
	// The origin is at the tube back, so COM is at +length/2 before rotation.
	auto const z_oriented_offset = SpaceLength<BodyCOM> { 0_m, 0_m, 0.5 * params.length };
	auto const z_oriented_inertia_tensor_at_origin = inertia_tensor_com_to_point (params.mass, inertia_tensor_at_com, z_oriented_offset);
	auto const z_oriented_mass_moments = MassMomentsAtArm<BodyCOM> (params.mass, z_oriented_offset, z_oriented_inertia_tensor_at_origin);
	set_mass_moments (xf::y_rotation<BodyCOM> (90_deg) * z_oriented_mass_moments);
}


si::Pressure
PrandtlTube::static_pressure() const
{
	auto static_pressure = _static_pressure.lock();

	if (!*static_pressure)
	{
		auto const sensor_position = coordinate_system_cast<ECEFSpace, void, WorldSpace, void> (placement().position());
		*static_pressure = _atmosphere.pressure_at (sensor_position);
	}

	return static_pressure->value();
}


si::Pressure
PrandtlTube::total_pressure() const
{
	auto total_pressure = _total_pressure.lock();

	if (!*total_pressure)
	{
		using math::coordinate_system_cast;

		auto const sensor_position = coordinate_system_cast<ECEFSpace, ECEFSpace, WorldSpace, BodyCOM> (placement());
		auto const sensor_velocity = coordinate_system_cast<ECEFSpace, void, WorldSpace, void> (velocity_moments<WorldSpace>().velocity());
		*total_pressure = xf::total_pressure (_atmosphere, sensor_position, sensor_velocity);
	}

	return total_pressure->value();
}


void
PrandtlTube::evolve ([[maybe_unused]] si::Time dt)
{
	_static_pressure->reset();
	_total_pressure->reset();
}

} // namespace xf::sim

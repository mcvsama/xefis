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
#include <xefis/support/earth/air/air.h>
#include <xefis/support/nature/various_inertia_tensors.h>
#include <xefis/support/simulation/rigid_body/various_materials.h>
#include <xefis/support/simulation/rigid_body/various_shapes.h>

// Standard:
#include <cstddef>


namespace xf::sim {

PrandtlTube::PrandtlTube (Atmosphere const& atmosphere, PrandtlTubeParameters const& params):
	Body (MassMoments<BodyCOM>()),
	_atmosphere (&atmosphere)
{
	auto const material = rigid_body::make_material ({ 0xff, 0xaa, 0x00 });
	auto shape = rigid_body::make_cylinder_shape ({
		.length { params.length },
		.radius { 0.5 * params.diameter },
		.num_faces { 7 },
		.with_bottom = true,
		.with_top = true,
		.material = material,
	});
	set_shape (shape);

	auto const inertia_tensor_at_com = make_centered_solid_cylinder_inertia_tensor<BodyCOM> ({ .mass = params.mass, .radius = 0.5 * params.diameter, .length = params.length });
	auto const inertia_tensor_at_origin = inertia_tensor_com_to_point (params.mass, inertia_tensor_at_com, SpaceLength<BodyCOM> { 0_m, 0_m, 0.5 * params.length });
	set_mass_moments (MassMomentsAtArm<BodyCOM> (params.mass, { 0_m, 0_m, 0.5 * params.length }, inertia_tensor_at_origin));

	// Make X point into the wind:
	rotate_about_body_origin (xf::y_rotation<WorldSpace> (90_deg));
}


si::Pressure
PrandtlTube::static_pressure() const
{
	// TODO calculate relative wind from atmosphere and own velocities
	// TODO variations of pressure coming from the relative wind
	return _atmosphere->air_at (placement().coordinate_system_cast<ECEFSpace, void>().position()).pressure;
}


si::Pressure
PrandtlTube::total_pressure() const
{
	using math::coordinate_system_cast;

	// TODO calculate relative wind from atmosphere and own velocities
	// TODO variations of pressure coming from the relative wind
	return xf::total_pressure (*_atmosphere,
							   coordinate_system_cast<ECEFSpace, ECEFSpace> (placement()),
							   coordinate_system_cast<ECEFSpace, void> (velocity_moments<WorldSpace>().velocity()));
}

} // namespace xf::sim


/* vim:ts=4
 *
 * Copyleft 2019  Michał Gawron
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
#include "fixed_constraint.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/geometry.h>

// Standard:
#include <cstddef>


namespace xf::rigid_body {

FixedConstraint::FixedConstraint (Body& body_1, Body& body_2):
	Constraint (body_1, body_2),
	_fixed_orientation (body_1.location(), body_2.location())
{
	SpaceLength<WorldSpace> const origin (math::zero);
	_anchor_1 = body_1.location().bound_transform_to_body (origin);
	_anchor_2 = body_2.location().bound_transform_to_body (origin);
}


ConstraintForces
FixedConstraint::do_constraint_forces (VelocityMoments<WorldSpace> const& vm_1, ForceMoments<WorldSpace> const& ext_forces_1,
									   VelocityMoments<WorldSpace> const& vm_2, ForceMoments<WorldSpace> const& ext_forces_2,
									   si::Time dt)
{
	auto const loc_1 = body_1().location();
	auto const loc_2 = body_2().location();
	auto const x1 = loc_1.position();
	auto const x2 = loc_2.position();
	auto const r1 = loc_1.unbound_transform_to_base (_anchor_1);
	auto const r2 = loc_2.unbound_transform_to_base (_anchor_2);

	auto const Jv1 = JacobianV<6> {
		// Translation:
		-1.0,  0.0,  0.0,
		 0.0, -1.0,  0.0,
		 0.0,  0.0, -1.0,
		// Rotation:
		 0.0,  0.0,  0.0,
		 0.0,  0.0,  0.0,
		 0.0,  0.0,  0.0,
	};

	auto Jw1 = JacobianW<6> {
		// Translation:
		 0.0_m,  0.0_m,  0.0_m,
		 0.0_m,  0.0_m,  0.0_m,
		 0.0_m,  0.0_m,  0.0_m,
		// Rotation:
		-1.0_m,  0.0_m,  0.0_m,
		 0.0_m, -1.0_m,  0.0_m,
		 0.0_m,  0.0_m, -1.0_m,
	};
	// Translation:
	Jw1.put (make_pseudotensor (r1), 0, 0);

	auto const Jv2 = JacobianV<6> {
		// Translation:
		+1.0,  0.0,  0.0,
		 0.0, +1.0,  0.0,
		 0.0,  0.0, +1.0,
		// Rotation:
		 0.0,  0.0,  0.0,
		 0.0,  0.0,  0.0,
		 0.0,  0.0,  0.0,
	};

	auto Jw2 = JacobianW<6> {
		// Translation:
		 0.0_m,  0.0_m,  0.0_m,
		 0.0_m,  0.0_m,  0.0_m,
		 0.0_m,  0.0_m,  0.0_m,
		// Rotation:
		+1.0_m,  0.0_m,  0.0_m,
		 0.0_m, +1.0_m,  0.0_m,
		 0.0_m,  0.0_m, +1.0_m,
	};
	// Translation:
	Jw2.put (-make_pseudotensor (r2), 0, 0);

	LocationConstraint<6> location_constraint_value;
	location_constraint_value.put (x2 + r2 - x1 - r1, 0, 0);
	location_constraint_value.put (_fixed_orientation.rotation_constraint_value (loc_1, loc_2), 0, 3);

	auto const K = calculate_K (Jv1, Jw1, Jv2, Jw2);

	return calculate_constraint_forces (vm_1, ext_forces_1, Jv1, Jw1,
										vm_2, ext_forces_2, Jv2, Jw2,
										location_constraint_value, K, dt);
}

} // namespace xf::rigid_body


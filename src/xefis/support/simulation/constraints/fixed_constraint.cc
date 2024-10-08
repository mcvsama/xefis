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
	_fixed_orientation (body_1.placement(), body_2.placement())
{
	set_label ("fixed constraint");
	SpaceLength<WorldSpace> const origin (math::zero);
	_anchor_1 = body_1.placement().bound_transform_to_body (origin);
	_anchor_2 = body_2.placement().bound_transform_to_body (origin);
}


ConstraintForces
FixedConstraint::do_constraint_forces (VelocityMoments<WorldSpace> const& vm_1, ForceMoments<WorldSpace> const& ext_forces_1,
									   VelocityMoments<WorldSpace> const& vm_2, ForceMoments<WorldSpace> const& ext_forces_2,
									   si::Time dt) const
{
	auto const pl_1 = body_1().placement();
	auto const pl_2 = body_2().placement();
	auto const x1 = pl_1.position();
	auto const x2 = pl_2.position();
	auto const r1 = pl_1.unbound_transform_to_base (_anchor_1);
	auto const r2 = pl_2.unbound_transform_to_base (_anchor_2);

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
	location_constraint_value.put (_fixed_orientation.rotation_constraint_value (pl_1, pl_2), 0, 3);

	auto const J = calculate_jacobian (vm_1, ext_forces_1, Jv1, Jw1,
									   vm_2, ext_forces_2, Jv2, Jw2,
									   dt);
	auto const K = calculate_K (Jv1, Jw1, Jv2, Jw2);
	auto const lambda = calculate_lambda (location_constraint_value, J, K, dt);

	return calculate_constraint_forces (Jv1, Jw1, Jv2, Jw2, lambda);
}

} // namespace xf::rigid_body


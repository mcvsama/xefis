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

	_Jv1 = JacobianV<6> {
		// Translation:
		-1.0,  0.0,  0.0,
		 0.0, -1.0,  0.0,
		 0.0,  0.0, -1.0,
		// Rotation:
		 0.0,  0.0,  0.0,
		 0.0,  0.0,  0.0,
		 0.0,  0.0,  0.0,
	};

	_Jw1 = JacobianW<6> {
		// Translation:
		 0.0_m,  0.0_m,  0.0_m,
		 0.0_m,  0.0_m,  0.0_m,
		 0.0_m,  0.0_m,  0.0_m,
		// Rotation:
		-1.0_m,  0.0_m,  0.0_m,
		 0.0_m, -1.0_m,  0.0_m,
		 0.0_m,  0.0_m, -1.0_m,
	};

	_Jv2 = JacobianV<6> {
		// Translation:
		+1.0,  0.0,  0.0,
		 0.0, +1.0,  0.0,
		 0.0,  0.0, +1.0,
		// Rotation:
		 0.0,  0.0,  0.0,
		 0.0,  0.0,  0.0,
		 0.0,  0.0,  0.0,
	};

	_Jw2 = JacobianW<6> {
		// Translation:
		 0.0_m,  0.0_m,  0.0_m,
		 0.0_m,  0.0_m,  0.0_m,
		 0.0_m,  0.0_m,  0.0_m,
		// Rotation:
		+1.0_m,  0.0_m,  0.0_m,
		 0.0_m, +1.0_m,  0.0_m,
		 0.0_m,  0.0_m, +1.0_m,
	};
}


void
FixedConstraint::initialize_step (si::Time const dt)
{
	auto const placement_1 = body_1().placement();
	auto const placement_2 = body_2().placement();

	auto const x1 = placement_1.position();
	auto const x2 = placement_2.position();

	auto const r1 = placement_1.unbound_transform_to_base (_anchor_1);
	auto const r2 = placement_2.unbound_transform_to_base (_anchor_2);

	_Jw1.put (+make_pseudotensor (r1), 0, 0); // Translation
	_Jw2.put (-make_pseudotensor (r2), 0, 0); // Translation

	_location_constraint_value = LocationConstraint<6>(); // TODO maybe this reset is not needed?
	_location_constraint_value.put (x2 + r2 - x1 - r1, 0, 0);
	_location_constraint_value.put (_fixed_orientation.rotation_constraint_value (placement_1, placement_2), 0, 3);

	_Z = calculate_Z (_Jv1, _Jw1, _Jv2, _Jw2, dt);
}


ConstraintForces
FixedConstraint::do_constraint_forces (VelocityMoments<WorldSpace> const& vm_1, VelocityMoments<WorldSpace> const& vm_2, si::Time dt)
{
	auto const J = calculate_jacobian (vm_1, _Jv1, _Jw1, vm_2, _Jv2, _Jw2);
	auto const lambda = calculate_lambda (_location_constraint_value, J, _Z, dt);

	// TODO this internally transposes jacobians, so store here the transposed ones and reuse them
	return calculate_constraint_forces (_Jv1, _Jw1, _Jv2, _Jw2, lambda);
}

} // namespace xf::rigid_body


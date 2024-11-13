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
#include "hinge_constraint.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/euler_angles.h>
#include <xefis/support/math/geometry.h>

// Standard:
#include <cstddef>


namespace xf::rigid_body {

HingeConstraint::HingeConstraint (HingePrecalculation& hinge_precalculation):
	Constraint (hinge_precalculation),
	_hinge_precalculation (hinge_precalculation)
{
	set_label ("hinge");
}


ConstraintForces
HingeConstraint::do_constraint_forces (VelocityMoments<WorldSpace> const& vm_1, ForceMoments<WorldSpace> const& ext_forces_1,
									   VelocityMoments<WorldSpace> const& vm_2, ForceMoments<WorldSpace> const& ext_forces_2,
									   si::Time dt) const
{
	auto const& c = _hinge_precalculation.data();

	auto const Jv1 = JacobianV<5> {
		// Translation:
		-1.0,  0.0,  0.0,
		 0.0, -1.0,  0.0,
		 0.0,  0.0, -1.0,
		// Rotation:
		 0.0,  0.0,  0.0,
		 0.0,  0.0,  0.0,
	};

	auto Jw1 = JacobianW<5>();
	// Translation:
	Jw1.put (make_pseudotensor (c.r1), 0, 0);
	// Rotation:
	Jw1.put (-~c.t1, 0, 3);
	Jw1.put (-~c.t2, 0, 4);

	auto const Jv2 = JacobianV<5> {
		// Translation:
		+1.0,  0.0,  0.0,
		 0.0, +1.0,  0.0,
		 0.0,  0.0, +1.0,
		// Rotation:
		 0.0,  0.0,  0.0,
		 0.0,  0.0,  0.0,
	};

	auto Jw2 = JacobianW<5>();
	// Translation:
	Jw2.put (-make_pseudotensor (c.r2), 0, 0);
	// Rotation:
	Jw2.put (~c.t1, 0, 3);
	Jw2.put (~c.t2, 0, 4);

	LocationConstraint<5> location_constraint_value;
	location_constraint_value.put (c.u, 0, 0);
	auto const a1xa2 = cross_product (c.a1, c.a2);
	location_constraint_value[0, 3] = dot_product (c.t1, a1xa2);
	location_constraint_value[0, 4] = dot_product (c.t2, a1xa2);

	auto const J = calculate_jacobian (vm_1, ext_forces_1, Jv1, Jw1,
									   vm_2, ext_forces_2, Jv2, Jw2,
									   dt);
	auto const K = calculate_K (Jv1, Jw1, Jv2, Jw2);
	auto const lambda = calculate_lambda (location_constraint_value, J, K, dt);

	return calculate_constraint_forces (Jv1, Jw1, Jv2, Jw2, lambda);
}

} // namespace xf::rigid_body


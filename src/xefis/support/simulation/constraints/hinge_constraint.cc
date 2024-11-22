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
#include <xefis/support/math/geometry.h>

// Standard:
#include <cstddef>


namespace xf::rigid_body {

HingeConstraint::HingeConstraint (HingePrecalculation& hinge_precalculation):
	Constraint (hinge_precalculation),
	_hinge_precalculation (hinge_precalculation)
{
	set_label ("hinge");

	_Jv1 = JacobianV<5> {
		// Translation:
		-1.0,  0.0,  0.0,
		 0.0, -1.0,  0.0,
		 0.0,  0.0, -1.0,
		// Rotation:
		 0.0,  0.0,  0.0,
		 0.0,  0.0,  0.0,
	};

	_Jv2 = JacobianV<5> {
		// Translation:
		+1.0,  0.0,  0.0,
		 0.0, +1.0,  0.0,
		 0.0,  0.0, +1.0,
		// Rotation:
		 0.0,  0.0,  0.0,
		 0.0,  0.0,  0.0,
	};
}


void
HingeConstraint::initialize_step (si::Time const dt)
{
	auto const& hinge = _hinge_precalculation.data();

	// Translation:
	_Jw1.put (make_pseudotensor (hinge.r1), 0, 0);
	// Rotation:
	_Jw1.put (-~hinge.t1, 0, 3);
	_Jw1.put (-~hinge.t2, 0, 4);

	_Jw2 = JacobianW<5>();
	// Translation:
	_Jw2.put (-make_pseudotensor (hinge.r2), 0, 0);
	// Rotation:
	_Jw2.put (~hinge.t1, 0, 3);
	_Jw2.put (~hinge.t2, 0, 4);

	_Z = calculate_Z (_Jv1, _Jw1, _Jv2, _Jw2, dt);

	_location_constraint_value.put (hinge.u, 0, 0);
	auto const a1xa2 = cross_product (hinge.a1, hinge.a2);
	_location_constraint_value[0, 3] = dot_product (hinge.t1, a1xa2);
	_location_constraint_value[0, 4] = dot_product (hinge.t2, a1xa2);
}


ConstraintForces
HingeConstraint::do_constraint_forces (VelocityMoments<WorldSpace> const& vm_1, VelocityMoments<WorldSpace> const& vm_2, si::Time dt)
{
	auto const J = calculate_jacobian (vm_1, _Jv1, _Jw1, vm_2, _Jv2, _Jw2);
	auto const lambda = calculate_lambda (_location_constraint_value, J, _Z, dt);

	return calculate_constraint_forces (_Jv1, _Jw1, _Jv2, _Jw2, lambda);
}

} // namespace xf::rigid_body


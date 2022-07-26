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
#include "slider_constraint.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/euler_angles.h>
#include <xefis/support/math/geometry.h>

// Standard:
#include <cstddef>


namespace xf::rigid_body {

SliderConstraint::SliderConstraint (SliderPrecalculation& slider_precalculation):
	Constraint (slider_precalculation),
	_slider_precalculation (slider_precalculation)
{ }


ConstraintForces
SliderConstraint::do_constraint_forces (VelocityMoments<WorldSpace> const& vm_1, ForceMoments<WorldSpace> const& ext_forces_1,
										VelocityMoments<WorldSpace> const& vm_2, ForceMoments<WorldSpace> const& ext_forces_2,
										si::Time dt)
{
	auto const& c = _slider_precalculation.data();

	auto Jv1 = JacobianV<5> {
		// Translation:
		 0.0,  0.0,  0.0,
		 0.0,  0.0,  0.0,
		// Rotation:
		 0.0,  0.0,  0.0,
		 0.0,  0.0,  0.0,
		 0.0,  0.0,  0.0,
	};
	Jv1.put (-~c.t1, 0, 0);
	Jv1.put (-~c.t2, 0, 1);

	auto Jw1 = JacobianW<5> {
		// Translation:
		 0.0_m,  0.0_m,  0.0_m,
		 0.0_m,  0.0_m,  0.0_m,
		// Rotation:
		-1.0_m,  0.0_m,  0.0_m,
		 0.0_m, -1.0_m,  0.0_m,
		 0.0_m,  0.0_m, -1.0_m,
	};
	// Translation:
	Jw1.put (-~cross_product (c.r1 + c.u, c.t1), 0, 0);
	Jw1.put (-~cross_product (c.r1 + c.u, c.t2), 0, 1);

	auto Jv2 = JacobianV<5> {
		// Translation:
		 0.0,  0.0,  0.0,
		 0.0,  0.0,  0.0,
		// Rotation:
		 0.0,  0.0,  0.0,
		 0.0,  0.0,  0.0,
		 0.0,  0.0,  0.0,
	};
	Jv2.put (~c.t1, 0, 0);
	Jv2.put (~c.t2, 0, 1);

	auto Jw2 = JacobianW<5> {
		// Translation:
		 0.0_m,  0.0_m,  0.0_m,
		 0.0_m,  0.0_m,  0.0_m,
		// Rotation:
		+1.0_m,  0.0_m,  0.0_m,
		 0.0_m, +1.0_m,  0.0_m,
		 0.0_m,  0.0_m, +1.0_m,
	};
	// Translation:
	Jw2.put (~cross_product (c.r2, c.t1), 0, 0);
	Jw2.put (~cross_product (c.r2, c.t2), 0, 1);

	LocationConstraint<5> location_constraint_value;
	location_constraint_value.put (~c.u * c.t1, 0, 0);
	location_constraint_value.put (~c.u * c.t2, 0, 1);
	location_constraint_value.put (c.rotation_error, 0, 2);

	auto const K = calculate_K (Jv1, Jw1, Jv2, Jw2);

	return calculate_constraint_forces (vm_1, ext_forces_1, Jv1, Jw1,
										vm_2, ext_forces_2, Jv2, Jw2,
										location_constraint_value, K, dt);
}

} // namespace xf::rigid_body


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
#include <xefis/support/math/geometry.h>

// Standard:
#include <cstddef>


namespace xf::rigid_body {

SliderConstraint::SliderConstraint (SliderPrecalculation& slider_precalculation):
	Constraint (slider_precalculation),
	_slider_precalculation (slider_precalculation)
{
	set_label ("slider");

	_Jv1 = JacobianV<5> {
		// Translation:
		 0.0,  0.0,  0.0,
		 0.0,  0.0,  0.0,
		// Rotation:
		 0.0,  0.0,  0.0,
		 0.0,  0.0,  0.0,
		 0.0,  0.0,  0.0,
	};

	_Jw1 = JacobianW<5> {
		// Translation:
		 0.0_m,  0.0_m,  0.0_m,
		 0.0_m,  0.0_m,  0.0_m,
		// Rotation:
		-1.0_m,  0.0_m,  0.0_m,
		 0.0_m, -1.0_m,  0.0_m,
		 0.0_m,  0.0_m, -1.0_m,
	};

	_Jv2 = JacobianV<5> {
		// Translation:
		 0.0,  0.0,  0.0,
		 0.0,  0.0,  0.0,
		// Rotation:
		 0.0,  0.0,  0.0,
		 0.0,  0.0,  0.0,
		 0.0,  0.0,  0.0,
	};

	_Jw2 = JacobianW<5> {
		// Translation:
		 0.0_m,  0.0_m,  0.0_m,
		 0.0_m,  0.0_m,  0.0_m,
		// Rotation:
		+1.0_m,  0.0_m,  0.0_m,
		 0.0_m, +1.0_m,  0.0_m,
		 0.0_m,  0.0_m, +1.0_m,
	};
}


void
SliderConstraint::initialize_step (si::Time const dt)
{
	auto const& slider_data = _slider_precalculation.data();

	_Jv1.put (-~slider_data.t1, 0, 0);
	_Jv1.put (-~slider_data.t2, 0, 1);
	// Translation:
	_Jw1.put (-~cross_product (slider_data.r1 + slider_data.u, slider_data.t1), 0, 0);
	_Jw1.put (-~cross_product (slider_data.r1 + slider_data.u, slider_data.t2), 0, 1);

	_Jv2.put (~slider_data.t1, 0, 0);
	_Jv2.put (~slider_data.t2, 0, 1);
	// Translation:
	_Jw2.put (~cross_product (slider_data.r2, slider_data.t1), 0, 0);
	_Jw2.put (~cross_product (slider_data.r2, slider_data.t2), 0, 1);

	_location_constraint_value.put (~slider_data.u * slider_data.t1, 0, 0);
	_location_constraint_value.put (~slider_data.u * slider_data.t2, 0, 1);
	_location_constraint_value.put (slider_data.rotation_error, 0, 2);

	_Z = calculate_Z (_Jv1, _Jw1, _Jv2, _Jw2, dt);
}


ConstraintForces
SliderConstraint::do_constraint_forces (VelocityMoments<WorldSpace> const& vm_1, VelocityMoments<WorldSpace> const& vm_2, si::Time dt)
{
	auto const J = calculate_jacobian (vm_1, _Jv1, _Jw1, vm_2, _Jv2, _Jw2);
	auto const lambda = calculate_lambda (_location_constraint_value, J, _Z, dt);

	return calculate_constraint_forces (_Jv1, _Jw1, _Jv2, _Jw2, lambda);
}

} // namespace xf::rigid_body


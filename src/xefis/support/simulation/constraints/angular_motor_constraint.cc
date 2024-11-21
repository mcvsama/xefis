/* vim:ts=4
 *
 * Copyleft 2023  Michał Gawron
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
#include "angular_motor_constraint.h"

// Standard:
#include <cstddef>


namespace xf::rigid_body {

AngularMotorConstraint::AngularMotorConstraint (HingePrecalculation& hinge_precalculation, si::AngularVelocity const max_angular_velocity, si::Torque const torque):
	Constraint (hinge_precalculation),
	_hinge_precalculation (hinge_precalculation),
	_max_angular_velocity (max_angular_velocity),
	_force (abs (torque / 1_m))
{
	set_label ("angular motor");
}


ConstraintForces
AngularMotorConstraint::do_constraint_forces (VelocityMoments<WorldSpace> const& vm_1, VelocityMoments<WorldSpace> const& vm_2, si::Time dt) const
{
	auto const& c = _hinge_precalculation.data();

	JacobianV<1> Jv (math::zero);
	JacobianW<1> Jw1;
	JacobianW<1> Jw2;
	LocationConstraint<1> location_constraint_value;

	Jw1.put (1_m * ~c.a1, 0, 0);
	Jw2.put (1_m * -~c.a1, 0, 0);
	location_constraint_value = _max_angular_velocity * 1_m / 1_rad * 1_s;

	auto const J = calculate_jacobian (vm_1, Jv, Jw1, vm_2, Jv, Jw2);
	auto const K = calculate_K (Jw1, Jw2);
	auto lambda = calculate_lambda (location_constraint_value, J, K, dt);
	lambda = std::clamp (lambda.scalar(), -_force, +_force); // TODO scalar?

	return calculate_constraint_forces (Jv, Jw1, Jv, Jw2, lambda);
}

} // namespace xf::rigid_body


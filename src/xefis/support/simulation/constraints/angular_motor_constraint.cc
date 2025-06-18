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

AngularMotorConstraint::AngularMotorConstraint (HingePrecomputation& hinge_precomputation, si::AngularVelocity const max_angular_velocity, si::Torque const torque):
	Constraint (hinge_precomputation),
	_hinge_precomputation (hinge_precomputation),
	_max_angular_velocity (max_angular_velocity),
	_force (abs (torque / 1_m))
{
	set_label ("angular motor");
}


void
AngularMotorConstraint::initialize_step (si::Time const dt)
{
	auto const& hinge = _hinge_precomputation.data();

	_Jw1.put (1_m * ~hinge.a1, 0, 0);
	_Jw2.put (1_m * -~hinge.a1, 0, 0);
	_Z = compute_Z (_Jw1, _Jw2, dt);
	_location_constraint_value = _max_angular_velocity * 1_m / 1_rad * 1_s;
}


ConstraintForces
AngularMotorConstraint::do_constraint_forces (VelocityMoments<WorldSpace> const& vm_1, VelocityMoments<WorldSpace> const& vm_2, si::Time dt)
{
	auto const J = compute_jacobian (vm_1, _Jv, _Jw1, vm_2, _Jv, _Jw2);
	auto lambda = compute_lambda (_location_constraint_value, J, _Z, dt);
	lambda = std::clamp (lambda.scalar(), -_force, +_force); // TODO scalar?

	return compute_constraint_forces (_Jv, _Jw1, _Jv, _Jw2, lambda);
}

} // namespace xf::rigid_body


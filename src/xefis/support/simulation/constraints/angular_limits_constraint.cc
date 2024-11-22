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
#include "angular_limits_constraint.h"

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>


namespace xf::rigid_body {

AngularLimitsConstraint::AngularLimitsConstraint (HingePrecalculation& hinge_precalculation, std::optional<si::Angle> const min_angle, std::optional<si::Angle> const max_angle):
	Constraint (hinge_precalculation),
	_hinge_precalculation (hinge_precalculation),
	_min_angle (min_angle),
	_max_angle (max_angle)
{
	set_label ("angular limits");
}


AngularLimitsConstraint::AngularLimitsConstraint (HingePrecalculation& hinge_precalculation, Range<si::Angle> const range):
	AngularLimitsConstraint (hinge_precalculation, range.min(), range.max())
{ }


void
AngularLimitsConstraint::initialize_step (si::Time const dt)
{
	auto const& hinge = _hinge_precalculation.data();

	_min_Jw1.put (1_m * -~hinge.a1, 0, 0);
	_min_Jw2.put (1_m * ~hinge.a1, 0, 0);
	// _max_* are reversed _min_*.
	_min_Z = calculate_Z (_min_Jw1, _min_Jw2, dt);
	_min_location_constraint_value = (hinge.angle - *_min_angle) * 1_m / 1_rad;
	_max_Z = calculate_Z (_min_Jw2, _min_Jw1, dt);
	_max_location_constraint_value = (*_max_angle - hinge.angle) * 1_m / 1_rad;
}


ConstraintForces
AngularLimitsConstraint::do_constraint_forces (VelocityMoments<WorldSpace> const& vm_1, VelocityMoments<WorldSpace> const& vm_2, si::Time dt)
{
	ConstraintForces fc;
	auto const& hinge_data = _hinge_precalculation.data();

	if (auto fc_min = min_angle_corrections (vm_1, vm_2, dt, hinge_data))
		fc = fc + *fc_min;

	if (auto fc_max = max_angle_corrections (vm_1, vm_2, dt, hinge_data))
		fc = fc + *fc_max;

	return fc;
}


std::optional<ConstraintForces>
AngularLimitsConstraint::min_angle_corrections (VelocityMoments<WorldSpace> const& vm_1,
												VelocityMoments<WorldSpace> const& vm_2,
												si::Time dt,
												HingePrecalculationData const& hinge_data) const
{
	if (_min_angle && hinge_data.angle < *_min_angle)
	{
		auto const J = calculate_jacobian (vm_1, _Jv, _min_Jw1, vm_2, _Jv, _min_Jw2);
		auto const lambda = calculate_lambda (_min_location_constraint_value, J, _min_Z, dt);

		return calculate_constraint_forces (_Jv, _min_Jw1, _Jv, _min_Jw2, lambda);
	}
	else
		return std::nullopt;
}


std::optional<ConstraintForces>
AngularLimitsConstraint::max_angle_corrections (VelocityMoments<WorldSpace> const& vm_1,
												VelocityMoments<WorldSpace> const& vm_2,
												si::Time dt,
												HingePrecalculationData const& hinge_data) const
{
	if (_max_angle && hinge_data.angle > *_max_angle)
	{
		auto const J = calculate_jacobian (vm_1, _Jv, _min_Jw2, vm_2, _Jv, _min_Jw1);
		auto const lambda = calculate_lambda (_max_location_constraint_value, J, _max_Z, dt);

		return calculate_constraint_forces (_Jv, _min_Jw2, _Jv, _min_Jw1, lambda);
	}
	else
		return std::nullopt;
}

} // namespace xf::rigid_body


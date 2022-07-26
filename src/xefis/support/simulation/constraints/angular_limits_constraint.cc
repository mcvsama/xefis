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
#include <array>


namespace xf::rigid_body {

AngularLimitsConstraint::AngularLimitsConstraint (HingePrecalculation& hinge_precalculation, std::optional<si::Angle> const min_angle, std::optional<si::Angle> const max_angle):
	Constraint (hinge_precalculation),
	_hinge_precalculation (hinge_precalculation),
	_min_angle (min_angle),
	_max_angle (max_angle)
{ }


AngularLimitsConstraint::AngularLimitsConstraint (HingePrecalculation& hinge_precalculation, Range<si::Angle> const range):
	AngularLimitsConstraint (hinge_precalculation, range.min(), range.max())
{ }


ConstraintForces
AngularLimitsConstraint::do_constraint_forces (VelocityMoments<WorldSpace> const& vm_1, ForceMoments<WorldSpace> const& ext_forces_1,
											   VelocityMoments<WorldSpace> const& vm_2, ForceMoments<WorldSpace> const& ext_forces_2,
											   si::Time dt)
{
	ConstraintForces fc;
	auto const& hinge_data = _hinge_precalculation.data();

	if (auto fc_min = min_angle_corrections (vm_1, ext_forces_1, vm_2, ext_forces_2, dt, hinge_data))
		fc = fc + *fc_min;

	if (auto fc_max = max_angle_corrections (vm_1, ext_forces_1, vm_2, ext_forces_2, dt, hinge_data))
		fc = fc + *fc_max;

	return fc;
}


std::optional<ConstraintForces>
AngularLimitsConstraint::min_angle_corrections (VelocityMoments<WorldSpace> const& vm_1, ForceMoments<WorldSpace> const& ext_forces_1,
												VelocityMoments<WorldSpace> const& vm_2, ForceMoments<WorldSpace> const& ext_forces_2,
												si::Time dt,
												HingePrecalculationData const& c) const
{
	if (_min_angle && c.angle < *_min_angle)
	{
		JacobianV<1> Jv (math::zero);
		JacobianW<1> Jw1;
		JacobianW<1> Jw2;
		LocationConstraint<1> location_constraint_value;

		Jw1.put (1_m * -~c.a1, 0, 0);
		Jw2.put (1_m * ~c.a1, 0, 0);
		location_constraint_value (0, 0) = (c.angle - *_min_angle) * 1_m / 1_rad;
		auto const K = calculate_K (Jw1, Jw2);

		return calculate_constraint_forces (vm_1, ext_forces_1, Jv, Jw1,
											vm_2, ext_forces_2, Jv, Jw2,
											location_constraint_value, K, dt);
	}
	else
		return std::nullopt;
}


std::optional<ConstraintForces>
AngularLimitsConstraint::max_angle_corrections (VelocityMoments<WorldSpace> const& vm_1, ForceMoments<WorldSpace> const& ext_forces_1,
												VelocityMoments<WorldSpace> const& vm_2, ForceMoments<WorldSpace> const& ext_forces_2,
												si::Time dt,
												HingePrecalculationData const& c) const
{
	if (_max_angle && c.angle > *_max_angle)
	{
		JacobianV<1> Jv (math::zero);
		JacobianW<1> Jw1;
		JacobianW<1> Jw2;
		LocationConstraint<1> location_constraint_value;

		Jw1.put (1_m * ~c.a1, 0, 0);
		Jw2.put (1_m * -~c.a1, 0, 0);
		location_constraint_value (0, 0) = (*_max_angle - c.angle) * 1_m / 1_rad;
		auto const K = calculate_K (Jw1, Jw2);

		return calculate_constraint_forces (vm_1, ext_forces_1, Jv, Jw1,
											vm_2, ext_forces_2, Jv, Jw2,
											location_constraint_value, K, dt);
	}
	else
		return std::nullopt;
}

} // namespace xf::rigid_body


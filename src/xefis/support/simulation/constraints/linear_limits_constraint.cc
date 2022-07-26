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
#include "linear_limits_constraint.h"

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>
#include <array>


namespace xf::rigid_body {

LinearLimitsConstraint::LinearLimitsConstraint (SliderPrecalculation& slider_precalculation, std::optional<si::Length> const min_distance, std::optional<si::Length> const max_distance):
	Constraint (slider_precalculation),
	_slider_precalculation (slider_precalculation),
	_min_distance (min_distance),
	_max_distance (max_distance)
{ }


LinearLimitsConstraint::LinearLimitsConstraint (SliderPrecalculation& slider_precalculation, Range<si::Length> const range):
	LinearLimitsConstraint (slider_precalculation, range.min(), range.max())
{ }


ConstraintForces
LinearLimitsConstraint::do_constraint_forces (VelocityMoments<WorldSpace> const& vm_1, ForceMoments<WorldSpace> const& ext_forces_1,
											  VelocityMoments<WorldSpace> const& vm_2, ForceMoments<WorldSpace> const& ext_forces_2,
											  si::Time dt)
{
	ConstraintForces fc;
	auto const& slider_data = _slider_precalculation.data();
	// TODO doesn't seem to work correctly

	if (auto fc_min = min_distance_corrections (vm_1, ext_forces_1, vm_2, ext_forces_2, dt, slider_data))
		fc = fc + *fc_min;

	if (auto fc_max = max_distance_corrections (vm_2, ext_forces_1, vm_2, ext_forces_2, dt, slider_data))
		fc = fc + *fc_max;

	return fc;
}


std::optional<ConstraintForces>
LinearLimitsConstraint::min_distance_corrections (VelocityMoments<WorldSpace> const& vm_1, ForceMoments<WorldSpace> const& ext_forces_1,
												  VelocityMoments<WorldSpace> const& vm_2, ForceMoments<WorldSpace> const& ext_forces_2,
												  si::Time dt,
												  SliderPrecalculationData const& c) const
{
	if (_min_distance && c.distance < *_min_distance)
	{
		JacobianV<1> Jv1;
		JacobianW<1> Jw1;
		JacobianV<1> Jv2;
		JacobianW<1> Jw2;
		LocationConstraint<1> location_constraint_value;

		Jv1.put (-~c.a, 0, 0);
		Jw1.put (-c.r1uxa, 0, 0);
		Jv2.put (~c.a, 0, 0);
		Jw2.put (c.r2xa, 0, 0);
		location_constraint_value (0, 0) = c.distance - *_min_distance;
		auto const K = calculate_K (Jv1, Jw1, Jv2, Jw2);

		return calculate_constraint_forces (vm_1, ext_forces_1, Jv1, Jw1,
											vm_2, ext_forces_2, Jv2, Jw2,
											location_constraint_value, K, dt);
	}
	else
		return std::nullopt;
}


std::optional<ConstraintForces>
LinearLimitsConstraint::max_distance_corrections (VelocityMoments<WorldSpace> const& vm_1, ForceMoments<WorldSpace> const& ext_forces_1,
												  VelocityMoments<WorldSpace> const& vm_2, ForceMoments<WorldSpace> const& ext_forces_2,
												  si::Time dt,
												  SliderPrecalculationData const& c) const
{
	if (_max_distance && c.distance > *_max_distance)
	{
		JacobianV<1> Jv1;
		JacobianW<1> Jw1;
		JacobianV<1> Jv2;
		JacobianW<1> Jw2;
		LocationConstraint<1> location_constraint_value;

		Jv1.put (~c.a, 0, 0);
		Jw1.put (c.r1uxa, 0, 0);
		Jv2.put (-~c.a, 0, 0);
		Jw2.put (-c.r2xa, 0, 0);
		location_constraint_value (0, 0) = *_max_distance - c.distance;
		auto const K = calculate_K (Jv1, Jw1, Jv2, Jw2);

		return calculate_constraint_forces (vm_1, ext_forces_1, Jv1, Jw1,
											vm_2, ext_forces_2, Jv2, Jw2,
											location_constraint_value, K, dt);
	}
	else
		return std::nullopt;
}

} // namespace xf::rigid_body


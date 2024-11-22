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


namespace xf::rigid_body {

LinearLimitsConstraint::LinearLimitsConstraint (SliderPrecalculation& slider_precalculation, std::optional<si::Length> const min_distance, std::optional<si::Length> const max_distance):
	Constraint (slider_precalculation),
	_slider_precalculation (slider_precalculation),
	_min_distance (min_distance),
	_max_distance (max_distance)
{
	set_label ("linear limits");
}


LinearLimitsConstraint::LinearLimitsConstraint (SliderPrecalculation& slider_precalculation, Range<si::Length> const range):
	LinearLimitsConstraint (slider_precalculation, range.min(), range.max())
{ }


void
LinearLimitsConstraint::initialize_step (si::Time const dt)
{
	auto const& slider_data = _slider_precalculation.data();

	_min_Jv1.put (-~slider_data.a, 0, 0);
	_min_Jw1.put (-slider_data.r1uxa, 0, 0);
	_min_Jv2.put (~slider_data.a, 0, 0);
	_min_Jw2.put (slider_data.r2xa, 0, 0);
	_min_location_constraint_value = slider_data.distance - *_min_distance;
	_min_Z = calculate_Z (_min_Jv1, _min_Jw1, _min_Jv2, _min_Jw2, dt);

	// TODO doesn't seem to work correctly; perhaps look at symmetry of _min_Jw1 == _max_Jw2 in angular_limits
	// TODO and try to replicate it here:
	_max_Jv1.put (~slider_data.a, 0, 0);
	_max_Jw1.put (slider_data.r1uxa, 0, 0);
	_max_Jv2.put (-~slider_data.a, 0, 0);
	_max_Jw2.put (-slider_data.r2xa, 0, 0);
	_max_location_constraint_value = *_max_distance - slider_data.distance;
	_max_Z = calculate_Z (_max_Jv1, _max_Jw1, _max_Jv2, _max_Jw2, dt);
}


ConstraintForces
LinearLimitsConstraint::do_constraint_forces (VelocityMoments<WorldSpace> const& vm_1, VelocityMoments<WorldSpace> const& vm_2, si::Time dt)
{
	ConstraintForces fc;
	auto const& slider_data = _slider_precalculation.data();

	if (auto fc_min = min_distance_corrections (vm_1, vm_2, dt, slider_data))
		fc = fc + *fc_min;

	if (auto fc_max = max_distance_corrections (vm_2, vm_2, dt, slider_data))
		fc = fc + *fc_max;

	return fc;
}


std::optional<ConstraintForces>
LinearLimitsConstraint::min_distance_corrections (VelocityMoments<WorldSpace> const& vm_1,
												  VelocityMoments<WorldSpace> const& vm_2,
												  si::Time dt,
												  SliderPrecalculationData const& slider_data) const
{
	if (_min_distance && slider_data.distance < *_min_distance)
	{
		auto const J = calculate_jacobian (vm_1, _min_Jv1, _min_Jw1, vm_2, _min_Jv2, _min_Jw2);
		auto const lambda = calculate_lambda (_min_location_constraint_value, J, _min_Z, dt);

		return calculate_constraint_forces (_min_Jv1, _min_Jw1, _min_Jv2, _min_Jw2, lambda);
	}
	else
		return std::nullopt;
}


std::optional<ConstraintForces>
LinearLimitsConstraint::max_distance_corrections (VelocityMoments<WorldSpace> const& vm_1,
												  VelocityMoments<WorldSpace> const& vm_2,
												  si::Time dt,
												  SliderPrecalculationData const& slider_data) const
{
	if (_max_distance && slider_data.distance > *_max_distance)
	{
		auto const J = calculate_jacobian (vm_1, _max_Jv1, _max_Jw1, vm_2, _max_Jv2, _max_Jw2);
		auto const lambda = calculate_lambda (_max_location_constraint_value, J, _max_Z, dt);

		return calculate_constraint_forces (_max_Jv1, _max_Jw1, _max_Jv2, _max_Jw2, lambda);
	}
	else
		return std::nullopt;
}

} // namespace xf::rigid_body


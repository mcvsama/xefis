/* vim:ts=4
 *
 * Copyleft 2026  Michał Gawron
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
#include "cylinder_parasitic_drag.h"

// Xefis:
#include <xefis/support/aerodynamics/reynolds_number.h>

// Neutrino:
#include <neutrino/numeric.h>

// Standard:
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <numbers>


namespace xf {

namespace detail {

[[nodiscard]]
static inline double
axial_skin_friction_coefficient (ReynoldsNumber const re)
{
	auto const re_value = std::max (1.0, *re);
	// External flow over a slender cylinder approximated as a flat plate:
	// Cf_laminar = 1.328 / sqrt(Re), Cf_turbulent = 0.074 / Re^(1/5).
	auto const cf_laminar = 1.328 / std::sqrt (re_value);
	auto const cf_turbulent = 0.074 / std::pow (re_value, 0.2);
	return std::max (cf_laminar, cf_turbulent);
}


[[nodiscard]]
static inline double
transverse_pressure_drag_coefficient ([[maybe_unused]] ReynoldsNumber const re)
{
	// Circular cylinder in cross-flow, subcritical regime approximation.
	return 1.20;
}

} // namespace detail


ForceMoments<void>
cylinder_parasitic_drag_force_moments (CylinderParasiticDragParameters<void> const& params)
{
	auto axis = params.axis_direction;

	if (abs (axis) <= 1e-12)
		axis = { 1.0, 0.0, 0.0 };
	else
		axis.normalize();

	auto const relative_velocity = params.relative_air.velocity;
	auto const axial_velocity = projection_onto_normalized (relative_velocity, axis);
	auto const transverse_velocity = relative_velocity - axial_velocity;

	auto const axial_speed = abs (axial_velocity);
	auto const transverse_speed = abs (transverse_velocity);

	if (axial_speed <= 1e-9_mps && transverse_speed <= 1e-9_mps)
		return ForceMoments<void>::zero();

	auto const diameter = 2.0 * params.radius;
	// Reference areas:
	// A_front = π r² (flow parallel to axis),
	// A_side = d L (flow perpendicular to axis),
	// A_wet = 2 π r L (skin-friction wetted area).
	auto const frontal_area = std::numbers::pi * nu::square (params.radius);
	auto const side_area = diameter * params.length;
	auto const wetted_area = 2.0 * std::numbers::pi * params.radius * params.length;

	// Dynamic pressure for each velocity component:
	// q = 1/2 ρ V².
	auto const axial_dynamic_pressure = dynamic_pressure (params.relative_air.density, axial_speed);
	auto const transverse_dynamic_pressure = dynamic_pressure (params.relative_air.density, transverse_speed);

	auto const axial_re = reynolds_number (params.relative_air.density, axial_speed, params.length, params.relative_air.dynamic_viscosity);
	auto const transverse_re = reynolds_number (params.relative_air.density, transverse_speed, diameter, params.relative_air.dynamic_viscosity);

	auto const axial_cf = detail::axial_skin_friction_coefficient (axial_re);
	auto const transverse_cd = detail::transverse_pressure_drag_coefficient (transverse_re);

	// Transverse component:
	// F_perp = q_perp * Cd_perp * A_side.
	auto const transverse_drag_force = transverse_dynamic_pressure * transverse_cd * side_area;
	// Axial component:
	// F_axis = q_axis * (Cd_nose * A_front + Cf * A_wet).
	auto constexpr kAxialPressureDragCoefficient = 0.82;
	auto const axial_drag_force = axial_dynamic_pressure * (kAxialPressureDragCoefficient * frontal_area + axial_cf * wetted_area);

	auto const transverse_drag_direction = normalized_direction_or_zero (transverse_velocity);
	auto const axial_drag_direction = normalized_direction_or_zero (axial_velocity);

	auto const total_drag_force = transverse_drag_force * transverse_drag_direction + axial_drag_force * axial_drag_direction;
	auto const arm = params.center_of_pressure - params.reference_point;
	auto const total_drag_torque = cross_product (arm, total_drag_force);

	return ForceMoments<void> (total_drag_force, total_drag_torque);
}

} // namespace xf

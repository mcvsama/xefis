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
#include "whip_antenna.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/nature/various_inertia_tensors.h>
#include <xefis/support/shapes/various_materials.h>
#include <xefis/support/shapes/various_shapes.h>

// Standard:
#include <algorithm>
#include <cstddef>
#include <numbers>
#include <utility>


namespace xf::sim {
namespace {

WhipAntennaParameters const&
validate_parameters (WhipAntennaParameters const& params)
{
	if (params.mass <= 0_kg)
		throw nu::InvalidArgument ("whip antenna mass must be positive");

	if (params.antenna_length <= 0_m)
		throw nu::InvalidArgument ("whip antenna length must be positive");

	if (params.body_length <= 0_m)
		throw nu::InvalidArgument ("whip antenna body_length must be positive");

	if (params.base_length <= 0_m || params.base_length >= params.body_length)
		throw nu::InvalidArgument ("whip antenna base_length must be in range (0, body_length)");

	if (params.shaft_diameter <= 0_m)
		throw nu::InvalidArgument ("whip antenna shaft_diameter must be positive");

	if (params.base_diameter <= 0_m)
		throw nu::InvalidArgument ("whip antenna base_diameter must be positive");

	if (params.base_diameter < params.shaft_diameter)
		throw nu::InvalidArgument ("whip antenna base_diameter must be >= shaft_diameter");

	if (params.num_faces < 3)
		throw nu::InvalidArgument ("whip antenna num_faces must be at least 3");

	return params;
}

} // namespace


WhipAntenna::WhipAntenna (xf::AntennaSystem& antenna_system,
						  WhipAntennaParameters const& params,
						  xf::Antenna::SignalReceptionCallback signal_reception_callback):
	WhipAntennaBase (validate_parameters (params)),
	Antenna (compute_body_com_mass_moments (params), WhipAntennaBase::antenna_model(), antenna_system, std::move (signal_reception_callback))
{
	set_shape (make_shape (params));
}


MassMomentsAtArm<BodyCOM>
WhipAntenna::compute_body_com_mass_moments (WhipAntennaParameters const& params)
{
	auto const base_radius = 0.5 * params.base_diameter;
	auto const shaft_radius = 0.5 * params.shaft_diameter;
	auto const upper_length = params.body_length - params.base_length;
	auto const base_volume = std::numbers::pi * nu::square (base_radius) * params.base_length;
	auto const upper_volume = std::numbers::pi * nu::square (shaft_radius) * upper_length;
	auto const total_volume = base_volume + upper_volume;
	auto const base_mass = params.mass * (base_volume / total_volume);
	auto const upper_mass = params.mass - base_mass;

	auto const base_inertia_at_com = make_centered_solid_cylinder_inertia_tensor<BodyCOM> ({
		.mass = base_mass,
		.radius = base_radius,
		.length = params.base_length,
	});
	auto const base_offset = SpaceLength<BodyCOM> { 0_m, 0_m, 0.5 * params.base_length };
	auto const base_inertia_at_origin = inertia_tensor_com_to_point (base_mass, base_inertia_at_com, base_offset);
	auto const base_mass_moments = MassMomentsAtArm<BodyCOM> (base_mass, base_offset, base_inertia_at_origin);

	auto const upper_inertia_at_com = make_centered_solid_cylinder_inertia_tensor<BodyCOM> ({
		.mass = upper_mass,
		.radius = shaft_radius,
		.length = upper_length,
	});
	auto const upper_offset = SpaceLength<BodyCOM> { 0_m, 0_m, params.base_length + 0.5 * upper_length };
	auto const upper_inertia_at_origin = inertia_tensor_com_to_point (upper_mass, upper_inertia_at_com, upper_offset);
	auto const upper_mass_moments = MassMomentsAtArm<BodyCOM> (upper_mass, upper_offset, upper_inertia_at_origin);

	return base_mass_moments + upper_mass_moments;
}


Shape
WhipAntenna::make_shape (WhipAntennaParameters const& params)
{
	auto const shaft_radius = 0.5 * params.shaft_diameter;
	auto const base_radius = 0.5 * params.base_diameter;
	auto const tip_radius = shaft_radius;

	if (params.body_length <= params.base_length + tip_radius)
		throw nu::InvalidArgument ("whip antenna body_length must be greater than base_length + shaft_radius");

	auto const upper_length_without_tip = params.body_length - params.base_length - tip_radius;
	auto desired_cone_length = 9.0 * (base_radius - shaft_radius);

	if (desired_cone_length < 1_mm)
		desired_cone_length = 1_mm;

	auto const max_cone_length = 0.5 * upper_length_without_tip;
	auto const cone_length = std::min (desired_cone_length, max_cone_length);
	auto const shaft_length = upper_length_without_tip - cone_length;
	auto const shaft_material = make_material ({ 0x38, 0x38, 0x38 });
	auto const base_material = make_material ({ 0x20, 0x20, 0x20 });

	auto shape = make_cylinder_shape ({
		.length = params.base_length,
		.radius = base_radius,
		.num_faces = params.num_faces,
		.with_bottom = true,
		.with_top = true,
		.material = base_material,
	});

	auto shaft_shape = make_cylinder_shape ({
		.length = shaft_length,
		.radius = shaft_radius,
		.num_faces = params.num_faces,
		.with_bottom = false,
		.with_top = false,
		.material = shaft_material,
	});
	shaft_shape.translate ({ 0_m, 0_m, params.base_length + cone_length });

	auto cone_shape = make_truncated_cone_shape ({
		.length = cone_length,
		.bottom_radius = base_radius,
		.top_radius = shaft_radius,
		.num_faces = params.num_faces,
		.with_bottom = false,
		.with_top = false,
		.material = shaft_material,
	});
	cone_shape.translate ({ 0_m, 0_m, params.base_length });

	auto tip_shape = make_centered_sphere_shape ({
		.radius = tip_radius,
		.n_slices = std::max (params.num_faces, 8uz),
		.n_stacks = std::max (params.num_faces / 2, 4uz),
		.v_range = { 0_deg, +90_deg },
		.material = shaft_material,
	});
	tip_shape.translate ({ 0_m, 0_m, params.base_length + cone_length + shaft_length });

	return shape + cone_shape + shaft_shape + tip_shape;
}

} // namespace xf::sim

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
#include "various_shapes.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/geometry.h>
#include <xefis/support/simulation/rigid_body/shape_vertex.h>

// Neutrino:
#include <neutrino/numeric.h>

// Standard:
#include <algorithm>
#include <cstddef>
#include <numbers>
#include <ranges>


namespace xf::rigid_body {

Shape
make_centered_cube_shape (si::Length const edge_length, ShapeMaterial const& material)
{
	return make_centered_cube_shape (SpaceLength<BodyOrigin> { edge_length, edge_length, edge_length }, material);
}


Shape
make_centered_cube_shape (SpaceLength<BodyOrigin> const& dimensions, ShapeMaterial const& material)
{
	Shape shape;
	auto const x = 0.5 * dimensions[0];
	auto const y = 0.5 * dimensions[1];
	auto const z = 0.5 * dimensions[2];

	shape.triangles() = {
		// Front:
		{ { +x, +y, +z }, { -x, +y, +z }, { -x, -y, +z } },
		{ { +x, +y, +z }, { -x, -y, +z }, { +x, -y, +z } },
		// Right:
		{ { +x, +y, +z }, { +x, -y, +z }, { +x, -y, -z } },
		{ { +x, +y, +z }, { +x, -y, -z }, { +x, +y, -z } },
		// Top:
		{ { +x, +y, +z }, { +x, +y, -z }, { -x, +y, -z } },
		{ { +x, +y, +z }, { -x, +y, -z }, { -x, +y, +z } },
		// Back:
		{ { -x, -y, -z }, { +x, +y, -z }, { +x, -y, -z } },
		{ { -x, -y, -z }, { -x, +y, -z }, { +x, +y, -z } },
		// Left:
		{ { -x, -y, -z }, { -x, +y, +z }, { -x, +y, -z } },
		{ { -x, -y, -z }, { -x, -y, +z }, { -x, +y, +z } },
		// Bottom:
		{ { -x, -y, -z }, { +x, -y, +z }, { -x, -y, +z } },
		{ { -x, -y, -z }, { +x, -y, -z }, { +x, -y, +z } },
	};

	for (auto& triangle: shape.triangles())
	{
		set_planar_normal (triangle);
		set_material (triangle, material);
	}

	return shape;
}


Shape
make_centered_cube_shape (xf::MassMoments<BodyCOM> const& mm, ShapeMaterial const& material)
{
	// Assuming center of mass position is 0.

	auto const k = mm.mass() / 12;
	auto const inv_double_k = 1 / (2 * k);
	auto const I = mm.inertia_tensor(); // Assuming it's ortogonalized
	auto const d0 = I[0, 0];
	auto const d1 = I[1, 1];
	auto const d2 = I[2, 2];

	si::Length const x = sqrt ((-d0 +d1 +d2) * inv_double_k);
	si::Length const y = sqrt ((+d0 -d1 +d2) * inv_double_k);
	si::Length const z = sqrt ((+d0 +d1 -d2) * inv_double_k);

	return make_centered_cube_shape (SpaceLength<BodyOrigin> { x, y, z }, material);
}


Shape
make_centered_sphere_shape (SphereShapeParameters const& params)
{
	auto const slices = std::max<size_t> (params.slices, 3);
	auto const stacks = std::max<size_t> (params.stacks, 2);

	si::Angle const dh = params.h_range.extent() / slices;
	si::Angle const dv = params.v_range.extent() / stacks;

	Shape shape;
	si::Angle angle_v = params.v_range.min();

	for (size_t iv = 0; iv < stacks; ++iv, angle_v += dv)
	{
		Shape::TriangleStrip& strip = shape.triangle_strips().emplace_back();
		si::Angle angle_h = params.h_range.max();

		for (size_t ih = 0; ih < slices + 1; ++ih, angle_h += dh)
		{
			// Not the most effective (could reuse vertices), but who cares.
			auto const get_vector = [](si::Angle v, si::Angle const h)
			{
				v -= 90_deg;

				auto const w = sin (v);
				auto const x = w * sin (h);
				auto const y = w * cos (h);
				auto const z = cos (v);

				return SpaceVector<double, BodyOrigin> (x, y, z);
			};

			auto const p1 = get_vector (angle_v, angle_h);
			auto const p2 = get_vector (angle_v + dv, angle_h);

			if (params.setup_material)
			{
				auto p1_material = params.material;
				params.setup_material (p1_material, angle_v);
				strip.emplace_back (p1 * params.radius, p1, p1_material);

				auto p2_material = params.material;
				params.setup_material (p2_material, angle_v + dv);
				strip.emplace_back (p2 * params.radius, p2, p2_material);
			}
			else
			{
				strip.emplace_back (p1 * params.radius, p1, params.material);
				strip.emplace_back (p2 * params.radius, p2, params.material);
			}
		}
	}

	shape.for_all_vertices ([] (ShapeVertex& v) {
		v.set_normal (v.position() / 1_m);
	});

	return shape;
}


Shape
make_cylinder_shape (CylinderShapeParameters const& params)
{
	auto const num_faces = params.num_faces < 3u ? 3u : params.num_faces;
	Shape shape;
	Shape::TriangleStrip& strip = shape.triangle_strips().emplace_back();
	std::optional<Shape::TriangleFan> bottom;
	std::optional<Shape::TriangleFan> top;

	if (params.with_bottom)
	{
		bottom.emplace();
		bottom->emplace_back (SpaceLength<BodyOrigin> (0_m, 0_m, 0_m), SpaceVector<double, BodyOrigin> (0.0, 0.0, -1.0), params.material);
	}

	if (params.with_top)
	{
		top.emplace();
		top->emplace_back (SpaceLength<BodyOrigin> (0_m, 0_m, params.length), SpaceVector<double, BodyOrigin> (0.0, 0.0, +1.0), params.material);
	}

	si::Angle const delta = params.range.extent() / num_faces;
	si::Angle angle = params.range.min();

	for (size_t i = 0; i < num_faces + 1; ++i, angle += delta)
	{
		auto const x = sin (angle);
		auto const y = cos (angle);
		auto const x_len = params.radius * x;
		auto const y_len = params.radius * y;
		SpaceVector<double, BodyOrigin> const normal (x, y, 0);
		SpaceLength<BodyOrigin> const p1 (x_len, y_len, 0_m);
		SpaceLength<BodyOrigin> const p2 (x_len, y_len, params.length);

		strip.emplace_back (p1, normal, params.material);
		strip.emplace_back (p2, normal, params.material);

		if (params.with_bottom)
			bottom->emplace_back (p1, SpaceVector<double, BodyOrigin> (0.0, 0.0, -1.0), params.material);

		if (params.with_top)
			top->emplace_back (p2, SpaceVector<double, BodyOrigin> (0.0, 0.0, +1.0), params.material);
	}

	if (params.with_bottom)
		shape.triangle_fans().push_back (std::move (*bottom));

	if (params.with_top)
	{
		// Reverse order to keep the face facing outside:
		std::reverse (std::next (top->begin()), top->end());
		shape.triangle_fans().push_back (std::move (*top));
	}

	return shape;
}


Shape
make_cone_shape (ConeShapeParameters const& params)
{
	return make_truncated_cone_shape ({
		.length = params.length,
		.bottom_radius = params.radius,
		.top_radius = 0_m,
		.num_faces = params.num_faces,
		.with_bottom = params.with_bottom,
		.with_top = false,
		.material = params.material,
	});
}


Shape
make_truncated_cone_shape (TruncatedConeShapeParameters const& params)
{
	auto const num_faces = params.num_faces < 3u ? 3u : params.num_faces;
	Shape shape;
	Shape::TriangleStrip& cone_strip = shape.triangle_strips().emplace_back();
	std::optional<Shape::TriangleFan> top_fan;
	std::optional<Shape::TriangleFan> bottom_fan;

	if (params.with_top || params.with_bottom)
		shape.triangle_fans().reserve (2);

	if (params.with_bottom)
	{
		bottom_fan.emplace();
		bottom_fan->emplace_back (SpaceLength<BodyOrigin> (0_m, 0_m, 0_m), SpaceVector<double, BodyOrigin> (0.0, 0.0, -1.0), params.material);
	}

	if (params.with_top)
	{
		top_fan.emplace();
		top_fan->emplace_back (SpaceLength<BodyOrigin> (0_m, 0_m, params.length), SpaceVector<double, BodyOrigin> (0.0, 0.0, +1.0), params.material);
	}

	si::Angle const delta = params.range.extent() / num_faces;
	si::Angle angle = params.range.min();

	for (size_t i = 0; i < num_faces + 1; ++i, angle += delta)
	{
		using std::sin;
		using std::atan;

		auto const y = cos (angle);
		auto const x = sin (angle);
		auto const z = sin (atan ((params.bottom_radius - params.top_radius) / params.length));
		SpaceVector<double, BodyOrigin> const normal (x, y, z);
		SpaceLength<BodyOrigin> const p_bottom (x * params.bottom_radius, y * params.bottom_radius, 0_m);
		SpaceLength<BodyOrigin> const p_top (x * params.top_radius, y * params.top_radius, params.length);

		cone_strip.emplace_back (p_bottom, normal, params.material);
		cone_strip.emplace_back (p_top, normal, params.material);

		if (params.with_top)
			top_fan->emplace_back (p_top, SpaceVector<double, BodyOrigin> (0.0, 0.0, +1.0), params.material);

		if (params.with_bottom)
			bottom_fan->emplace_back (p_bottom, SpaceVector<double, BodyOrigin> (0.0, 0.0, -1.0), params.material);
	}

	if (params.with_bottom)
		shape.triangle_fans().push_back (std::move (*bottom_fan));

	if (top_fan)
		std::ranges::reverse (*top_fan);

	if (params.with_top)
		shape.triangle_fans().push_back (std::move (*top_fan));

	return shape;
}


Shape
make_solid_circle (si::Length const radius, Range<si::Angle> const range, size_t num_slices, ShapeMaterial const& material)
{
	if (num_slices < 3)
		num_slices = 3;

	Shape shape;
	Shape::TriangleFan& fan = shape.triangle_fans().emplace_back();
	fan.emplace_back (SpaceLength<BodyOrigin> (0_m, 0_m, 0_m), SpaceVector<double, BodyOrigin> (0.0, 0.0, 1.0), material);

	si::Angle const delta = range.extent() / num_slices;
	si::Angle angle = range.min();

	for (size_t i = 0; i < num_slices + 1; ++i, angle += delta)
	{
		auto const y = sin (angle);
		auto const x = cos (angle);

		fan.emplace_back (SpaceLength<BodyOrigin> (x * radius, y * radius, 0_m), SpaceVector<double, BodyOrigin> (0.0, 0.0, +1.0), material);
	}

	return shape;
}


Shape
make_airfoil_shape (AirfoilShapeParameters const& params)
{
	Shape shape;
	Shape::TriangleStrip& strip = shape.triangle_strips().emplace_back();
	std::optional<Shape::TriangleFan> bottom;
	std::optional<Shape::TriangleFan> top;

	if (params.with_bottom)
	{
		bottom.emplace();
		bottom->emplace_back (SpaceLength<BodyOrigin> (0_m, 0_m, 0_m), SpaceVector<double, BodyOrigin> (0.0, 0.0, -1.0), params.material);
	}

	if (params.with_top)
	{
		top.emplace();
		top->emplace_back (SpaceLength<BodyOrigin> (0_m, 0_m, params.wing_length), SpaceVector<double, BodyOrigin> (0.0, 0.0, +1.0), params.material);
	}

	auto const n_points = params.spline.points().size();

	for (ptrdiff_t i = neutrino::to_signed (n_points) + 1; i > 0; --i)
	{
		auto const prev_point = params.spline.points()[wrap_array_index (i - 1, n_points)];
		auto const point = params.spline.points()[to_unsigned (i) % n_points];
		auto const next_point = params.spline.points()[wrap_array_index (i + 1, n_points)];

		auto const x_len = params.chord_length * point[0];
		auto const y_len = params.chord_length * point[1];
		SpaceLength<BodyOrigin> const p1 (x_len, y_len, 0_m);
		SpaceLength<BodyOrigin> const p2 (x_len, y_len, params.wing_length);

		SpaceVector<double, BodyOrigin> const z_versor (0, 0, 1);
		SpaceVector<double, BodyOrigin> const k_towards_prev = SpaceVector<double, BodyOrigin> (prev_point[0], prev_point[1], 0) - SpaceVector<double, BodyOrigin> (point[0], point[1], 0);
		SpaceVector<double, BodyOrigin> const k_towards_next = SpaceVector<double, BodyOrigin> (next_point[0], next_point[1], 0) - SpaceVector<double, BodyOrigin> (point[0], point[1], 0);
		SpaceVector<double, BodyOrigin> const normal_with_prev (cross_product (z_versor, k_towards_prev));
		SpaceVector<double, BodyOrigin> const normal_with_next (cross_product (k_towards_next, z_versor));
		SpaceVector<double, BodyOrigin> const normal = (normal_with_prev + normal_with_next).normalized();

		strip.emplace_back (p1, normal, params.material);
		strip.emplace_back (p2, normal, params.material);

		if (params.with_bottom)
			bottom->emplace_back (p1, SpaceVector<double, BodyOrigin> (0.0, 0.0, -1.0), params.material);

		if (params.with_top)
			top->emplace_back (p2, SpaceVector<double, BodyOrigin> (0.0, 0.0, +1.0), params.material);
	}

	if (params.with_bottom)
		shape.triangle_fans().push_back (std::move (*bottom));

	if (params.with_top)
	{
		// Reverse order to keep the face facing outside:
		std::reverse (std::next (top->begin()), top->end());
		shape.triangle_fans().push_back (std::move (*top));
	}

	return shape;
}


Shape
make_propeller_shape (PropellerShapeParameters const& params)
{
	using std::numbers::pi;

	Shape shape;
	// Reserve strips for each blade (front and back side).
	shape.triangle_strips().reserve (2 * params.blades);
	auto const blade_length = 0.5 * params.diameter;
	auto const angle_between_blades = 360_deg / params.blades;
	auto const max_pitch_radius = 0.292 * blade_length;
	auto const width = blade_length / 10; // Looks good like this.
	auto const pitch_height = width * params.pitch / (2 * pi * max_pitch_radius);
	auto const pitch_height_b = 0.65 * pitch_height;
	auto const pitch_height_f = 0.35 * pitch_height;
	auto const point_spacing = 1.0 / params.points_per_blade;
	auto const rotation_direction_factor = (params.rotation_direction == ClockWise) ? +1.0 : -1.0;

	for (uint16_t blade = 0; blade < params.blades; ++blade)
	{
		if (blade > 0)
			shape.rotate (xf::z_rotation<BodyOrigin> (angle_between_blades));

		auto strip = Shape::TriangleStrip();
		// Center of the blade:
		strip.emplace_back (SpaceLength<BodyOrigin> { 0_m, 0_m, 0_m }, params.material);

		for (uint32_t p = 0; p < params.points_per_blade; ++p)
		{
			// More triangles at the center and at the tip than on the center:
			auto const p_norm = 0.5 + 0.5 * -std::cos (p * point_spacing * pi);

			auto const y = p_norm * blade_length;
			auto const x_l = width * std::pow (std::sin (p_norm * pi), 0.5) * rotation_direction_factor;
			auto const x_t = x_l * 0.5; // Trailing edge is flatter.
			auto const z_b = pitch_height_b * std::pow (std::sin (std::pow (p_norm, 0.7) * pi), 3.0);
			auto const z_f = pitch_height_f * square (std::sin (p_norm * pi));
			strip.emplace_back (SpaceLength<BodyOrigin> { -x_t, y, -z_b }, params.material);
			strip.emplace_back (SpaceLength<BodyOrigin> { +x_l, y, +z_f }, params.material);
		}

		// Tip of the blade:
		strip.emplace_back (SpaceLength<BodyOrigin> { 0_m, blade_length, -0.01 * width }, params.material);
		strip.emplace_back (SpaceLength<BodyOrigin> { 0_m, blade_length, 0_m }, params.material);

		for (auto triangle: strip | std::views::slide (3))
			set_planar_normal (triangle);

		// For back faces, add the same points in the reverse order:
		auto back_strip = strip;
		std::ranges::reverse (back_strip);
		back_strip.pop_back();

		shape.triangle_strips().push_back (strip);
		shape.triangle_strips().push_back (back_strip);
	}

	shape.translate ({ 0_m, 0_m, pitch_height_b });
	return shape;
}


Shape
make_propeller_cone_shape (PropellerConeShapeParameters const& params)
{
	auto const cylinder_shape = make_cylinder_shape ({
		.length = params.base_length,
		.radius = params.radius,
		.num_faces = params.num_faces,
		.with_bottom = true,
		.with_top = false,
		.material = params.material,
	});

	auto cone_shape = make_cone_shape ({ .length = params.cone_length, .radius = params.radius, .num_faces = params.num_faces, .with_bottom = false, .material = params.material });
	cone_shape.translate ({ 0_m, 0_m, params.base_length });

	return cylinder_shape + cone_shape;
}


Shape
make_motor_shape (MotorShapeParameters const& params)
{
	auto const back_shaft_length = 0.5 * params.back_cone_length;
	auto back_shaft_shape = make_cylinder_shape ({
		.length = back_shaft_length,
		.radius = params.shaft_radius,
		.num_faces = 6,
		.with_bottom = true,
		.material = params.shaft_material,
	});
	back_shaft_shape.translate ({ 0_m, 0_m, -back_shaft_length - params.back_cone_length - params.center_length - params.front_cone_length });

	auto back_cone_shape = make_truncated_cone_shape ({
		.length = params.back_cone_length,
		.bottom_radius = params.back_radius,
		.top_radius = params.center_radius,
		.num_faces = params.num_faces,
		.with_bottom = true,
		.material = params.cones_material,
	});
	back_cone_shape.translate ({ 0_m, 0_m, -params.back_cone_length - params.center_length - params.front_cone_length });

	auto cylinder_shape = make_cylinder_shape ({
		.length = params.center_length,
		.radius = params.center_radius,
		.num_faces = params.num_faces,
		.material = params.center_material,
	});
	cylinder_shape.translate ({ 0_m, 0_m, -params.center_length - params.front_cone_length });

	auto front_cone_shape = make_truncated_cone_shape ({
		.length = params.front_cone_length,
		.bottom_radius = params.center_radius,
		.top_radius = params.front_radius,
		.num_faces = params.num_faces,
		.with_top = true,
		.material = params.cones_material,
	});
	front_cone_shape.translate ({ 0_m, 0_m, -params.front_cone_length });

	auto shaft_shape = make_cylinder_shape ({
		.length = params.shaft_length,
		.radius = params.shaft_radius,
		.num_faces = 6,
		.material = params.shaft_material,
	});

	auto const sticker_length = 0.6 * params.center_length;
	auto const sticker_faces = std::max<std::size_t> (3u, params.num_faces / 3u);
	auto sticker_shape = make_cylinder_shape ({
		.length = 0.6 * params.center_length,
		.radius = params.center_radius + 0.1_mm,
		.range = { 0_deg, 360_deg / params.num_faces * sticker_faces },
		.num_faces = sticker_faces,
		.material = params.sticker_material,
	});
	sticker_shape.translate ({ 0_m, 0_m, -0.5 * sticker_length - 0.5 * params.center_length - params.front_cone_length });

	return back_cone_shape + cylinder_shape + front_cone_shape + shaft_shape + back_shaft_shape + sticker_shape;
}


Shape
make_center_of_mass_symbol_shape (si::Length const radius, ShapeMaterial const& a, ShapeMaterial const& b)
{
	return make_centered_sphere_shape ({ .radius = radius, .slices = 8, .stacks = 8, .h_range = {   0_deg,  90_deg }, .v_range = { -90_deg,   0_deg }, .material = a })
		 + make_centered_sphere_shape ({ .radius = radius, .slices = 8, .stacks = 8, .h_range = {   0_deg,  90_deg }, .v_range = {   0_deg, +90_deg }, .material = b })
		 + make_centered_sphere_shape ({ .radius = radius, .slices = 8, .stacks = 8, .h_range = {  90_deg, 180_deg }, .v_range = { -90_deg,   0_deg }, .material = b })
		 + make_centered_sphere_shape ({ .radius = radius, .slices = 8, .stacks = 8, .h_range = {  90_deg, 180_deg }, .v_range = {   0_deg, +90_deg }, .material = a })
		 + make_centered_sphere_shape ({ .radius = radius, .slices = 8, .stacks = 8, .h_range = { 180_deg, 270_deg }, .v_range = { -90_deg,   0_deg }, .material = a })
		 + make_centered_sphere_shape ({ .radius = radius, .slices = 8, .stacks = 8, .h_range = { 180_deg, 270_deg }, .v_range = {   0_deg, +90_deg }, .material = b })
		 + make_centered_sphere_shape ({ .radius = radius, .slices = 8, .stacks = 8, .h_range = { 270_deg, 360_deg }, .v_range = { -90_deg,   0_deg }, .material = b })
		 + make_centered_sphere_shape ({ .radius = radius, .slices = 8, .stacks = 8, .h_range = { 270_deg, 360_deg }, .v_range = {   0_deg, +90_deg }, .material = a });
}


void
set_planar_normal (Shape::Triangle& triangle)
{
	if (triangle.size() != 3)
		throw InvalidArgument ("set_planar_normal (Shape::Triangle&): std::size (triangle) must be 3");

	auto const normal = triangle_surface_normal (triangle[0].position(),
												 triangle[1].position(),
												 triangle[2].position());

	for (auto& vertex: triangle)
		vertex.set_normal (normal);
}


void
set_planar_normal (std::span<ShapeVertex> triangle)
{
	if (triangle.size() != 3)
		throw InvalidArgument ("set_planar_normal (span<>): std::size (triangle) must be 3");

	auto const normal = triangle_surface_normal (triangle[0].position(),
												 triangle[1].position(),
												 triangle[2].position());

	for (auto& vertex: triangle)
		vertex.set_normal (normal);
}


void
negate_normals (std::vector<ShapeVertex>& vertices)
{
	for (auto& vertex: vertices)
		if (auto normal = vertex.normal())
			vertex.set_normal (-*normal);
}


void
negate_normals (Shape& shape)
{
	for (auto& triangle: shape.triangles())
		negate_normals (triangle);

	for (auto& strip: shape.triangle_strips())
		negate_normals (strip);

	for (auto& fan: shape.triangle_fans())
		negate_normals (fan);
}


void
set_material (std::vector<ShapeVertex>& vertices, ShapeMaterial const& material)
{
	for (auto& vertex: vertices)
		vertex.set_material (material);
}

} // namespace xf::rigid_body


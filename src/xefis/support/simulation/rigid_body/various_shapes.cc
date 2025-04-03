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
#include <neutrino/wait_group.h>

// Standard:
#include <algorithm>
#include <cstddef>
#include <numbers>
#include <ranges>
#include <type_traits>
#include <vector>


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
		{ .vertices = { { +x, +y, +z }, { -x, +y, +z }, { -x, -y, +z } } },
		{ .vertices = { { +x, +y, +z }, { -x, -y, +z }, { +x, -y, +z } } },
		// Right:
		{ .vertices = { { +x, +y, +z }, { +x, -y, +z }, { +x, -y, -z } } },
		{ .vertices = { { +x, +y, +z }, { +x, -y, -z }, { +x, +y, -z } } },
		// Top:
		{ .vertices = { { +x, +y, +z }, { +x, +y, -z }, { -x, +y, -z } } },
		{ .vertices = { { +x, +y, +z }, { -x, +y, -z }, { -x, +y, +z } } },
		// Back:
		{ .vertices = { { -x, -y, -z }, { +x, +y, -z }, { +x, -y, -z } } },
		{ .vertices = { { -x, -y, -z }, { -x, +y, -z }, { +x, +y, -z } } },
		// Left:
		{ .vertices = { { -x, -y, -z }, { -x, +y, +z }, { -x, +y, -z } } },
		{ .vertices = { { -x, -y, -z }, { -x, -y, +z }, { -x, +y, +z } } },
		// Bottom:
		{ .vertices = { { -x, -y, -z }, { +x, -y, +z }, { -x, -y, +z } } },
		{ .vertices = { { -x, -y, -z }, { +x, -y, -z }, { +x, -y, +z } } },
	};

	for (auto& triangle: shape.triangles())
	{
		set_planar_normal (triangle);
		set_material (triangle.vertices, material);
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
make_cube_shape (xf::MassMomentsAtArm<BodyCOM> const& mm, ShapeMaterial const& material)
{
	auto shape = make_centered_cube_shape (mm.centered_at_center_of_mass(), material);
	shape.translate (math::coordinate_system_cast<BodyOrigin, void> (mm.center_of_mass_position())); // FIXME maybe -?
	return shape;
}


/**
 * Fill in missing vertex data on a sphere by copying shared vertices between adjacent triangle strips.
 *
 * In the sphere mesh, certain vertices (specifically, the odd-indexed ones in the upper triangle strip)
 * are intentionally left uncomputed. This function iterates over each pair of adjacent triangle strips,
 * and for every odd-indexed vertex in the upper strip, it copies the corresponding vertex from the lower strip.
 * This ensures that the shared vertices between strips are consistent, which is important for correct mesh rendering.
 *
 * \param	shape
 *			The Shape object containing the triangle strips that represent the sphere.
 */
void
fill_in_uncomputed_points_on_sphere (Shape& shape)
{
	// Copy the vertices for shared points:
	for (auto const strips: shape.triangle_strips() | std::views::slide (2))
	{
		auto const& lower = strips[0];
		auto& upper = strips[1];

		// Odd points are not calculated, need to be copied from the even points on the next strip.
		for (std::size_t i = 1; i < lower.vertices.size(); i += 2)
			upper.vertices[i] = lower.vertices[i - 1];
	}
}


/**
 * Compute and assign normalized normal vectors for each vertex of a sphere.
 *
 * \param	shape
 *			The Shape object containing the sphere vertices.
 * \param	radius
 *			The radius of the sphere used to normalize the vertex positions.
 */
void
set_sphere_normals (Shape& shape, si::Length radius)
{
	shape.for_all_vertices ([&] (ShapeVertex& v) {
		v.set_normal (v.position() / radius);
	});
}


template<class SetupMaterial>
	Shape
	make_centered_sphere_shape (SphereShapeParameters const& params, SetupMaterial const setup_material)
	{
		auto constexpr synchronous_setup_material = std::is_same<SetupMaterial, SynchronousSetupMaterial>();
		auto constexpr asynchronous_setup_material = std::is_same<SetupMaterial, AsynchronousSetupMaterial>();
		auto constexpr future_based_setup_material = std::is_same<SetupMaterial, FutureBasedSetupMaterial>();

		auto const n_slices = std::max<size_t> (params.n_slices, 3);
		auto const n_stacks = std::max<size_t> (params.n_stacks, 2);

		si::Angle const dh = params.h_range.extent() / n_slices;
		si::Angle const dv = params.v_range.extent() / n_stacks;

		Shape shape;
		shape.triangle_strips().reserve (n_stacks);

		[[maybe_unused]] auto all_materials_set_up = std::conditional_t<asynchronous_setup_material, neutrino::WaitGroup, std::monostate>();
		[[maybe_unused]] auto all_setup_material_futures = std::conditional_t<future_based_setup_material, std::vector<std::future<void>>, std::monostate>();

		if constexpr (future_based_setup_material)
		{
			// +1 because first stack has both lower and upper points computed:
			all_setup_material_futures.reserve ((n_stacks + 1) * (n_slices + 1));
		}

		si::Angle angle_v = params.v_range.min();

		// TODO Optimize poles (setup_material is called multiple times for each pole)
		for (size_t iv = 0; iv < n_stacks; ++iv, angle_v += dv)
		{
			Shape::TriangleStrip& strip = shape.triangle_strips().emplace_back();
			strip.texture = params.texture;
			auto& vertices = strip.vertices;
			vertices.reserve (2 * (n_slices + 1));
			si::Angle angle_h = params.h_range.min();

			auto const add_vertex = [&] (si::LonLat const lonlat) {
				auto const cartesian_position = math::coordinate_system_cast<BodyOrigin, void> (to_cartesian (lonlat));
				auto& vertex = vertices.emplace_back (cartesian_position * params.radius, params.material);

				if constexpr (synchronous_setup_material)
					setup_material (vertex.material(), lonlat);
				else if constexpr (asynchronous_setup_material)
					setup_material (vertex.material(), lonlat, all_materials_set_up.make_work_token());
				else if constexpr (future_based_setup_material)
					all_setup_material_futures.push_back (setup_material (vertex.material(), lonlat));
			};

			for (size_t ih = 0; ih < n_slices + 1; ++ih, angle_h += dh)
			{
				add_vertex ({ angle_h, angle_v + dv });

				if (iv == 0)
					add_vertex ({ angle_h, angle_v });
				else
					vertices.emplace_back(); // This point will be calculated in fill_in_uncomputed_points_on_sphere().
			}
		}

		// Wait until all setup_material() callbacks finish:
		if constexpr (asynchronous_setup_material)
			all_materials_set_up.wait();
		else if constexpr (future_based_setup_material)
			for (auto& future: all_setup_material_futures)
				future.wait();

		fill_in_uncomputed_points_on_sphere (shape);
		set_sphere_normals (shape, params.radius);

		return shape;
	}


Shape
make_centered_sphere_shape (SphereShapeParameters const& params)
{
	if (auto const* const callback = std::get_if<SynchronousSetupMaterial> (&params.setup_material))
	{
		if (*callback)
			return make_centered_sphere_shape (params, *callback);
	}
	else if (auto const* const callback = std::get_if<AsynchronousSetupMaterial> (&params.setup_material))
	{
		if (*callback)
			return make_centered_sphere_shape (params, *callback);
	}
	else if (auto const* const callback = std::get_if<FutureBasedSetupMaterial> (&params.setup_material))
	{
		if (*callback)
			return make_centered_sphere_shape (params, *callback);
	}

	return make_centered_sphere_shape (params, std::monostate());
}


template<class SetupMaterial>
	Shape
	make_centered_irregular_sphere_shape (IrregularSphereShapeParameters const& params, SetupMaterial const setup_material)
	{
		auto constexpr synchronous_setup_material = std::is_same<SetupMaterial, SynchronousSetupMaterial>();
		auto constexpr asynchronous_setup_material = std::is_same<SetupMaterial, AsynchronousSetupMaterial>();
		auto constexpr future_based_setup_material = std::is_same<SetupMaterial, FutureBasedSetupMaterial>();

		auto const n_slices = params.slice_angles.size();
		auto const n_stacks = params.stack_angles.size();

		if (n_slices < 3)
			throw Exception ("IrregularSphereShapeParameters: must have at least 3 slices");

		if (n_stacks < 2)
			throw Exception ("IrregularSphereShapeParameters: must have at least 2 stacks");

		Shape shape;
		shape.triangle_strips().reserve (n_stacks);

		[[maybe_unused]] auto all_materials_set_up = std::conditional_t<asynchronous_setup_material, neutrino::WaitGroup, std::monostate>();
		[[maybe_unused]] auto all_setup_material_futures = std::conditional_t<future_based_setup_material, std::vector<std::future<void>>, std::monostate>();

		if constexpr (future_based_setup_material)
		{
			// +1 because first stack has both lower and upper points computed:
			all_setup_material_futures.reserve ((params.stack_angles.size() + 1) * params.slice_angles.size());
		}

		auto first_strip = true;
		// First, calculate only the lower-latitude points for each stack.
		// The top ones are shared, so we'll just copy them later. This way
		// we'll avoid calling setup_material() twice for the same points.
		// TODO Optimize poles (setup_material is called multiple times for each pole)
		for (auto const latitudes: params.stack_angles | std::views::slide (2))
		{
			Shape::TriangleStrip& strip = shape.triangle_strips().emplace_back();
			strip.texture = params.texture;
			auto& vertices = strip.vertices;
			vertices.reserve (2 * n_slices);

			auto const add_vertex = [&] (si::LonLat const lonlat) {
				auto const cartesian_position = math::coordinate_system_cast<BodyOrigin, void> (to_cartesian (lonlat));
				auto& vertex = vertices.emplace_back (cartesian_position * params.radius, params.material);

				if constexpr (synchronous_setup_material)
					setup_material (vertex.material(), lonlat);
				else if constexpr (asynchronous_setup_material)
					setup_material (vertex.material(), lonlat, all_materials_set_up.make_work_token());
				else if constexpr (future_based_setup_material)
					all_setup_material_futures.push_back (setup_material (vertex.material(), lonlat));
			};

			for (auto const longitude: params.slice_angles)
			{
				add_vertex ({ longitude, latitudes[1] });

				if (first_strip)
					add_vertex ({ longitude, latitudes[0] });
				else
					vertices.emplace_back(); // This point will be calculated in fill_in_uncomputed_points_on_sphere().
			}

			first_strip = false;
		}

		// Wait until all setup_material() callbacks finish:
		if constexpr (asynchronous_setup_material)
			all_materials_set_up.wait();
		else if constexpr (future_based_setup_material)
			for (auto& future: all_setup_material_futures)
				future.wait();

		fill_in_uncomputed_points_on_sphere (shape);
		set_sphere_normals (shape, params.radius);

		return shape;
	}


Shape
make_centered_irregular_sphere_shape (IrregularSphereShapeParameters const& params)
{
	if (auto const* const callback = std::get_if<SynchronousSetupMaterial> (&params.setup_material))
	{
		if (*callback)
			return make_centered_irregular_sphere_shape (params, *callback);
	}
	else if (auto const* const callback = std::get_if<AsynchronousSetupMaterial> (&params.setup_material))
	{
		if (*callback)
			return make_centered_irregular_sphere_shape (params, *callback);
	}
	else if (auto const* const callback = std::get_if<FutureBasedSetupMaterial> (&params.setup_material))
	{
		if (*callback)
			return make_centered_irregular_sphere_shape (params, *callback);
	}

	return make_centered_irregular_sphere_shape (params, std::monostate());
}


Shape
make_cylinder_shape (CylinderShapeParameters const& params)
{
	auto const num_faces = params.num_faces < 3u ? 3u : params.num_faces;
	Shape shape;
	Shape::TriangleStrip& strip = shape.triangle_strips().emplace_back();
	auto& vertices = strip.vertices;
	vertices.reserve (2 * (num_faces + 1));
	std::optional<Shape::TriangleFan> bottom;
	std::optional<Shape::TriangleFan> top;

	if (params.with_bottom)
	{
		bottom.emplace();
		bottom->vertices.reserve (2 + num_faces);
		bottom->vertices.emplace_back (SpaceLength<BodyOrigin> (0_m, 0_m, 0_m), SpaceVector<double, BodyOrigin> (0.0, 0.0, -1.0), params.material);
	}

	if (params.with_top)
	{
		top.emplace();
		top->vertices.reserve (2 + num_faces);
		top->vertices.emplace_back (SpaceLength<BodyOrigin> (0_m, 0_m, params.length), SpaceVector<double, BodyOrigin> (0.0, 0.0, +1.0), params.material);
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

		vertices.emplace_back (p1, normal, params.material);
		vertices.emplace_back (p2, normal, params.material);

		if (params.with_bottom)
			bottom->vertices.emplace_back (p1, SpaceVector<double, BodyOrigin> (0.0, 0.0, -1.0), params.material);

		if (params.with_top)
			top->vertices.emplace_back (p2, SpaceVector<double, BodyOrigin> (0.0, 0.0, +1.0), params.material);
	}

	if (params.with_bottom)
		shape.triangle_fans().push_back (std::move (*bottom));

	if (params.with_top)
	{
		// Reverse order to keep the face facing outside:
		std::reverse (std::next (top->vertices.begin()), top->vertices.end());
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
	cone_strip.vertices.reserve (2 * (num_faces + 1));
	std::optional<Shape::TriangleFan> top_fan;
	std::optional<Shape::TriangleFan> bottom_fan;

	if (params.with_top || params.with_bottom)
		shape.triangle_fans().reserve (2);

	if (params.with_bottom)
	{
		bottom_fan.emplace();
		bottom_fan->vertices.reserve (2 + num_faces);
		bottom_fan->vertices.emplace_back (SpaceLength<BodyOrigin> (0_m, 0_m, 0_m), SpaceVector<double, BodyOrigin> (0.0, 0.0, -1.0), params.material);
	}

	if (params.with_top)
	{
		top_fan.emplace();
		top_fan->vertices.reserve (2 + num_faces);
		top_fan->vertices.emplace_back (SpaceLength<BodyOrigin> (0_m, 0_m, params.length), SpaceVector<double, BodyOrigin> (0.0, 0.0, +1.0), params.material);
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

		cone_strip.vertices.emplace_back (p_bottom, normal, params.material);
		cone_strip.vertices.emplace_back (p_top, normal, params.material);

		if (params.with_top)
			top_fan->vertices.emplace_back (p_top, SpaceVector<double, BodyOrigin> (0.0, 0.0, +1.0), params.material);

		if (params.with_bottom)
			bottom_fan->vertices.emplace_back (p_bottom, SpaceVector<double, BodyOrigin> (0.0, 0.0, -1.0), params.material);
	}

	if (params.with_bottom)
		shape.triangle_fans().push_back (std::move (*bottom_fan));

	if (top_fan)
		std::ranges::reverse (top_fan->vertices);

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
	auto& vertices = fan.vertices;
	vertices.reserve (2 + num_slices);
	vertices.emplace_back (SpaceLength<BodyOrigin> (0_m, 0_m, 0_m), SpaceVector<double, BodyOrigin> (0.0, 0.0, 1.0), material);

	si::Angle const delta = range.extent() / num_slices;
	si::Angle angle = range.min();

	for (size_t i = 0; i < num_slices + 1; ++i, angle += delta)
	{
		auto const y = sin (angle);
		auto const x = cos (angle);

		vertices.emplace_back (SpaceLength<BodyOrigin> (x * radius, y * radius, 0_m), SpaceVector<double, BodyOrigin> (0.0, 0.0, +1.0), material);
	}

	return shape;
}


Shape
make_airfoil_shape (AirfoilShapeParameters const& params)
{
	auto const n_points = params.spline.points().size();
	Shape shape;
	Shape::TriangleStrip& strip = shape.triangle_strips().emplace_back();
	strip.vertices.reserve (2 * (n_points + 1));
	std::optional<Shape::TriangleFan> bottom;
	std::optional<Shape::TriangleFan> top;

	if (params.with_bottom)
	{
		bottom.emplace();
		bottom->vertices.reserve (2 + n_points);
		bottom->vertices.emplace_back (SpaceLength<BodyOrigin> (0_m, 0_m, 0_m), SpaceVector<double, BodyOrigin> (0.0, 0.0, -1.0), params.material);
	}

	if (params.with_top)
	{
		top.emplace();
		top->vertices.reserve (2 + n_points);
		top->vertices.emplace_back (SpaceLength<BodyOrigin> (0_m, 0_m, params.wing_length), SpaceVector<double, BodyOrigin> (0.0, 0.0, +1.0), params.material);
	}

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

		strip.vertices.emplace_back (p1, normal, params.material);
		strip.vertices.emplace_back (p2, normal, params.material);

		if (params.with_bottom)
			bottom->vertices.emplace_back (p1, SpaceVector<double, BodyOrigin> (0.0, 0.0, -1.0), params.material);

		if (params.with_top)
			top->vertices.emplace_back (p2, SpaceVector<double, BodyOrigin> (0.0, 0.0, +1.0), params.material);
	}

	if (params.with_bottom)
		shape.triangle_fans().push_back (std::move (*bottom));

	if (params.with_top)
	{
		// Reverse order to keep the face facing outside:
		std::reverse (std::next (top->vertices.begin()), top->vertices.end());
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
		auto& vertices = strip.vertices;
		auto const total_strip_size = 1 // (center of the blade)
									+ 2 * params.points_per_blade
									+ 2; // Tip of the blade;
		vertices.reserve (total_strip_size);
		// Center of the blade:
		vertices.emplace_back (SpaceLength<BodyOrigin> { 0_m, 0_m, 0_m }, params.material);

		for (uint32_t p = 0; p < params.points_per_blade; ++p)
		{
			// More triangles at the center and at the tip than on the center:
			auto const p_norm = 0.5 + 0.5 * -std::cos (p * point_spacing * pi);

			auto const y = p_norm * blade_length;
			auto const x_l = width * std::pow (std::sin (p_norm * pi), 0.5) * rotation_direction_factor;
			auto const x_t = x_l * 0.5; // Trailing edge is flatter.
			auto const z_b = pitch_height_b * std::pow (std::sin (std::pow (p_norm, 0.7) * pi), 3.0);
			auto const z_f = pitch_height_f * square (std::sin (p_norm * pi));
			vertices.emplace_back (SpaceLength<BodyOrigin> { -x_t, y, -z_b }, params.material);
			vertices.emplace_back (SpaceLength<BodyOrigin> { +x_l, y, +z_f }, params.material);
		}

		// Tip of the blade:
		vertices.emplace_back (SpaceLength<BodyOrigin> { 0_m, blade_length, -0.01 * width }, params.material);
		vertices.emplace_back (SpaceLength<BodyOrigin> { 0_m, blade_length, 0_m }, params.material);

		for (auto triangle: vertices | std::views::slide (3))
			set_planar_normal (triangle);

		// For back faces, add the same points in the reverse order:
		auto back_vertices = vertices;
		std::ranges::reverse (back_vertices);
		back_vertices.pop_back();

		shape.triangle_strips().push_back (rigid_body::Shape::TriangleStrip { .vertices = vertices });
		shape.triangle_strips().push_back (rigid_body::Shape::TriangleStrip { .vertices = back_vertices });
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
	return make_centered_sphere_shape ({ .radius = radius, .n_slices = 8, .n_stacks = 4, .h_range = {   0_deg,  90_deg }, .v_range = { -90_deg,   0_deg }, .material = a })
		 + make_centered_sphere_shape ({ .radius = radius, .n_slices = 8, .n_stacks = 4, .h_range = {   0_deg,  90_deg }, .v_range = {   0_deg, +90_deg }, .material = b })
		 + make_centered_sphere_shape ({ .radius = radius, .n_slices = 8, .n_stacks = 4, .h_range = {  90_deg, 180_deg }, .v_range = { -90_deg,   0_deg }, .material = b })
		 + make_centered_sphere_shape ({ .radius = radius, .n_slices = 8, .n_stacks = 4, .h_range = {  90_deg, 180_deg }, .v_range = {   0_deg, +90_deg }, .material = a })
		 + make_centered_sphere_shape ({ .radius = radius, .n_slices = 8, .n_stacks = 4, .h_range = { 180_deg, 270_deg }, .v_range = { -90_deg,   0_deg }, .material = a })
		 + make_centered_sphere_shape ({ .radius = radius, .n_slices = 8, .n_stacks = 4, .h_range = { 180_deg, 270_deg }, .v_range = {   0_deg, +90_deg }, .material = b })
		 + make_centered_sphere_shape ({ .radius = radius, .n_slices = 8, .n_stacks = 4, .h_range = { 270_deg, 360_deg }, .v_range = { -90_deg,   0_deg }, .material = b })
		 + make_centered_sphere_shape ({ .radius = radius, .n_slices = 8, .n_stacks = 4, .h_range = { 270_deg, 360_deg }, .v_range = {   0_deg, +90_deg }, .material = a });
}


void
set_planar_normal (Shape::Triangle& triangle)
{
	if (triangle.vertices.size() != 3)
		throw InvalidArgument ("set_planar_normal (Shape::Triangle&): std::size (triangle) must be 3");

	auto const normal = triangle_surface_normal (triangle.vertices[0].position(),
												 triangle.vertices[1].position(),
												 triangle.vertices[2].position());

	for (auto& vertex: triangle.vertices)
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
		negate_normals (triangle.vertices);

	for (auto& strip: shape.triangle_strips())
		negate_normals (strip.vertices);

	for (auto& fan: shape.triangle_fans())
		negate_normals (fan.vertices);
}


void
set_material (std::vector<ShapeVertex>& vertices, ShapeMaterial const& material)
{
	for (auto& vertex: vertices)
		vertex.set_material (material);
}

} // namespace xf::rigid_body


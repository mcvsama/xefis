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

#ifndef XEFIS__SUPPORT__SIMULATION__RIGID_BODY__VARIOUS_SHAPES_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__RIGID_BODY__VARIOUS_SHAPES_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/aerodynamics/airfoil_spline.h>
#include <xefis/support/simulation/rigid_body/shape.h>
#include <xefis/support/simulation/rigid_body/shape_material.h>

// Standard:
#include <cstddef>


namespace xf::rigid_body {

// Called by make_sphere to get material for vertices.
using MakeSphereMaterialCallback	= std::function<void (rigid_body::ShapeMaterial&, si::Angle latitude)>;


/**
 * Make a cube of given size.
 */
rigid_body::Shape
make_cube_shape (si::Length edge_length, rigid_body::ShapeMaterial const& material = {});

/**
 * Make sphere of given radius.
 */
rigid_body::Shape
make_sphere_shape (si::Length radius, size_t slices, size_t stacks,
				   Range<si::Angle> h_range = { 0_deg, 360_deg }, Range<si::Angle> v_range = { -90_deg, +90_deg },
				   rigid_body::ShapeMaterial const& material = {}, MakeSphereMaterialCallback = nullptr);

/**
 * Make a rod shape without bottom/top faces, placed along the Z axis.
 */
rigid_body::Shape
make_cylinder_shape (si::Length length, si::Length radius, size_t num_faces, bool with_front_and_back = true,
					 rigid_body::ShapeMaterial const& = {});

/**
 * Make a cone shape placed along the Z axis with pointy part pointing towards positive Z values.
 */
rigid_body::Shape
make_cone_shape (si::Length length, si::Length radius, size_t num_faces, bool with_bottom = true,
				 rigid_body::ShapeMaterial const& = {});

/**
 * Make a solid circle placed on X-Y plane.
 */
rigid_body::Shape
make_solid_circle (si::Length radius, size_t num_slices, rigid_body::ShapeMaterial const& = {});

/**
 * Make a wing shape.
 */
rigid_body::Shape
make_airfoil_shape (AirfoilSpline const& spline, si::Length chord_length, si::Length wing_length, bool with_front_and_back = true,
					rigid_body::ShapeMaterial const& = {});


/**
 * Set planar normals for triangles, that is make each vertex' normal
 * perpendicular to the surface of the triangle.
 */
template<class TriangleIterator>
	inline void
	set_planar_normals (TriangleIterator begin, TriangleIterator end)
	{
		for (auto triangle = begin; triangle != end; ++triangle)
		{
			auto normal = triangle_surface_normal (*triangle);

			for (auto& vertex: *triangle)
				vertex.set_normal (normal);
		}
	}


/**
 * Negate normals for all given vertices.
 */
inline void
negate_normals (std::vector<ShapeVertex>& vertices)
{
	for (auto& vertex: vertices)
		if (auto normal = vertex.normal())
			vertex.set_normal (-*normal);
}


/**
 * Negate normals for all given triangles.
 */
template<class TriangleIterator>
	inline void
	negate_normals (TriangleIterator begin, TriangleIterator end)
	{
		for (auto triangle = begin; triangle != end; ++triangle)
			for (auto& vertex: *triangle)
				if (auto normal = vertex.normal())
					vertex.set_normal (-*normal);
	}


/**
 * Negate all normals in given shape.
 */
inline void
negate_normals (Shape& shape)
{
	negate_normals (shape.triangles().begin(), shape.triangles().end());
	negate_normals (shape.triangle_strips().begin(), shape.triangle_strips().end());
	negate_normals (shape.triangle_fans().begin(), shape.triangle_fans().end());
}


/**
 * Set given material for all vertices in all triangles.
 */
template<class TriangleIterator>
	inline void
	set_material (TriangleIterator begin, TriangleIterator end, ShapeMaterial const& material)
	{
		for (auto triangle = begin; triangle != end; ++triangle)
			for (auto& vertex: *triangle)
				vertex.set_material (material);
	}

} // namespace xf::rigid_body

#endif


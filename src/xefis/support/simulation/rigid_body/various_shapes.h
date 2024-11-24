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
#include <xefis/support/math/geometry.h>
#include <xefis/support/nature/mass_moments.h>
#include <xefis/support/simulation/rigid_body/shape.h>
#include <xefis/support/simulation/rigid_body/shape_material.h>
#include <xefis/support/simulation/rigid_body/various_materials.h>

// Standard:
#include <cstddef>


namespace xf::rigid_body {

// Called by make_sphere to get material for vertices.
using MakeSphereMaterialCallback = std::function<void (ShapeMaterial&, si::Angle latitude)>;


enum RotationDirection
{
	ClockWise,
	CounterClockWise,
};


struct SphereShapeParameters
{
	si::Length					radius;
	std::size_t					slices;
	std::size_t					stacks;
	Range<si::Angle>			h_range			{ 0_deg, 360_deg };
	Range<si::Angle>			v_range			{ -90_deg, +90_deg };
	ShapeMaterial const&		material		{ };
	MakeSphereMaterialCallback	setup_material	{ nullptr };
};


struct CylinderShapeParameters
{
	si::Length				length;
	si::Length				radius;
	Range<si::Angle>		range				{ 0_deg, 360_deg };
	std::size_t				num_faces			{ 10 };
	bool					with_bottom			{ false };
	bool					with_top			{ false };
	ShapeMaterial const&	material			{ };
};


struct ConeShapeParameters
{
	si::Length				length;
	si::Length				radius;
	Range<si::Angle>		range				{ 0_deg, 360_deg };
	size_t					num_faces			{ 10 };
	bool					with_bottom			{ false };
	ShapeMaterial const&	material			{ };
};


struct TruncatedConeShapeParameters
{
	si::Length				length;
	si::Length				bottom_radius;
	si::Length				top_radius;
	Range<si::Angle>		range				{ 0_deg, 360_deg };
	size_t					num_faces			{ 10 };
	bool					with_bottom			{ false };
	bool					with_top			{ false };
	ShapeMaterial const&	material			{ };
};


struct AirfoilShapeParameters
{
	AirfoilSpline const&	spline;
	si::Length				chord_length;
	si::Length				wing_length;
	bool					with_bottom			{ true };
	bool					with_top			{ true };
	ShapeMaterial const&	material			{ };
};


struct PropellerShapeParameters
{
	uint8_t					blades				{ };
	RotationDirection		rotation_direction	{ ClockWise };
	si::Length				diameter;
	si::Length				pitch;
	ShapeMaterial const&	material			{ };
	uint32_t				points_per_blade	{ 20 };
};


struct PropellerConeShapeParameters
{
	std::size_t				num_faces			{ 10 };
	si::Length				radius;
	si::Length				base_length;
	si::Length				cone_length;
	ShapeMaterial const&	material			{ };
};


struct MotorShapeParameters
{
	si::Length				back_radius;
	si::Length				back_cone_length;
	si::Length				center_radius;
	si::Length				center_length;
	si::Length				front_radius;
	si::Length				front_cone_length;
	si::Length				shaft_radius;
	si::Length				shaft_length;
	std::size_t				num_faces			{ 16 };
	ShapeMaterial const&	cones_material		{ };
	ShapeMaterial const&	center_material		{ };
	ShapeMaterial const&	shaft_material		{ };
	ShapeMaterial const&	sticker_material	{ };
};


/**
 * Make cube centered around the [0, 0, 0] point.
 */
Shape
make_centered_cube_shape (si::Length edge_length, ShapeMaterial const& material = {});

/**
 * Make cube centered around the [0, 0, 0] point.
 */
Shape
make_centered_cube_shape (SpaceLength<BodyOrigin> const& dimensions, ShapeMaterial const& material = {});

/**
 * Make cube that represents given moments of inertia. Assumes that off-diagonal
 * elements of the inertia matrix are 0.
 * The cube is centered around the [0, 0, 0] point.
 */
Shape
make_centered_cube_shape (xf::MassMoments<BodyCOM> const&, ShapeMaterial const& material = {});

/**
 * Make cube that represents given moments of inertia.
 */
Shape
make_cube_shape (xf::MassMomentsAtArm<BodyCOM> const&, ShapeMaterial const& material = {});

/**
 * Make sphere of given radius.
 */
Shape
make_centered_sphere_shape (SphereShapeParameters const&);

/**
 * Make a rod shape without bottom/top faces, placed along the Z axis.
 * The beginning of the rod is at the [0, 0, 0] position.
 */
Shape
make_cylinder_shape (CylinderShapeParameters const&);

/**
 * Make a cone shape placed along the Z axis with pointy part pointing towards positive Z values.
 */
Shape
make_cone_shape (ConeShapeParameters const&);

/**
 * Make a truncated cone shape placed along the Z axis with back at X, Y = 0 and front towards positive Z.
 */
Shape
make_truncated_cone_shape (TruncatedConeShapeParameters const&);

/**
 * Make a solid circle placed on X-Y plane.
 */
Shape
make_solid_circle (si::Length radius, Range<si::Angle> range, std::size_t num_slices, ShapeMaterial const& = {});

/**
 * Make a wing shape. Extrude airfoil spline (defined in X-Y axes) along +Z axis.
 */
Shape
make_airfoil_shape (AirfoilShapeParameters const&);

/**
 * Make a "typical" propeller shape. The front of the propeller (where it creates a force) is towards the positive Z axis.
 *
 * \param	pitch
 *			Propeller pitch. Positive for CW propellers, negative for CCW ones.
 */
Shape
make_propeller_shape (PropellerShapeParameters const&);

/**
 * Make a cone for a propeller. Positive Z towards the front of the propeller.
 */
Shape
make_propeller_cone_shape (PropellerConeShapeParameters const&);

/**
 * Make a motor shape with two cones and stuff.
 */
Shape
make_motor_shape (MotorShapeParameters const&);

/**
 * Make a center-of-mass symbol.
 */
Shape
make_center_of_mass_symbol_shape (si::Length radius, ShapeMaterial const& a = kBlackMatte, ShapeMaterial const& b = kWhiteMatte);


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
 * Set planar normals for triangles, that is make each vertex' normal
 * perpendicular to the surface of the triangle.
 */
void
set_planar_normal (Shape::Triangle& triangle);

void
set_planar_normal (std::span<ShapeVertex> triangle);

/**
 * Negate normals for all given vertices.
 */
void
negate_normals (std::vector<ShapeVertex>& vertices);

/**
 * Negate all normals in given shape.
 */
void
negate_normals (Shape& shape);

/**
 * Set given material for all given vertices.
 */
void
set_material (std::vector<ShapeVertex>& vertices, ShapeMaterial const& material);

} // namespace xf::rigid_body

#endif


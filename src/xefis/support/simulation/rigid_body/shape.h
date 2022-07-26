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

#ifndef XEFIS__SUPPORT__SIMULATION__RIGID_BODY__SHAPE_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__RIGID_BODY__SHAPE_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/geometry.h>
#include <xefis/support/simulation/rigid_body/concepts.h>
#include <xefis/support/simulation/rigid_body/shape_vertex.h>

// Standard:
#include <cstddef>
#include <functional>
#include <vector>


namespace xf::rigid_body {

/**
 * Defines a 3D shape for a rigid body.
 */
class Shape
{
  public:
	using Triangle		= std::vector<ShapeVertex>;
	using TriangleStrip	= std::vector<ShapeVertex>;
	using TriangleFan	= std::vector<ShapeVertex>;

  public:
	/**
	 * Return vector of triangles.
	 */
	std::vector<Triangle>&
	triangles() noexcept
		{ return _triangles; }

	/**
	 * Return vector of triangles.
	 */
	std::vector<Triangle> const&
	triangles() const noexcept
		{ return _triangles; }

	/**
	 * Return vector of triangle strips.
	 * Each 3 adjacent points define a triangle like in OpenGL triangle strips.
	 */
	std::vector<TriangleStrip>&
	triangle_strips() noexcept
		{ return _triangle_strips; }

	/**
	 * Return vector of triangle strips.
	 */
	std::vector<TriangleStrip> const&
	triangle_strips() const noexcept
		{ return _triangle_strips; }

	/**
	 * Return vector of triangle fans.
	 * First point is common to all triangles, and each adjacent 2 points and the firs point define a triangle
	 * like in OpenGL triangle fans.
	 */
	std::vector<TriangleFan>&
	triangle_fans() noexcept
		{ return _triangle_fans; }

	/**
	 * Return vector of triangle fans.
	 */
	std::vector<TriangleFan> const&
	triangle_fans() const noexcept
		{ return _triangle_fans; }

	/**
	 * Rotate the shape about space origin by provided rotation matrix.
	 */
	void
	rotate (RotationMatrix<BodySpace> const&);

	/**
	 * Translate the shape by given vector.
	 */
	void
	translate (SpaceLength<BodySpace> const&);

	/**
	 * Apply given function for all vertices.
	 */
	void
	for_all_vertices (std::function<void (ShapeVertex&)>);

  private:
	std::vector<Triangle>		_triangles;
	std::vector<TriangleStrip>	_triangle_strips;
	std::vector<TriangleFan>	_triangle_fans;
};


/*
 * Global functions
 */


inline Shape&
operator+= (Shape& a, Shape const& b)
{
	a.triangles().insert (a.triangles().end(), b.triangles().begin(), b.triangles().end());
	a.triangle_strips().insert (a.triangle_strips().end(), b.triangle_strips().begin(), b.triangle_strips().end());
	a.triangle_fans().insert (a.triangle_fans().end(), b.triangle_fans().begin(), b.triangle_fans().end());
	return a;
}


inline Shape
operator+ (Shape a, Shape const& b)
{
	return a += b;
}

} // namespace xf::rigid_body

#endif


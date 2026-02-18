/* vim:ts=4
 *
 * Copyleft 2018  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__SHAPES__SHAPE_H__INCLUDED
#define XEFIS__SUPPORT__SHAPES__SHAPE_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/geometry.h>
#include <xefis/support/shapes/shape_vertex.h>
#include <xefis/support/simulation/rigid_body/concepts.h>

// Standard:
#include <cstddef>
#include <concepts>
#include <functional>
#include <vector>


namespace xf {

/**
 * Defines a 3D shape for a rigid body.
 */
class Shape
{
  public:
	struct Geometry
	{
		std::vector<ShapeVertex>		vertices;
		std::shared_ptr<QOpenGLTexture>	texture		{ nullptr };
	};

	using Triangle		= Geometry;
	using TriangleStrip	= Geometry;
	using TriangleFan	= Geometry;
	using Quad			= Geometry;

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
	 * Return vector of quads.
	 */
	std::vector<Quad>&
	quads() noexcept
		{ return _quads; }

	/**
	 * Return vector of quads.
	 */
	std::vector<Quad> const&
	quads() const noexcept
		{ return _quads; }

	/**
	 * Transform each point by given matrix.
	 */
	void
	transform (AffineTransform<BodyOrigin> const&);

	/**
	 * Rotate the shape about space origin by provided rotation quaternion.
	 */
	void
	rotate (RotationQuaternion<BodyOrigin> const&);

	/**
	 * Translate the shape by given vector.
	 */
	void
	translate (SpaceLength<BodyOrigin> const&);

	/**
	 * Apply given function for all vertices.
	 */
	void
	for_each_vertex (std::invocable<ShapeVertex&> auto&&);

	/**
	 * Apply given function for all vertices.
	 */
	void
	for_each_vertex (std::invocable<ShapeVertex const&> auto&&) const;

	/**
	 * Apply given function for all triangles (including triangles in strips, fans and quads).
	 */
	void
	for_each_triangle (std::invocable<ShapeVertex&, ShapeVertex&, ShapeVertex&> auto&&);

	/**
	 * Apply given function for all triangles (including triangles in strips, fans and quads).
	 */
	void
	for_each_triangle (std::invocable<ShapeVertex const&, ShapeVertex const&, ShapeVertex const&> auto&&) const;

  private:
	std::vector<Triangle>		_triangles;
	std::vector<TriangleStrip>	_triangle_strips;
	std::vector<TriangleFan>	_triangle_fans;
	std::vector<Quad>			_quads;
};


inline void
Shape::for_each_vertex (std::invocable<ShapeVertex&> auto&& vertex_function)
{
	for (auto* geometries: { &_triangles, &_triangle_strips, &_triangle_fans, &_quads })
		for (auto& geometry: *geometries)
			for (auto& vertex: geometry.vertices)
				vertex_function (vertex);
}


inline void
Shape::for_each_vertex (std::invocable<ShapeVertex const&> auto&& vertex_function) const
{
	const_cast<Shape&> (*this).for_each_vertex ([&vertex_function] (ShapeVertex const& vertex) {
		vertex_function (vertex);
	});
}


inline void
Shape::for_each_triangle (std::invocable<ShapeVertex&, ShapeVertex&, ShapeVertex&> auto&& triangle_function)
{
	for (auto& triangle_geometry: _triangles)
	{
		auto const& vertices = triangle_geometry.vertices;

		for (std::size_t i = 0; i + 2 < vertices.size(); i += 3)
			triangle_function (vertices[i], vertices[i + 1], vertices[i + 2]);
	}

	for (auto& strip_geometry: _triangle_strips)
	{
		auto const& vertices = strip_geometry.vertices;

		// Triangle strips reuse two vertices per step; swap the first two vertices
		// on odd steps to keep a consistent winding for all emitted triangles.
		for (std::size_t i = 0; i + 2 < vertices.size(); ++i)
		{
			if (i % 2 == 0)
				triangle_function (vertices[i], vertices[i + 1], vertices[i + 2]);
			else
				triangle_function (vertices[i + 1], vertices[i], vertices[i + 2]);
		}
	}

	for (auto& fan_geometry: _triangle_fans)
	{
		auto const& vertices = fan_geometry.vertices;

		for (std::size_t i = 1; i + 1 < vertices.size(); ++i)
			triangle_function (vertices[0], vertices[i], vertices[i + 1]);
	}

	for (auto& quad_geometry: _quads)
	{
		auto const& vertices = quad_geometry.vertices;

		for (std::size_t i = 0; i + 3 < vertices.size(); i += 4)
		{
			triangle_function (vertices[i], vertices[i + 1], vertices[i + 2]);
			triangle_function (vertices[i], vertices[i + 2], vertices[i + 3]);
		}
	}
}


inline void
Shape::for_each_triangle (std::invocable<ShapeVertex const&, ShapeVertex const&, ShapeVertex const&> auto&& triangle_function) const
{
	const_cast<Shape&> (*this).for_each_triangle ([&triangle_function] (ShapeVertex const& a, ShapeVertex const& b, ShapeVertex const& c) {
		triangle_function (a, b, c);
	});
}


/*
 * Global functions
 */


inline Shape&
operator+= (Shape& a, Shape const& b)
{
	a.triangles().append_range (b.triangles());
	a.triangle_strips().append_range (b.triangle_strips());
	a.triangle_fans().append_range (b.triangle_fans());
	a.quads().append_range (b.quads());
	return a;
}


inline Shape
operator+ (Shape a, Shape const& b)
{
	return a += b;
}

} // namespace xf

#endif


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

#ifndef XEFIS__SUPPORT__MATH__TRIANGLE_H__INCLUDED
#define XEFIS__SUPPORT__MATH__TRIANGLE_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>


namespace xf {

template<class Scalar, std::size_t N, class Space>
	inline math::Vector<Scalar, N, Space>
	triangle_centroid (math::Vector<Scalar, N, Space> const& a,
					   math::Vector<Scalar, N, Space> const& b,
					   math::Vector<Scalar, N, Space> const& c)
	{
		return 1.0 / 3 * (a + b + c);
	}


template<class Scalar, std::size_t N, class Space>
	inline math::Vector<Scalar, N, Space>
	triangle_centroid (Triangle<Scalar, N, Space> const& triangle)
	{
		return triangle_centroid (triangle[0], triangle[1], triangle[2]);
	}


/**
 * Return normal vector for given triangle (front face is defined by CCW vertex order).
 */
template<math::Scalar S, math::CoordinateSystem Space>
	constexpr auto
	triangle_surface_normal (SpaceVector<S, Space> const& a,
							 SpaceVector<S, Space> const& b,
							 SpaceVector<S, Space> const& c)
	{
		return ((1 / (S (1) * S (1))) * cross_product (b - a, c - a)).normalized();
	}


/**
 * Return normal vector for given triangle (front face is defined by CCW vertex order).
 */
template<TriangleConcept Triangle>
	constexpr auto
	triangle_surface_normal (Triangle const& triangle)
	{
		if (std::size (triangle) != 3)
			throw nu::InvalidArgument ("triangle_surface_normal(): std::size (triangle) must be 3");

		return triangle_surface_normal (triangle[0], triangle[1], triangle[2]);
	}


/**
 * Return area of the 2D triangle.
 */
template<PointConcept Point>
	constexpr auto
	area_2d (Point const& a, Point const& b, Point const& c)
	{
		using std::abs;

		return 0.5 * abs (a[0] * (b[1] - c[1]) + b[0] * (c[1] - a[1]) + c[0] * (a[1] - b[1]));
	}


/**
 * Return area of the 2D triangle.
 */
constexpr auto
area_2d (TriangleConcept auto const& triangle)
{
	using std::abs;
	using std::size;

	if (size (triangle) != 3)
		throw nu::InvalidArgument ("area(): std::size (triangle) must be 3");

	return area_2d (triangle[0], triangle[1], triangle[2]);
}


/**
 * Return a predicate that returns true if its argument (point) is inside of the triangle.
 */
constexpr auto
is_point_2d_inside_triangle_tester (TriangleConcept auto const& triangle)
{
	using Point = decltype (triangle[0]);
	using Scalar = decltype (std::declval<Point>()[0]);

	constexpr size_t x = 0;
	constexpr size_t y = 1;

	auto const p0 = triangle[0];
	auto const p1 = triangle[1];
	auto const p2 = triangle[2];

	auto const y12 = p1[y] - p2[y];
	auto const x21 = p2[x] - p1[x];
	auto const y20 = p2[y] - p0[y];
	auto const x02 = p0[x] - p2[x];

	// If det == 0, triangle is collinear:
	auto const det = y12 * x02 - x21 * y20;
	auto const min_d = std::min (det, Scalar (0));
	auto const max_d = std::max (det, Scalar (0));

	return [=] (Point const& p) -> bool
	{
		auto const dx = p[x] - p2[x];
		auto const dy = p[y] - p2[y];

		auto a = y12 * dx + x21 * dy;

		if (a <= min_d || a >= max_d)
			return false;

		auto b = y20 * dx + x02 * dy;

		if (b <= min_d || b >= max_d)
			return false;

		auto c = det - a - b;

		if (c <= min_d || c >= max_d)
			return false;

		return true;
	};
}

} // namespace xf

#endif


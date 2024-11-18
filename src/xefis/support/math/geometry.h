/* vim:ts=4
 *
 * Copyleft 2024  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__MATH__GEOMETRY_H__INCLUDED
#define XEFIS__SUPPORT__MATH__GEOMETRY_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/concepts.h>
#include <xefis/support/math/coordinate_systems.h>
#include <xefis/support/math/lonlat_radius.h>
#include <xefis/support/math/geometry_types.h>
#include <xefis/support/math/rotations.h>

// Neutrino:
#include <neutrino/math/math.h>
#include <neutrino/numeric.h>

// Standard:
#include <cstddef>
#include <cmath>


namespace xf {

/**
 * Return tangential velocity for given angular velocity and arm.
 */
template<math::CoordinateSystem Space>
	[[nodiscard]]
	inline SpaceVector<si::Velocity, Space>
	tangential_velocity (SpaceVector<si::AngularVelocity, Space> const& w, SpaceLength<Space> const& r)
	{
		return cross_product (w, r) / 1_rad;
	}


/**
 * Make a skew-symmetric matrix (pseudotensor) W from vector v⃗, so that it acts as it was v⃗× operator:
 * v⃗ × Z = W * Z.
 */
template<math::Scalar S, math::CoordinateSystem TargetSpace = void, math::CoordinateSystem SourceSpace = TargetSpace>
	[[nodiscard]]
	constexpr SpaceMatrix<S, TargetSpace, SourceSpace>
	make_pseudotensor (SpaceVector<S, TargetSpace> const& v)
	{
		return {
			S (0), -v[2], +v[1],
			+v[2], S (0), -v[0],
			-v[1], +v[0], S (0),
		};
	}


/**
 * Lay given vector as diagonal of the newly created matrix.
 */
template<math::Scalar S, math::CoordinateSystem TargetSpace = void, math::CoordinateSystem SourceSpace = TargetSpace>
	[[nodiscard]]
	constexpr SpaceMatrix<S, TargetSpace, SourceSpace>
	make_diagonal_matrix (SpaceVector<S, TargetSpace> const& v)
	{
		return {
			v[0],    0,    0,
			   0, v[1],    0,
			   0,    0, v[2],
		};
	}


/**
 * Normalize vectors in matrix.
 * Use for orientation matrices.
 */
template<math::Scalar S, std::size_t C, std::size_t R, math::CoordinateSystem TargetSpace, math::CoordinateSystem SourceSpace>
	constexpr void
	normalize_vectors (math::Matrix<S, C, R, TargetSpace, SourceSpace>& matrix)
	{
		for (std::size_t c = 0; c < C; ++c)
		{
			auto const v_norm = abs (matrix.column (c));

			for (std::size_t r = 0; r < R; ++r)
				matrix[c, r] /= si::quantity (v_norm);
		}
	}


/**
 * Normalize vectors in matrix.
 * Use for orientation matrices.
 */
template<math::Scalar S, std::size_t C, std::size_t R, math::CoordinateSystem TargetSpace, math::CoordinateSystem SourceSpace>
	[[nodiscard]]
	constexpr math::Matrix<S, C, R, TargetSpace, SourceSpace>
	vector_normalized (math::Matrix<S, C, R, TargetSpace, SourceSpace> matrix)
	{
		normalize_vectors (matrix);
		return matrix;
	}


/**
 * Return vector orthogonalized onto another vector.
 */
template<math::Scalar S, math::CoordinateSystem Space>
	[[nodiscard]]
	constexpr SpaceVector<S, Space>
	orthogonalized (SpaceVector<S, Space> const& vector, SpaceVector<S, Space> const& onto)
	{
		return vector - (dot_product (vector, onto) * onto / square (abs (onto)));
	}


/**
 * Make matrix orthogonal so that X stays unchanged.
 */
template<math::Scalar S, math::CoordinateSystem TargetSpace, math::CoordinateSystem SourceSpace>
	[[nodiscard]]
	constexpr SpaceMatrix<S, TargetSpace, SourceSpace>
	orthogonalized (SpaceMatrix<S, TargetSpace, SourceSpace> const& m)
	{
		auto const new_y = orthogonalized (m.column (1), m.column (0));
		auto const new_z = cross_product (m.column (0), new_y);

		return { m.column (0), new_y, new_z };
	}


/**
 * Ensure that the length of a vector does not exceed a specified maximum,
 * adjusting the vector's magnitude if necessary while preserving its direction.
 */
template<math::Scalar S, math::CoordinateSystem Space>
	[[nodiscard]]
	constexpr SpaceVector<S, Space>
	length_limited (SpaceVector<S, Space> vector, S const& max_length)
	{
		auto const length = abs (vector);

		if (length > max_length)
			vector = vector * max_length / length;

		return vector;
	}


/**
 * Project vector "vector" onto "onto" vector.
 */
template<math::Scalar S1, math::Scalar S2, math::CoordinateSystem Space>
	[[nodiscard]]
	constexpr auto
	projection (SpaceVector<S1, Space> const& vector, SpaceVector<S2, Space> const& onto)
	{
		return dot_product (vector, onto.normalized()) * onto;
	}


/**
 * This version takes normalized "onto" vector, if caller has one, to save on computing time.
 */
template<math::Scalar S1, math::Scalar S2, math::CoordinateSystem Space>
	[[nodiscard]]
	constexpr auto
	projection_onto_normalized (SpaceVector<S1, Space> const& vector, SpaceVector<S2, Space> const& normalized_onto)
	{
		return dot_product (vector, normalized_onto) * normalized_onto;
	}


/**
 * Find a vector that is non-colinear with given input vector.
 */
template<math::Scalar S, math::CoordinateSystem Space>
	[[nodiscard]]
	constexpr SpaceVector<S, Space>
	find_non_colinear (SpaceVector<S, Space> input)
	{
		input.normalize();
		auto output = kXRotationPlus90<Space, Space> * input;

		if (abs (cross_product (input, output)) > 0)
			return output;
		else
			return kYRotationPlus90<Space, Space> * input;
	}


/**
 * Find any non-normalized perpendicular vector to given vector.
 */
template<math::Scalar S, math::CoordinateSystem Space>
	[[nodiscard]]
	constexpr SpaceVector<S, Space>
	find_any_perpendicular (SpaceVector<S, Space> const& input)
	{
		return cross_product (input, find_non_colinear (input));
	}


/**
 * Create orthonormal basis matrix from given vector Z.
 * Two orthonormal vectors to Z will be chosen randomly.
 */
template<math::Scalar S, math::CoordinateSystem TargetSpace, math::CoordinateSystem SourceSpace = TargetSpace>
	[[nodiscard]]
	constexpr RotationMatrix<TargetSpace, SourceSpace>
	make_basis_from_z (SpaceVector<S, TargetSpace> const& z)
	{
		auto const x = find_any_perpendicular (z).normalized();
		auto const y = cross_product (z, x).normalized();

		return RotationMatrix<TargetSpace, SourceSpace> {
			x[0], y[0], z[0],
			x[1], y[1], z[1],
			x[2], y[2], z[2],
		};
	}


/**
 * Return normal vector for given triangle (front face is defined by CCW vertex order).
 */
template<math::Scalar S, math::CoordinateSystem Space>
	inline auto
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
	inline auto
	triangle_surface_normal (Triangle const& triangle)
	{
		if (std::size (triangle) != 3)
			throw InvalidArgument ("triangle_surface_normal(): std::size (triangle) must be 3");

		return triangle_surface_normal (triangle[0], triangle[1], triangle[2]);
	}


/**
 * Return area of the 2D triangle.
 */
template<PointConcept Point>
	inline auto
	area_2d (Point const& a, Point const& b, Point const& c)
	{
		using std::abs;

		return 0.5 * abs (a[0] * (b[1] - c[1]) + b[0] * (c[1] - a[1]) + c[0] * (a[1] - b[1]));
	}


/**
 * Return area of the 2D triangle.
 */
inline auto
area_2d (TriangleConcept auto const& triangle)
{
	using std::abs;
	using std::size;

	if (size (triangle) != 3)
		throw InvalidArgument ("area(): std::size (triangle) must be 3");

	return area_2d (triangle[0], triangle[1], triangle[2]);
}


/**
 * Return a predicate that returns true if its argument (point) is inside of the triangle.
 */
inline auto
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


/*
 * Polar-cartesian conversions
 */


[[nodiscard]]
inline SpaceVector<si::Length, ECEFSpace>
cartesian (LonLatRadius const& position)
{
	auto const r = si::Length (position.radius()).value();
	auto const wz = std::polar (r, position.lat().in<si::Radian>());
	auto const xy = std::polar (wz.real(), position.lon().in<si::Radian>());

	return {
		si::Length (xy.real()),
		si::Length (xy.imag()),
		si::Length (wz.imag()),
	};
}


[[nodiscard]]
inline LonLatRadius
polar (SpaceVector<si::Length, ECEFSpace> const& vector)
{
	std::complex<double> const xy (vector[0].value(), vector[1].value());
	std::complex<double> const wz (std::abs (xy), vector[2].value());

	return LonLatRadius {
		si::LonLat (1_rad * std::arg (xy), 1_rad * std::arg (wz)),
		si::Length (std::abs (wz))
	};
}

} // namespace xf

#endif


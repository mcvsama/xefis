/* vim:ts=4
 *
 * Copyleft 2012…2018  Michał Gawron
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
#include <xefis/support/math/lonlat_radius.h>

// Neutrino:
#include <neutrino/math/math.h>
#include <neutrino/numeric.h>

// Standard:
#include <cstddef>
#include <cmath>


namespace xf {

// Earth-centered Earth-fixed frame of reference:
struct ECEFSpace;

// Local-tangent-plane frame of reference:
struct NEDSpace;

// Simulated body frame of reference (X points to the front, Y to the right, Z down the body):
struct AirframeSpace;


template<class Scalar = double, class Space = void>
	using PlaneVector = math::Vector<Scalar, 2, Space, void>;

template<class Scalar = double, class Space = void>
	using SpaceVector = math::Vector<Scalar, 3, Space, void>;

template<class Scalar = double, class TargetSpace = void, class SourceSpace = TargetSpace>
	using PlaneMatrix = math::Matrix<Scalar, 2, 2, TargetSpace, SourceSpace>;

template<class Scalar = double, class TargetSpace = void, class SourceSpace = TargetSpace>
	using SpaceMatrix = math::Matrix<Scalar, 3, 3, TargetSpace, SourceSpace>;

template<class TargetSpace = void, class SourceSpace = TargetSpace>
	using RotationMatrix = SpaceMatrix<double, TargetSpace, SourceSpace>;

template<class TargetSpace = void, class SourceSpace = TargetSpace>
	RotationMatrix<TargetSpace, SourceSpace> const kNoRotation = math::unit;

template<class Scalar = double, class Space = void>
	using PlaneTriangle = std::array<PlaneVector<Scalar, Space>, 3>;

template<class Scalar = double, class Space = void>
	using SpaceTriangle = std::array<SpaceVector<Scalar, Space>, 3>;

// Typical units used in space:

template<class Space = void>
	using SpaceLength = SpaceVector<si::Length, Space>;

template<class Space = void>
	using SpaceForce = SpaceVector<si::Force, Space>;

template<class Space = void>
	using SpaceTorque = SpaceVector<si::Torque, Space>;


/**
 * Return rotation matrix along the axis X for given angle.
 */
template<class TF = void, class SF = TF>
	[[nodiscard]]
	constexpr RotationMatrix<TF, SF>
	x_rotation (si::Angle const angle)
	{
		double const sin_a = sin (angle);
		double const cos_a = cos (angle);

		return {
			1.0,    0.0,    0.0,
			0.0, +cos_a, -sin_a,
			0.0, +sin_a, +cos_a,
		};
	}


/**
 * Return rotation matrix along the axis Y for given angle.
 */
template<class TF = void, class SF = TF>
	[[nodiscard]]
	constexpr RotationMatrix<TF, SF>
	y_rotation (si::Angle const angle)
	{
		double const sin_a = sin (angle);
		double const cos_a = cos (angle);

		return {
			+cos_a, 0.0, +sin_a,
			   0.0, 1.0,    0.0,
			-sin_a, 0.0, +cos_a,
		};
	}


/**
 * Return rotation matrix along the axis Z for given angle.
 */
template<class TF = void, class SF = TF>
	[[nodiscard]]
	constexpr RotationMatrix<TF, SF>
	z_rotation (si::Angle const angle)
	{
		double const sin_a = sin (angle);
		double const cos_a = cos (angle);

		return {
			+cos_a, -sin_a, 0.0,
			+sin_a, +cos_a, 0.0,
			   0.0,    0.0, 1.0,
		};
	}


template<class TF, class SF>
	static auto const kXRotationPlus45	= x_rotation<TF, SF> (45_deg);

template<class TF, class SF>
	static auto const kYRotationPlus45	= y_rotation<TF, SF> (45_deg);

template<class TF, class SF>
	static auto const kZRotationPlus45	= z_rotation<TF, SF> (45_deg);

template<class TF, class SF>
	static auto const kXRotationPlus90	= x_rotation<TF, SF> (90_deg);

template<class TF, class SF>
	static auto const kYRotationPlus90	= y_rotation<TF, SF> (90_deg);

template<class TF, class SF>
	static auto const kZRotationPlus90	= z_rotation<TF, SF> (90_deg);

template<class TF, class SF>
	static auto const kXRotationPlus180	= x_rotation<TF, SF> (180_deg);

template<class TF, class SF>
	static auto const kYRotationPlus180	= y_rotation<TF, SF> (180_deg);

template<class TF, class SF>
	static auto const kZRotationPlus180	= z_rotation<TF, SF> (180_deg);


// Forward:
template<class S, std::size_t C, std::size_t R, class TF, class SF>
	[[nodiscard]]
	constexpr math::Matrix<S, C, R, TF, SF>
	vector_normalized (math::Matrix<S, C, R, TF, SF> matrix);


/**
 * Return tangential velocity for given angular velocity and arm.
 */
template<class Space>
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
template<class S, class TF = void, class SF = TF>
	[[nodiscard]]
	constexpr SpaceMatrix<S, TF, SF>
	make_pseudotensor (SpaceVector<S, TF> const& v)
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
template<class S, class TF = void, class SF = TF>
	[[nodiscard]]
	constexpr SpaceMatrix<S, TF, SF>
	make_diagonal_matrix (SpaceVector<S, TF> const& v)
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
template<class S, std::size_t C, std::size_t R, class TF, class SF>
	constexpr void
	normalize_vectors (math::Matrix<S, C, R, TF, SF>& matrix)
	{
		for (std::size_t c = 0; c < C; ++c)
		{
			auto const v_norm = abs (matrix.column (c));

			for (std::size_t r = 0; r < R; ++r)
				matrix (c, r) /= si::quantity (v_norm);
		}
	}


/**
 * Normalize vectors in matrix.
 * Use for orientation matrices.
 */
template<class S, std::size_t C, std::size_t R, class TF, class SF>
	[[nodiscard]]
	constexpr math::Matrix<S, C, R, TF, SF>
	vector_normalized (math::Matrix<S, C, R, TF, SF> matrix)
	{
		normalize_vectors (matrix);
		return matrix;
	}


/**
 * Return vector orthogonalized onto another vector.
 */
template<class S, class F>
	[[nodiscard]]
	constexpr SpaceVector<S, F>
	orthogonalized (SpaceVector<S, F> const& vector, SpaceVector<S, F> const& onto)
	{
		return vector - ((~vector * onto).scalar() * onto / square (abs (onto)));
	}


/**
 * Make matrix orthogonal so that X stays unchanged.
 */
template<class S, class TF, class SF>
	[[nodiscard]]
	constexpr SpaceMatrix<S, TF, SF>
	orthogonalized (SpaceMatrix<S, TF, SF> const& m)
	{
		auto const new_y = orthogonalized (m.column (1), m.column (0));
		auto const new_z = cross_product (m.column (0), new_y);

		return { m.column (0), new_y, new_z };
	}


template<class T, class F>
	[[nodiscard]]
	constexpr SpaceVector<T, F>
	length_limited (SpaceVector<T, F> vector, T const& max_length)
	{
		auto const length = abs (vector);

		if (length > max_length)
			vector = vector * max_length / length;

		return vector;
	}


template<class T, class F>
	[[nodiscard]]
	constexpr auto
	normalized (SpaceVector<T, F> const& vector)
	{
		return T (1) * vector / abs (vector);
	}


/**
 * Project vector "vector" onto "onto" vector.
 */
template<class T1, class T2, class Space>
	[[nodiscard]]
	constexpr auto
	projection (SpaceVector<T1, Space> const& vector, SpaceVector<T2, Space> const& onto)
	{
		return (~vector * normalized (onto)).scalar() * onto;
	}


/**
 * This version takes normalized "onto" vector, if caller has one, to save on computing time.
 */
template<class T1, class T2, class Space>
	[[nodiscard]]
	constexpr auto
	projection_onto_normalized (SpaceVector<T1, Space> const& vector, SpaceVector<T2, Space> const& normalized_onto)
	{
		return (~vector * normalized_onto).scalar() * normalized_onto;
	}


/**
 * Find a vector that is non-colinear with given input vector.
 */
template<class Scalar, class Space>
	[[nodiscard]]
	constexpr SpaceVector<Scalar, Space>
	find_non_colinear (SpaceVector<Scalar, Space> input)
	{
		input = normalized (input);

		auto output = kXRotationPlus90<Space, Space> * input;

		if (abs (cross_product (input, output)) > 0)
			return output;
		else
			return kYRotationPlus90<Space, Space> * input;
	}


/**
 * Find any non-normalized perpendicular vector to given vector.
 */
template<class T, class F>
	[[nodiscard]]
	constexpr SpaceVector<T, F>
	find_any_perpendicular (SpaceVector<T, F> const& input)
	{
		return cross_product (input, find_non_colinear (input));
	}


/**
 * Create orthonormal basis matrix from given vector Z.
 * Two orthonormal vectors to Z will be chosen randomly.
 */
template<class Scalar, class TF, class SF = TF>
	[[nodiscard]]
	constexpr RotationMatrix<TF, SF>
	make_basis_from_z (SpaceVector<Scalar, TF> const& z)
	{
		auto const x = normalized (find_any_perpendicular (z));
		auto const y = normalized (cross_product (z, x));

		return RotationMatrix<TF, SF> {
			x[0], y[0], z[0],
			x[1], y[1], z[1],
			x[2], y[2], z[2],
		};
	}


/**
 * Return rotation matrix about the given axis vector for given angle.
 */
template<class TF = void, class SF = TF>
	[[nodiscard]]
	constexpr RotationMatrix<TF, SF>
	rotation_about (SpaceVector<double, TF> const& axis, si::Angle const angle)
	{
		auto const sin_a = sin (angle);
		auto const cos_a = cos (angle);
		auto const k = 1.0 - cos_a;
		auto const x = axis[0];
		auto const y = axis[1];
		auto const z = axis[2];
		auto const x_sin_a = x * sin_a;
		auto const y_sin_a = y * sin_a;
		auto const z_sin_a = z * sin_a;
		auto const x_y_k = x * y * k;
		auto const x_z_k = x * z * k;
		auto const y_z_k = y * z * k;

		return {
			x * x * k + cos_a, x_y_k - z_sin_a,   x_z_k + y_sin_a,
			x_y_k + z_sin_a,   y * y * k + cos_a, y_z_k - x_sin_a,
			x_z_k - y_sin_a,   y_z_k + x_sin_a,   z * z * k + cos_a,
		};
	}


/**
 * Determine the non-normalized rotation axis from the matrix.
 * FIXME has problems with 0° (nans) and 180° (also nans)
 */
template<class TF = void, class SF = TF>
	[[nodiscard]]
	inline SpaceVector<double, TF>
	rotation_axis (RotationMatrix<TF, SF> const& m)
	{
		SpaceVector<double, TF> result {
			m (1, 2) - m (2, 1),
			m (2, 0) - m (0, 2),
			m (0, 1) - m (1, 0),
		};

		// FIXME What a hack, better use quaternions:
		if (abs (result) == 0.0)
			return { 1, 0, 0 };
		else
			return result;
	}


/**
 * Determine the rotation angle about any axis from the matrix.
 */
template<class TF = void, class SF = TF>
	inline si::Angle
	rotation_angle_about_matrix_axis (RotationMatrix<TF, SF> const& m, SpaceVector<double, TF> normalized_axis)
	{
		SpaceVector<double> const x = math::reframe<void, void> (normalized (find_any_perpendicular (normalized_axis)));
		SpaceVector<double> const y = math::reframe<void, void> (m) * x;

		auto const sin_theta = abs (cross_product (x, y));
		auto const cos_theta = (~x * y).scalar();

		return 1_rad * atan2 (sin_theta, cos_theta);
	}


/**
 * Determine the rotation angle about the rotaion axis of the matrix.
 */
template<class TF = void, class SF = TF>
	inline si::Angle
	rotation_angle (RotationMatrix<TF, SF> const& m)
	{
		auto const axis = normalized (rotation_axis (m));
		return rotation_angle_about_matrix_axis (m, axis);
	}


/**
 * Return rotation matrix for given vector-expressed rotation (right-hand rule,
 * length of vector corresponds to angle). Length of rotation_vector should be
 * expressed in radians.
 * FIXME has numerical instabilities at small rotations
 */
template<class TF = void, class SF = TF>
	[[nodiscard]]
	constexpr RotationMatrix<TF, SF>
	to_rotation_matrix (SpaceVector<si::Angle, TF> const& rotation_vector)
	{
		if (abs (rotation_vector) > 0.0_rad)
			return rotation_about (vector_normalized (rotation_vector) / 1_rad, abs (rotation_vector));
		else
			return math::unit;
	}


/**
 * Return rotation vector from rotation matrix.
 */
template<class TF = void, class SF = TF>
	[[nodiscard]]
	constexpr SpaceVector<si::Angle, TF>
	to_rotation_vector (RotationMatrix<TF, SF> const& matrix)
	{
		auto const axis = normalized (rotation_axis (matrix));
		return rotation_angle_about_matrix_axis (matrix, axis) * axis;
	}


/**
 * Return normal vector for given triangle (front face is defined by CCW vertex order).
 */
template<class Triangle>
	inline auto
	triangle_surface_normal (Triangle const& triangle)
	{
		if (std::size (triangle) != 3)
			throw InvalidArgument ("triangle_surface_normal(): std::size (triangle) must be 3");

		auto const scalar = decltype (triangle[0].position()[0]) { 1 };

		return normalized (cross_product (triangle[1].position() - triangle[0].position(),
										  triangle[2].position() - triangle[0].position()) / scalar / scalar);
	}


/**
 * Return area of the 2D triangle.
 */
template<class Point>
	inline auto
	area_2d (Point const& a, Point const& b, Point const& c)
	{
		using std::abs;
		using std::sqrt;

		auto const len_ab = abs (b - a);
		auto const len_ac = abs (c - a);
		auto const len_bc = abs (c - b);
		auto const s = 0.5 * (len_ab + len_ac + len_bc);

		return sqrt (s * (s - len_ab) * (s - len_ac) * (s - len_bc));
	}


/**
 * Return area of the 2D triangle.
 */
template<class Triangle>
	inline auto
	area_2d (Triangle const& triangle)
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
template<class Triangle>
	inline auto
	is_point_2d_inside_triangle_tester (Triangle const& triangle)
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


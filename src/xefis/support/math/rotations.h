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

#ifndef XEFIS__SUPPORT__MATH__ROTATIONS_H__INCLUDED
#define XEFIS__SUPPORT__MATH__ROTATIONS_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/geometry_types.h>

// Neutrino:
#include <neutrino/math/concepts.h>
#include <neutrino/math/math.h>
#include <neutrino/numeric.h>

// Standard:
#include <cstddef>
#include <cmath>


namespace xf {

// Forward:
template<math::Scalar S, math::CoordinateSystem Space>
	[[nodiscard]]
	constexpr auto
	normalized (SpaceVector<S, Space> const& vector);

// Forward:
template<math::Scalar S, std::size_t C, std::size_t R, math::CoordinateSystem TargetSpace, math::CoordinateSystem SourceSpace>
	[[nodiscard]]
	constexpr math::Matrix<S, C, R, TargetSpace, SourceSpace>
	vector_normalized (math::Matrix<S, C, R, TargetSpace, SourceSpace> matrix);

// Forward
template<math::Scalar S, math::CoordinateSystem Space>
	[[nodiscard]]
	constexpr SpaceVector<S, Space>
	find_any_perpendicular (SpaceVector<S, Space> const& input);


/**
 * Return rotation matrix along the axis X for given angle.
 */
template<math::CoordinateSystem TargetSpace = void, math::CoordinateSystem SourceSpace = TargetSpace>
	[[nodiscard]]
	constexpr RotationMatrix<TargetSpace, SourceSpace>
	x_rotation_matrix (si::Angle const angle)
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
template<math::CoordinateSystem TargetSpace = void, math::CoordinateSystem SourceSpace = TargetSpace>
	[[nodiscard]]
	constexpr RotationMatrix<TargetSpace, SourceSpace>
	y_rotation_matrix (si::Angle const angle)
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
template<math::CoordinateSystem TargetSpace = void, math::CoordinateSystem SourceSpace = TargetSpace>
	[[nodiscard]]
	constexpr RotationMatrix<TargetSpace, SourceSpace>
	z_rotation_matrix (si::Angle const angle)
	{
		double const sin_a = sin (angle);
		double const cos_a = cos (angle);

		return {
			+cos_a, -sin_a, 0.0,
			+sin_a, +cos_a, 0.0,
			   0.0,    0.0, 1.0,
		};
	}


template<math::CoordinateSystem TargetSpace = void, math::CoordinateSystem SourceSpace = TargetSpace>
	[[nodiscard]]
	constexpr RotationMatrix<TargetSpace, SourceSpace>
	x_rotation (si::Angle const angle)
		{ return x_rotation_matrix<TargetSpace, SourceSpace> (angle); }


template<math::CoordinateSystem TargetSpace = void, math::CoordinateSystem SourceSpace = TargetSpace>
	[[nodiscard]]
	constexpr RotationMatrix<TargetSpace, SourceSpace>
	y_rotation (si::Angle const angle)
		{ return y_rotation_matrix<TargetSpace, SourceSpace> (angle); }


template<math::CoordinateSystem TargetSpace = void, math::CoordinateSystem SourceSpace = TargetSpace>
	[[nodiscard]]
	constexpr RotationMatrix<TargetSpace, SourceSpace>
	z_rotation (si::Angle const angle)
		{ return z_rotation_matrix<TargetSpace, SourceSpace> (angle); }


template<math::CoordinateSystem TargetSpace, math::CoordinateSystem SourceSpace>
	static auto const kXRotationPlus45	= x_rotation<TargetSpace, SourceSpace> (45_deg);

template<math::CoordinateSystem TargetSpace, math::CoordinateSystem SourceSpace>
	static auto const kYRotationPlus45	= y_rotation<TargetSpace, SourceSpace> (45_deg);

template<math::CoordinateSystem TargetSpace, math::CoordinateSystem SourceSpace>
	static auto const kZRotationPlus45	= z_rotation<TargetSpace, SourceSpace> (45_deg);

template<math::CoordinateSystem TargetSpace, math::CoordinateSystem SourceSpace>
	static auto const kXRotationPlus90	= x_rotation<TargetSpace, SourceSpace> (90_deg);

template<math::CoordinateSystem TargetSpace, math::CoordinateSystem SourceSpace>
	static auto const kYRotationPlus90	= y_rotation<TargetSpace, SourceSpace> (90_deg);

template<math::CoordinateSystem TargetSpace, math::CoordinateSystem SourceSpace>
	static auto const kZRotationPlus90	= z_rotation<TargetSpace, SourceSpace> (90_deg);

template<math::CoordinateSystem TargetSpace, math::CoordinateSystem SourceSpace>
	static auto const kXRotationPlus180	= x_rotation<TargetSpace, SourceSpace> (180_deg);

template<math::CoordinateSystem TargetSpace, math::CoordinateSystem SourceSpace>
	static auto const kYRotationPlus180	= y_rotation<TargetSpace, SourceSpace> (180_deg);

template<math::CoordinateSystem TargetSpace, math::CoordinateSystem SourceSpace>
	static auto const kZRotationPlus180	= z_rotation<TargetSpace, SourceSpace> (180_deg);


/**
 * Angle difference between two rotation quaternions.
 */
template<math::CoordinateSystem TargetSpace, math::CoordinateSystem SourceSpace>
	[[nodiscard]]
	inline si::Angle
	angle_difference (RotationQuaternion<TargetSpace, SourceSpace> const& a,
					  RotationQuaternion<TargetSpace, SourceSpace> const& b)
	{
		return 2_rad * std::acos (std::min (std::abs (dot_product (a, b)), 1.0));
	}


/**
 * Angle difference between two rotation matrices.
 * Uses formula Tr(M) = 1 + 2cos(theta). Returns theta.
 */
template<math::CoordinateSystem TargetSpace1, math::CoordinateSystem TargetSpace2, math::CoordinateSystem SourceSpace1, math::CoordinateSystem SourceSpace2>
	[[nodiscard]]
	inline si::Angle
	angle_difference (RotationMatrix<TargetSpace1, SourceSpace1> const& a,
					  RotationMatrix<TargetSpace2, SourceSpace2> const& b)
	{
		return 1_rad * std::acos ((trace (dot_product (a, b)) - 1) / 2.0);
	}


/**
 * Return alpha and beta angles required to transform versor x to given vector.
 * Alpha is the X-Y plane angle, beta is X-Z plane angle.
 */
template<math::Scalar S, math::CoordinateSystem Space>
	[[nodiscard]]
	std::array<si::Angle, 2>
	alpha_beta_from_x_to (SpaceVector<S, Space> const& vector)
	{
		using std::atan2;

		auto const alpha = 1_rad * atan2 (vector[1], vector[0]);
		auto const r = S (1) * std::sqrt (square (vector[0] / S (1)) + square (vector[1] / S (1)));
		auto const beta = 1_rad * -atan2 (vector[2], r);

		return { alpha, beta };
	}


/**
 * Return rotation matrix about the given axis vector for given angle.
 */
template<math::CoordinateSystem TargetSpace = void, math::CoordinateSystem SourceSpace = TargetSpace>
	[[nodiscard]]
	constexpr RotationMatrix<TargetSpace, SourceSpace>
	rotation_about (SpaceVector<double, TargetSpace> const& axis, si::Angle const angle)
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
template<math::CoordinateSystem TargetSpace = void, math::CoordinateSystem SourceSpace = TargetSpace>
	[[nodiscard]]
	inline SpaceVector<double, TargetSpace>
	rotation_axis (RotationMatrix<TargetSpace, SourceSpace> const& m)
	{
		SpaceVector<double, TargetSpace> result {
			m[1, 2] - m[2, 1],
			m[2, 0] - m[0, 2],
			m[0, 1] - m[1, 0],
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
template<math::CoordinateSystem TargetSpace = void, math::CoordinateSystem SourceSpace = TargetSpace>
	inline si::Angle
	rotation_angle_about_matrix_axis (RotationMatrix<TargetSpace, SourceSpace> const& m, SpaceVector<double, TargetSpace> normalized_axis)
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
template<math::CoordinateSystem TargetSpace = void, math::CoordinateSystem SourceSpace = TargetSpace>
	inline si::Angle
	rotation_angle (RotationMatrix<TargetSpace, SourceSpace> const& m)
	{
		auto const axis = normalized (rotation_axis (m));
		return rotation_angle_about_matrix_axis (m, axis);
	}


template<math::CoordinateSystem Space>
	inline si::Angle
	angle_between (SpaceVector<auto, Space> const& a,
				   SpaceVector<auto, Space> const& b)
	{
		return 1_rad * std::acos (dot_product (a, b) / (abs (a) * abs (b)));
	}


template<math::CoordinateSystem Space>
	inline auto
	cos_angle_between (SpaceVector<auto, Space> const& a,
					   SpaceVector<auto, Space> const& b)
	{
		return dot_product (a, b) / (abs (a) * abs (b));
	}


/**
 * Return rotation matrix for given vector-expressed rotation (right-hand rule,
 * length of vector corresponds to angle). Length of rotation_vector should be
 * expressed in radians.
 * FIXME has numerical instabilities at small rotations
 */
template<math::CoordinateSystem TargetSpace = void, math::CoordinateSystem SourceSpace = TargetSpace>
	[[nodiscard]]
	constexpr RotationMatrix<TargetSpace, SourceSpace>
	to_rotation_matrix (SpaceVector<si::Angle, TargetSpace> const& rotation_vector)
	{
		if (abs (rotation_vector) > 0.0_rad)
			return rotation_about (vector_normalized (rotation_vector) / 1_rad, abs (rotation_vector));
		else
			return math::unit;
	}


/**
 * Return rotation vector from rotation matrix.
 */
template<math::CoordinateSystem TargetSpace = void, math::CoordinateSystem SourceSpace = TargetSpace>
	[[nodiscard]]
	constexpr SpaceVector<si::Angle, TargetSpace>
	to_rotation_vector (RotationMatrix<TargetSpace, SourceSpace> const& matrix)
	{
		auto const axis = normalized (rotation_axis (matrix));
		return rotation_angle_about_matrix_axis (matrix, axis) * axis;
	}

} // namespace xf

#endif


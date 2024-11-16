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

#ifndef XEFIS__SUPPORT__MATH__MATRIX_ROTATIONS_H__INCLUDED
#define XEFIS__SUPPORT__MATH__MATRIX_ROTATIONS_H__INCLUDED

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
	constexpr SpaceVector<S, Space>
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
 * Return rotation matrix about the given axis vector for given angle.
 */
template<math::CoordinateSystem TargetSpace = void, math::CoordinateSystem SourceSpace = TargetSpace>
	[[nodiscard]]
	constexpr RotationMatrix<TargetSpace, SourceSpace>
	matrix_rotation_about (SpaceVector<double, TargetSpace> const& axis, si::Angle const angle)
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


/**
 * Determine the non-normalized rotation axis from the matrix.
 * Result is not normalized.
 * Has problems near 0° (nans) and 180° (also nans).
 */
template<math::CoordinateSystem TargetSpace = void, math::CoordinateSystem SourceSpace = TargetSpace>
	[[nodiscard]]
	inline SpaceVector<double, TargetSpace>
	unnormalized_axis (RotationMatrix<TargetSpace, SourceSpace> const& rotation)
	{
		SpaceVector<double, TargetSpace> result {
			rotation[1, 2] - rotation[2, 1],
			rotation[2, 0] - rotation[0, 2],
			rotation[0, 1] - rotation[1, 0],
		};

		// What a hack, better use quaternions:
		if (abs (result) == 0.0)
			return { 1, 0, 0 };
		else
			return result;
	}


/**
 * Determine the non-normalized rotation axis from the matrix.
 * Result is normalized.
 * Has problems near 0° (nans) and 180° (also nans).
 */
template<math::CoordinateSystem TargetSpace = void, math::CoordinateSystem SourceSpace = TargetSpace>
	[[nodiscard]]
	inline SpaceVector<double, TargetSpace>
	normalized_axis (RotationMatrix<TargetSpace, SourceSpace> const& rotation)
	{
		return normalized (unnormalized_axis (rotation));
	}


/**
 * Determine the rotation angle about any axis from the matrix.
 */
template<math::CoordinateSystem TargetSpace = void, math::CoordinateSystem SourceSpace = TargetSpace>
	inline si::Angle
	angle_about_matrix_axis (RotationMatrix<TargetSpace, SourceSpace> const& rotation, SpaceVector<double, TargetSpace> normalized_axis)
	{
		SpaceVector<double> const x = math::reframe<void, void> (normalized (find_any_perpendicular (normalized_axis)));
		SpaceVector<double> const y = math::reframe<void, void> (rotation) * x;

		auto const sin_theta = abs (cross_product (x, y));
		auto const cos_theta = dot_product (x, y);

		return 1_rad * atan2 (sin_theta, cos_theta);
	}


/**
 * Determine the rotation angle about the rotation axis of the matrix.
 */
template<math::CoordinateSystem TargetSpace = void, math::CoordinateSystem SourceSpace = TargetSpace>
	inline si::Angle
	angle (RotationMatrix<TargetSpace, SourceSpace> const& rotation)
	{
		return angle_about_matrix_axis (rotation, normalized_axis (rotation));
	}


/**
 * Return rotation vector from rotation matrix.
 */
template<math::CoordinateSystem TargetSpace = void, math::CoordinateSystem SourceSpace = TargetSpace>
	[[nodiscard]]
	constexpr SpaceVector<si::Angle, TargetSpace>
	to_rotation_vector (RotationMatrix<TargetSpace, SourceSpace> const& matrix)
	{
		auto const axis = normalized_axis (matrix);
		return angle_about_matrix_axis (matrix, axis) * axis;
	}


/**
 * Return rotation matrix for given vector-expressed rotation (right-hand rule,
 * length of vector corresponds to angle). Length of rotation_vector should be
 * expressed in radians.
 * Warning: it has numerical instabilities at small rotations, better to use to_rotation_quaternion().
 */
template<math::CoordinateSystem TargetSpace = void, math::CoordinateSystem SourceSpace = TargetSpace>
	[[nodiscard]]
	constexpr RotationMatrix<TargetSpace, SourceSpace>
	to_rotation_matrix (SpaceVector<si::Angle, TargetSpace> const& rotation_vector)
	{
		if (abs (rotation_vector) > 0.0_rad)
			return matrix_rotation_about (vector_normalized (rotation_vector) / 1_rad, abs (rotation_vector));
		else
			return math::unit;
	}


template<class Target, class Source>
	[[nodiscard]]
	inline auto
	relative_rotation (RotationMatrix<Target, Source> const& from, RotationMatrix<Target, Source> const& to)
	{
		// Divide the "from" rotation matrix by the "to" rotation matrix (mutiply by inversion):
		return from * ~to;
	}

} // namespace xf

#endif


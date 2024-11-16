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

#ifndef XEFIS__SUPPORT__MATH__QUATERNION_ROTATIONS_H__INCLUDED
#define XEFIS__SUPPORT__MATH__QUATERNION_ROTATIONS_H__INCLUDED

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
#include <numbers>


namespace xf {

// Forward:
template<math::Scalar S, math::CoordinateSystem Space>
	[[nodiscard]]
	constexpr SpaceVector<S, Space>
	normalized (SpaceVector<S, Space> const& vector);


/**
 * Return rotation quaternion about the given axis vector for given angle.
 */
template<math::CoordinateSystem TargetSpace = void, math::CoordinateSystem SourceSpace = TargetSpace>
	[[nodiscard]]
	constexpr RotationQuaternion<TargetSpace, SourceSpace>
	quaternion_rotation_about (SpaceVector<double, TargetSpace> const& normalized_axis, si::Angle const angle)
	{
		auto const half_angle = 0.5 * angle;
		return { cos (half_angle), sin (half_angle) * normalized_axis };
	}


/**
 * Return rotation quaternion along the axis X for given angle.
 */
template<math::CoordinateSystem TargetSpace = void, math::CoordinateSystem SourceSpace = TargetSpace>
	[[nodiscard]]
	constexpr RotationQuaternion<TargetSpace, SourceSpace>
	x_rotation_quaternion (si::Angle const angle)
	{
		return quaternion_rotation_about<TargetSpace, SourceSpace> ({ 1.0, 0.0, 0.0 }, angle);
	}


/**
 * Return rotation quaternion along the axis Y for given angle.
 */
template<math::CoordinateSystem TargetSpace = void, math::CoordinateSystem SourceSpace = TargetSpace>
	[[nodiscard]]
	constexpr RotationQuaternion<TargetSpace, SourceSpace>
	y_rotation_quaternion (si::Angle const angle)
	{
		return quaternion_rotation_about<TargetSpace, SourceSpace> ({ 0.0, 1.0, 0.0 }, angle);
	}


/**
 * Return rotation quaternion along the axis Z for given angle.
 */
template<math::CoordinateSystem TargetSpace = void, math::CoordinateSystem SourceSpace = TargetSpace>
	[[nodiscard]]
	constexpr RotationQuaternion<TargetSpace, SourceSpace>
	z_rotation_quaternion (si::Angle const angle)
	{
		return quaternion_rotation_about<TargetSpace, SourceSpace> ({ 0.0, 0.0, 1.0 }, angle);
	}


/**
 * Determine the rotation angle about the rotation axis of the quaternion.
 * The resulting angle is always positive.
 */
template<math::CoordinateSystem TargetSpace = void, math::CoordinateSystem SourceSpace = TargetSpace>
	inline si::Angle
	angle (RotationQuaternion<TargetSpace, SourceSpace> const& rotation)
	{
		auto a = 2_rad * acos (std::clamp (rotation.w(), -1.0, +1.0));

		// Angle should always be positive, but in some cases axis will be inverse:
		if (rotation.w() < 0)
			a = 2.0_rad * std::numbers::pi - a;

		return a;
	}


/**
 * Determine the rotation axis form the rotation quaternion.
 * Result is not normalized.
 */
template<math::CoordinateSystem TargetSpace = void, math::CoordinateSystem SourceSpace = TargetSpace>
	[[nodiscard]]
	inline SpaceVector<double, TargetSpace>
	unnormalized_axis (RotationQuaternion<TargetSpace, SourceSpace> const& rotation)
	{
		auto axis = rotation.imag();

		if (axis[0] == 0.0 && axis[1] == 0.0 && axis[2] == 0.0)
			axis = { 1.0, 0.0, 0.0 };

		// Since angle() is always positive, we need to invert axis for cases -π…2π:
		return 1.0 * sgn (rotation.w()) * axis;
	}


/**
 * Determine the rotation axis form the rotation quaternion.
 * Result is normalized.
 */
template<math::CoordinateSystem TargetSpace = void, math::CoordinateSystem SourceSpace = TargetSpace>
	[[nodiscard]]
	inline SpaceVector<double, TargetSpace>
	normalized_axis (RotationQuaternion<TargetSpace, SourceSpace> const& rotation)
	{
		return normalized (unnormalized_axis (rotation));
	}


/**
 * Return rotation vector from rotation quaternion.
 */
template<math::CoordinateSystem TargetSpace = void, math::CoordinateSystem SourceSpace = TargetSpace>
	[[nodiscard]]
	constexpr SpaceVector<si::Angle, TargetSpace>
	to_rotation_vector (RotationQuaternion<TargetSpace, SourceSpace> const& quaternion)
	{
		return angle (quaternion) * normalized_axis (quaternion);
	}


/**
 * Return rotation quaternion for given vector-expressed rotation (right-hand rule,
 * length of vector corresponds to angle). Length of rotation_vector should be
 * expressed in radians.
 */
template<math::CoordinateSystem TargetSpace = void, math::CoordinateSystem SourceSpace = TargetSpace>
	[[nodiscard]]
	constexpr RotationQuaternion<TargetSpace, SourceSpace>
	to_rotation_quaternion (SpaceVector<si::Angle, TargetSpace> const& rotation_vector)
	{
		auto const angle = abs (rotation_vector);
		auto const half_angle = 0.5 * angle;
		auto const axis = normalized (rotation_vector).transformed ([](si::Angle const value) { return value.in<si::Radian>(); });

		if (angle != 0_rad)
			return { cos (half_angle), sin (half_angle) * axis };
		else
			return { 1.0, 0.0, 0.0, 0.0 };
	}


template<class Target, class Source>
	[[nodiscard]]
	inline auto
	relative_rotation (RotationQuaternion<Target, Source> const& from, RotationQuaternion<Target, Source> const& to)
	{
		// Divide the "from" rotation quaternion by the "to" rotation quaternion (mutiply by inversion):
		return from * ~to;
	}

} // namespace xf

#endif


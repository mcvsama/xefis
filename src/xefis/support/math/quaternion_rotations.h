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
 * Determine the rotation axis form the rotation quaternion.
 */
template<math::CoordinateSystem TargetSpace = void, math::CoordinateSystem SourceSpace = TargetSpace>
	[[nodiscard]]
	inline SpaceVector<double, TargetSpace>
	rotation_axis (RotationQuaternion<TargetSpace, SourceSpace> const& rotation)
	{
		return normalized (rotation.imag());
	}


/**
 * Determine the rotation angle about the rotation axis of the quaternion.
 */
template<math::CoordinateSystem TargetSpace = void, math::CoordinateSystem SourceSpace = TargetSpace>
	inline si::Angle
	rotation_angle (RotationQuaternion<TargetSpace, SourceSpace> const& rotation)
	{
		return 2_rad * acos (std::clamp (rotation.w(), -1.0, +1.0));
	}


/**
 * Return rotation vector from rotation quaternion.
 */
template<math::CoordinateSystem TargetSpace = void, math::CoordinateSystem SourceSpace = TargetSpace>
	[[nodiscard]]
	constexpr SpaceVector<si::Angle, TargetSpace>
	to_rotation_vector (RotationQuaternion<TargetSpace, SourceSpace> const& quaternion)
	{
		return rotation_angle (quaternion) * rotation_axis (quaternion);
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
			return { cos (0.5 * angle), axis * sin (half_angle) };
		else
			return { 1.0, 0.0, 0.0, 0.0 };
	}

} // namespace xf

#endif


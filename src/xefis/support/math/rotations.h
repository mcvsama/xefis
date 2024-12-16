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
#include <xefis/support/math/matrix_rotations.h>
#include <xefis/support/math/quaternion_rotations.h>

// Neutrino:
#include <neutrino/math/concepts.h>
#include <neutrino/math/math.h>
#include <neutrino/numeric.h>

// Standard:
#include <cstddef>
#include <cmath>


namespace xf {

template<math::CoordinateSystem TargetSpace = void, math::CoordinateSystem SourceSpace = TargetSpace>
	[[nodiscard]]
	constexpr RotationQuaternion<TargetSpace, SourceSpace>
	rotation_about (SpaceVector<double, TargetSpace> const& axis, si::Angle const angle)
	{
		return quaternion_rotation_about (axis, angle);
	}


template<math::CoordinateSystem TargetSpace = void, math::CoordinateSystem SourceSpace = TargetSpace>
	[[nodiscard]]
	constexpr RotationQuaternion<TargetSpace, SourceSpace>
	x_rotation (si::Angle const angle)
	{
		return x_rotation_quaternion<TargetSpace, SourceSpace> (angle);
	}


template<math::CoordinateSystem TargetSpace = void, math::CoordinateSystem SourceSpace = TargetSpace>
	[[nodiscard]]
	constexpr RotationQuaternion<TargetSpace, SourceSpace>
	y_rotation (si::Angle const angle)
	{
		return y_rotation_quaternion<TargetSpace, SourceSpace> (angle);
	}


template<math::CoordinateSystem TargetSpace = void, math::CoordinateSystem SourceSpace = TargetSpace>
	[[nodiscard]]
	constexpr RotationQuaternion<TargetSpace, SourceSpace>
	z_rotation (si::Angle const angle)
	{
		return z_rotation_quaternion<TargetSpace, SourceSpace> (angle);
	}


template<math::CoordinateSystem TargetSpace = void, math::CoordinateSystem SourceSpace = TargetSpace>
	static auto const kNoRotation		= RotationQuaternion<TargetSpace, SourceSpace> (math::identity);

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


template<math::Scalar S, math::CoordinateSystem Space>
	inline si::Angle
	angle_between (SpaceVector<S, Space> const& a,
				   SpaceVector<S, Space> const& b)
	{
		auto const arg = dot_product (a, b) / (abs (a) * abs (b));
		return 1_rad * std::acos (std::clamp (arg, -1.0, +1.0));
	}


template<math::Scalar S, math::CoordinateSystem Space>
	inline auto
	cos_angle_between (SpaceVector<S, Space> const& a,
					   SpaceVector<S, Space> const& b)
	{
		return dot_product (a, b) / (abs (a) * abs (b));
	}

} // namespace xf

#endif


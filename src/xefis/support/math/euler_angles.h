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

#ifndef XEFIS__SUPPORT__MATH__EULER_ANGLES_H__INCLUDED
#define XEFIS__SUPPORT__MATH__EULER_ANGLES_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/geometry.h>

// Standard:
#include <cstddef>
#include <cmath>
#include <numbers>


namespace xf {

struct EulerAngles: public SpaceVector<si::Angle>
{
	using SpaceVector<si::Angle>::SpaceVector;
	using SpaceVector<si::Angle>::operator=;

	// Ctor
	explicit
	EulerAngles (SpaceVector<si::Angle> const& other):
		SpaceVector<si::Angle> (other)
	{ }

	constexpr auto
	gamma() const noexcept
		{ return (*this)[0]; }

	constexpr auto
	alpha() const noexcept
		{ return (*this)[1]; }

	constexpr auto
	beta() const noexcept
		{ return (*this)[2]; }
};


template<math::CoordinateSystem SourceSpace1, math::CoordinateSystem SourceSpace2>
	[[nodiscard]]
	inline EulerAngles
	euler_angles (RotationQuaternion<SourceSpace1, SourceSpace2> const& quaternion)
	{
		using std::atan2;
		using std::sqrt;
		using std::numbers::pi;

		auto const [qw, qx, qy, qz] = quaternion.components();

		// <https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles#Quaternion_to_Euler_angles_(in_3-2-1_sequence)_conversion>
		auto const roll = 1_rad * (-0.5 * pi + 2 * (atan2 (sqrt (1 + 2 * (qw * qy - qx * qz)), sqrt (1 - 2 * (qw * qy - qx * qz)))));
		auto const pitch = 1_rad * atan2 (2 * (qw * qx + qy * qz), 1 - 2 * (square (qx) + square (qy)));
		auto const heading = 1_rad * atan2 (2 * (qw * qz + qx * qy), 1 - 2 * (square (qy) + square (qz)));

		return { roll, pitch, heading };
	}


/**
 * Return a set of Euler angles as difference in rotation between two bases.
 * Order of vector columns in resulting matrix: roll, pitch, yaw.
 */
template<math::CoordinateSystem TargetSpace1, math::CoordinateSystem TargetSpace2, math::CoordinateSystem SourceSpace1, math::CoordinateSystem SourceSpace2>
	[[nodiscard]]
	inline EulerAngles
	euler_angle_difference (RotationQuaternion<TargetSpace1, SourceSpace1> const& base_a,
							RotationQuaternion<TargetSpace2, SourceSpace2> const& base_b)
	{
		return euler_angles (base_b / base_a);
	}

} // namespace xf

#endif


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

// Neutrino:
#include <neutrino/math/math.h>

// Standard:
#include <cstddef>
#include <cmath>


namespace xf {

/**
 * Angle difference between two rotation quaternions.
 */
template<class TargetSpace, class SourceSpace>
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
template<class TargetSpace1, class TargetSpace2, class SourceSpace1, class SourceSpace2>
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
template<class T, class F>
	[[nodiscard]]
	std::array<si::Angle, 2>
	alpha_beta_from_x_to (SpaceVector<T, F> const& vector)
	{
		using std::atan2;

		auto const alpha = 1_rad * atan2 (vector[1], vector[0]);
		auto const r = T (1) * std::sqrt (square (vector[0] / T (1)) + square (vector[1] / T (1)));
		auto const beta = 1_rad * -atan2 (vector[2], r);

		return { alpha, beta };
	}

} // namespace xf

#endif


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

#ifndef XEFIS__SUPPORT__MATH__NORTH_EAST_DOWN_H__INCLUDED
#define XEFIS__SUPPORT__MATH__NORTH_EAST_DOWN_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/geometry.h>
#include <xefis/support/math/placement.h>


namespace xf {

/*
 * NED (North-East-Down) aka Local-Tangent-Plane functions
 */


[[nodiscard]]
constexpr auto
north_vector (RotationMatrix<NEDSpace, ECEFSpace> const& ned_matrix)
{
	return ned_matrix.column (0);
}


[[nodiscard]]
constexpr auto
east_vector (RotationMatrix<NEDSpace, ECEFSpace> const& ned_matrix)
{
	return ned_matrix.column (1);
}


[[nodiscard]]
constexpr auto
down_vector (RotationMatrix<NEDSpace, ECEFSpace> const& ned_matrix)
{
	return ned_matrix.column (2);
}


static inline RotationMatrix<NEDSpace, ECEFSpace> const EquatorPrimeMeridian {
//  N   E   D
	0,  0, -1,  // x
	0,  1,  0,  // y
	1,  0,  0,  // z
};

} // namespace xf

#endif


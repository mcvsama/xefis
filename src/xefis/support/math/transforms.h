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

#ifndef XEFIS__SUPPORT__MATH__TRANSFORMS_H__INCLUDED
#define XEFIS__SUPPORT__MATH__TRANSFORMS_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/geometry.h>
#include <xefis/support/math/placement.h>
#include <xefis/support/nature/constants.h>

// Standard:
#include <cstddef>


namespace xf {

class TaitBryanAngles;


[[nodiscard]]
RotationMatrix<ECEFSpace, AirframeSpace>
airframe_to_ecef_rotation (TaitBryanAngles const& angles, si::LonLat const& position);


[[nodiscard]]
inline RotationMatrix<ECEFSpace, AirframeSpace>
airframe_to_ecef_rotation (TaitBryanAngles const& angles, SpaceVector<si::Length, ECEFSpace> const& position)
{
	return airframe_to_ecef_rotation (angles, polar (position));
}


[[nodiscard]]
RotationMatrix<NEDSpace, ECEFSpace>
ecef_to_ned_rotation (si::LonLat const& position);


[[nodiscard]]
inline RotationMatrix<NEDSpace, ECEFSpace>
ecef_to_ned_rotation (SpaceVector<si::Length, ECEFSpace> const& position)
{
	return ecef_to_ned_rotation (polar (position));
}


[[nodiscard]]
inline RotationMatrix<NEDSpace, AirframeSpace>
airframe_to_ned_rotation (Placement<ECEFSpace, AirframeSpace> const& pr)
{
	return ecef_to_ned_rotation (pr.position()) * pr.body_to_base_rotation();
}

} // namespace xf

#endif


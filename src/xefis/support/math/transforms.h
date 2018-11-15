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

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/space.h>
#include <xefis/support/math/position_rotation.h>


namespace xf {

class TaitBryanAngles;


[[nodiscard]]
RotationMatrix<ECEFFrame, AirframeFrame>
airframe_to_ecef_rotation (TaitBryanAngles const& angles, si::LonLat const& position);


[[nodiscard]]
inline RotationMatrix<ECEFFrame, AirframeFrame>
airframe_to_ecef_rotation (TaitBryanAngles const& angles, SpaceVector<si::Length, ECEFFrame> const& position)
{
	return airframe_to_ecef_rotation (angles, polar (position));
}


[[nodiscard]]
RotationMatrix<NEDFrame, ECEFFrame>
ecef_to_ned_rotation (si::LonLat const& position);


[[nodiscard]]
inline RotationMatrix<NEDFrame, ECEFFrame>
ecef_to_ned_rotation (SpaceVector<si::Length, ECEFFrame> const& position)
{
	return ecef_to_ned_rotation (polar (position));
}


[[nodiscard]]
inline RotationMatrix<NEDFrame, AirframeFrame>
airframe_to_ned_rotation (PositionRotation<ECEFFrame, AirframeFrame> const& pr)
{
	return ecef_to_ned_rotation (pr.position()) * pr.body_to_base_rotation();
}

} // namespace xf

#endif


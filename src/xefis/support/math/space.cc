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

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/earth/earth.h>

// Local:
#include "space.h"


namespace xf {

SpaceMatrix<double, NEDFrame, ECEFFrame>
ecef_to_ned_transform (si::LonLat const& position)
{
	SpaceVector<double, NEDFrame> const n0 = north_vector (EquatorPrimeMeridian);
	SpaceVector<double, NEDFrame> const e0 = east_vector (EquatorPrimeMeridian);
	SpaceVector<double, NEDFrame> const e1 = rotation_about (n0, +position.lon()) * e0;
	SpaceVector<double, NEDFrame> const n1 = rotation_about (e1, -position.lat()) * n0;
	SpaceVector<double, NEDFrame> const d1 = cross_product (n1, e1);

	return { n1, e1, d1 };
}


SpaceMatrix<double, ECEFFrame, BodyFrame>
body_to_ecef_transform (TaitBryanAngles const& angles, si::LonLat const& position)
{
	SpaceMatrix<double, NEDFrame, ECEFFrame> const ned0 = ecef_to_ned_transform (position);
	SpaceMatrix<double, NEDFrame, ECEFFrame> const ned1 = rotation_about<NEDFrame> (down_vector (ned0), angles.yaw()) * ned0;
	SpaceMatrix<double, NEDFrame, ECEFFrame> const ned2 = rotation_about<NEDFrame> (east_vector (ned1), angles.pitch()) * ned1;
	SpaceMatrix<double, NEDFrame, ECEFFrame> const ned3 = rotation_about<NEDFrame> (north_vector (ned2), angles.roll()) * ned2;

	return SpaceMatrix<double, ECEFFrame, NEDFrame> { math::unit } * ned3 * SpaceMatrix<double, ECEFFrame, BodyFrame> { math::unit };
}

} // namespace xf


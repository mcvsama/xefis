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

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/earth/earth.h>
#include <xefis/support/math/geometry.h>
#include <xefis/support/math/north_east_down.h>
#include <xefis/support/math/tait_bryan_angles.h>

// Standard:
#include <cstddef>


namespace xf {

RotationMatrix<NEDSpace, ECEFSpace>
ecef_to_ned_rotation (si::LonLat const& position)
{
	SpaceVector<double, NEDSpace> const n0 = north_vector (EquatorPrimeMeridian);
	SpaceVector<double, NEDSpace> const e0 = east_vector (EquatorPrimeMeridian);
	SpaceVector<double, NEDSpace> const e1 = rotation_about (n0, +position.lon()) * e0;
	SpaceVector<double, NEDSpace> const n1 = rotation_about (e1, -position.lat()) * n0;
	SpaceVector<double, NEDSpace> const d1 = cross_product (n1, e1);

	return { n1, e1, d1 };
}


RotationMatrix<ECEFSpace, AirframeSpace>
airframe_to_ecef_rotation (TaitBryanAngles const& angles, si::LonLat const& position)
{
	RotationMatrix<NEDSpace, ECEFSpace> const ned0 = ecef_to_ned_rotation (position);
	RotationMatrix<NEDSpace, ECEFSpace> const ned1 = rotation_about<NEDSpace> (down_vector (ned0), angles.yaw()) * ned0;
	RotationMatrix<NEDSpace, ECEFSpace> const ned2 = rotation_about<NEDSpace> (east_vector (ned1), angles.pitch()) * ned1;
	RotationMatrix<NEDSpace, ECEFSpace> const ned3 = rotation_about<NEDSpace> (north_vector (ned2), angles.roll()) * ned2;

	return RotationMatrix<ECEFSpace, NEDSpace> { math::unit } * ned3 * RotationMatrix<ECEFSpace, AirframeSpace> { math::unit };
}

} // namespace xf


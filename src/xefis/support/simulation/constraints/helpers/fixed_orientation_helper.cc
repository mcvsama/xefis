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

// Local:
#include "fixed_orientation_helper.h"

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>


namespace xf::rigid_body {

FixedOrientationHelper::FixedOrientationHelper (Placement<WorldSpace, BodyCOM> const& location_1,
												Placement<WorldSpace, BodyCOM> const& location_2):
	_initial_relative_rotation (relative_rotation (location_1, location_2))
{ }


SpaceLength<WorldSpace>
FixedOrientationHelper::rotation_constraint_value (Placement<WorldSpace, BodyCOM> const& location_1,
												   Placement<WorldSpace, BodyCOM> const& location_2) const
{
	RotationQuaternion<BodyCOM, BodyCOM> const q_current_relative_rotation = relative_rotation (location_1.base_rotation(), location_2.base_rotation());
	RotationQuaternion<BodyCOM, BodyCOM> const q_body_error = ~_initial_relative_rotation * q_current_relative_rotation;
	SpaceVector<si::Angle, WorldSpace> const world_error = location_2.body_rotation() * to_rotation_vector (q_body_error);
	constexpr auto rad_to_meter = 1_m / 1_rad;
	return world_error * rad_to_meter;
}

} // namespace xf::rigid_body


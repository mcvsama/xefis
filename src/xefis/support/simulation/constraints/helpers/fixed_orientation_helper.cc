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
	RotationMatrix<BodyCOM, BodyCOM> const current_relative_rotation = relative_rotation (location_1, location_2);
	RotationMatrix<BodyCOM, BodyCOM> const body_error = ~_initial_relative_rotation * current_relative_rotation;
	SpaceVector<si::Angle, WorldSpace> const world_error = location_2.body_to_base_rotation() * to_rotation_vector (body_error);
	return world_error * 1_m / 1_rad;
}

} // namespace xf::rigid_body


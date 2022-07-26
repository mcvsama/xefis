/* vim:ts=4
 *
 * Copyleft 2019  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__SIMULATION__CONSTRAINTS__HELPERS__FIXED_ORIENTATION_HELPER_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__CONSTRAINTS__HELPERS__FIXED_ORIENTATION_HELPER_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/placement.h>
#include <xefis/support/simulation/rigid_body/concepts.h>

// Standard:
#include <cstddef>


namespace xf::rigid_body {

class FixedOrientationHelper
{
  public:
	/**
	 * Create a fixed orientation between two bodies helper.
	 *
	 * \param	body_1, body_2
	 *			References to connected bodies. Helper must not outlive bodys.
	 */
	explicit
	FixedOrientationHelper (Placement<WorldSpace, BodySpace> const& location_1,
							Placement<WorldSpace, BodySpace> const& location_2);

	/**
	 * Return value to put inside location constraint matrix as the rotation values.
	 */
	[[nodiscard]]
	SpaceLength<WorldSpace>
	rotation_constraint_value (Placement<WorldSpace, BodySpace> const& location_1,
							   Placement<WorldSpace, BodySpace> const& location_2) const;

  private:
	RotationMatrix<BodySpace, BodySpace> _initial_relative_rotation;
};


inline
FixedOrientationHelper::FixedOrientationHelper (Placement<WorldSpace, BodySpace> const& location_1,
												Placement<WorldSpace, BodySpace> const& location_2):
	_initial_relative_rotation (location_1.base_to_body_rotation() * location_2.body_to_base_rotation())
{ }


inline SpaceLength<WorldSpace>
FixedOrientationHelper::rotation_constraint_value (Placement<WorldSpace, BodySpace> const& location_1,
												   Placement<WorldSpace, BodySpace> const& location_2) const
{
	RotationMatrix<BodySpace, BodySpace> const current_relative_rotation = location_1.base_to_body_rotation() * location_2.body_to_base_rotation();
	RotationMatrix<BodySpace, BodySpace> const body_rotation_error = ~_initial_relative_rotation * current_relative_rotation;
	SpaceVector<si::Angle, WorldSpace> const world_rotation_error = location_2.body_to_base_rotation() * to_rotation_vector (body_rotation_error);
	return world_rotation_error * 1_m / 1_rad;
}

} // namespace xf::rigid_body

#endif


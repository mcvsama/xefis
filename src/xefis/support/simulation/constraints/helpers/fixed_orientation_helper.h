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
	FixedOrientationHelper (Placement<WorldSpace, BodyCOM> const& location_1,
							Placement<WorldSpace, BodyCOM> const& location_2);

	/**
	 * Return value to put inside location constraint matrix as the rotation values.
	 */
	[[nodiscard]]
	SpaceLength<WorldSpace>
	rotation_constraint_value (Placement<WorldSpace, BodyCOM> const& location_1,
							   Placement<WorldSpace, BodyCOM> const& location_2) const;

  private:
	RotationQuaternion<BodyCOM, BodyCOM>	_initial_relative_rotation;
};

} // namespace xf::rigid_body

#endif


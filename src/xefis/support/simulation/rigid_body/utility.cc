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

// Local:
#include "utility.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/nature/constants.h>
#include <xefis/support/nature/mass_moments.h>
#include <xefis/support/simulation/rigid_body/various_shapes.h>

// Standard:
#include <cstddef>
#include <memory>


namespace xf::rigid_body {

std::unique_ptr<Body>
make_earth()
{
	auto earth = std::make_unique<Body> (MassMoments<BodyCOM> (kEarthMass, math::coordinate_system_cast<BodyCOM, BodyCOM> (kEarthMomentOfInertia)));
	earth->set_shape (rigid_body::make_centered_sphere_shape ({
		.radius = 0.9 * kEarthMeanRadius,
		.slices = 10,
		.stacks = 10,
	}));
	return earth;
}

} // namespace xf::rigid_body


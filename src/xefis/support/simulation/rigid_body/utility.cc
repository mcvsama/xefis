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

// Standard:
#include <cstddef>
#include <memory>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/nature/constants.h>
#include <xefis/support/nature/mass_moments.h>

// Local:
#include "utility.h"


namespace xf::rigid_body {

std::unique_ptr<Body>
make_earth()
{
	return std::make_unique<Body> (MassMoments<BodySpace> (kEarthMass, math::zero, math::reframe<BodySpace, BodySpace> (kEarthMomentOfInertia)));
}

} // namespace xf::rigid_body


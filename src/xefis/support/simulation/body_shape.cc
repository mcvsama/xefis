/* vim:ts=4
 *
 * Copyleft 2008…2018  Michał Gawron
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

// Local:
#include "body_shape.h"


namespace xf::sim {

void
BodyShape::set_moment_of_inertia (SpaceMatrix<si::MomentOfInertia, BodyFrame> const& moment_of_inertia)
{
	// TODO should be recalculated from parts
	_total_moment_of_inertia = moment_of_inertia;
	_inversed_total_moment_of_inertia = inv (moment_of_inertia);
}

} // namespace xf::sim


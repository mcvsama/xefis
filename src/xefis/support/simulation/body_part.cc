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

BodyPart::BodyPart (SpaceVector<si::Length, BodyFrame> const& position,
					si::Mass mass,
					SpaceMatrix<si::MomentOfInertia, PartFrame> const& moment_of_inertia):
	_position (position),
	_mass (mass)
{
	set_moment_of_inertia (moment_of_inertia);
}


void
BodyPart::set_moment_of_inertia (SpaceMatrix<si::MomentOfInertia, PartFrame> const& moment_of_inertia)
{
	_moment_of_inertia = moment_of_inertia;
	_inversed_moment_of_inertia = inv (moment_of_inertia);
}

} // namespace xf::sim


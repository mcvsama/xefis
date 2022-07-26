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
#include "group.h"

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>


namespace xf::rigid_body {

void
Group::rotate_about_world_origin (RotationMatrix<WorldSpace> const& rotation)
{
	for (auto& body: _bodies)
		body->rotate_about_world_origin (rotation);
}


void
Group::rotate_about (SpaceLength<WorldSpace> const& about_point, RotationMatrix<WorldSpace> const& rotation)
{
	for (auto& body: _bodies)
		body->rotate_about (about_point, rotation);
}


void
Group::translate (SpaceLength<WorldSpace> const& translation)
{
	for (auto& body: _bodies)
		body->translate (translation);
}


si::Energy
Group::kinetic_energy() const
{
	si::Energy e = 0_J;

	for (auto const& body: _bodies)
		e += body->kinetic_energy();

	return e;
}

} // namespace xf::rigid_body


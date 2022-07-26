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
#include "system.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/nature/constants.h>

// Lib:
#include <boost/range/adaptors.hpp>

// Standard:
#include <cstddef>


namespace xf::rigid_body {

si::Energy
System::kinetic_energy() const
{
	si::Energy e = 0_J;

	for (auto const& body: _bodies)
		e += body->kinetic_energy();

	return e;
}


void
System::rotate_about_world_origin (RotationMatrix<WorldSpace> const& rotation)
{
	for (auto& body: _bodies)
		body->rotate_about_world_origin (rotation);
}


void
System::translate (SpaceLength<WorldSpace> const& translation)
{
	for (auto& body: _bodies)
		body->translate (translation);
}


void
System::set_baumgarte_factor (double factor) noexcept
{
	for (auto& c: _constraints)
		c->set_baumgarte_factor (factor);
}

} // namespace xf::rigid_body


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

// Lib:
#include <boost/range/adaptors.hpp>

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "body_shape.h"


namespace xf::sim {

void
BodyShape::recompute()
{
	_total_mass = std::accumulate (_parts.begin(), _parts.end(), 0_kg, [](auto const sum, auto const& part_ptr) {
		return sum + part_ptr->mass();
	});

	auto const range = _parts | boost::adaptors::transformed ([](auto const& part_ptr) { return std::make_tuple (part_ptr->mass(), part_ptr->aircraft_relative_position()); });
	_total_moment_of_inertia = xf::moment_of_inertia<AirframeFrame> (range.begin(), range.end());

	_inversed_total_moment_of_inertia = inv (_total_moment_of_inertia);
}

} // namespace xf::sim


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
#include "node.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/simulation/electrical/element.h>

// Standard:
#include <cstddef>


namespace xf::electrical {

Node::Node (Element& element, Direction direction):
	_name ("<" + element.name() + "." + (direction == Anode ? "A" : "K") + ">"),
	_element (&element),
	_direction (direction)
{ }

} // namespace xf::electrical


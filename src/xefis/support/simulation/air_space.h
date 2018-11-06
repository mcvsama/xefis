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

#ifndef XEFIS__SUPPORT__SIMULATION__AIR_SPACE_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__AIR_SPACE_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>


namespace xf::sim {

/**
 * Simulates a space of the air around the aircraft, ie. vector space of wind that
 * gets disturbed by the airfoils traveling through the air.
 */
class AirSpace
{
  public:
};

} // namespace xf::sim

#endif


/* vim:ts=4
 *
 * Copyleft 2026  Michał Gawron
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
#include "antenna.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/simulation/antennas/antenna_system.h>

// Standard:
#include <cstddef>


namespace xf {

Antenna::~Antenna()
{
	_system.deregister_antenna (*this);
}


void
Antenna::emit_signal (AntennaEmission const& antenna_emission)
{
	_system.emit_signal (*this, antenna_emission);
}

} // namespace xf

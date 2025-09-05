/* vim:ts=4
 *
 * Copyleft 2025  Michał Gawron
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
#include "radio_machine.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/xefis.h>

// Standard:
#include <cstddef>


namespace sim1::aircraft {

using namespace neutrino::si::literals;


RadioMachine::RadioMachine (xf::Xefis& xefis,
							xf::Clock& xefis_clock,
							xf::Clock& steady_clock,
							bool const inside_simulation):
	xf::SingleLoopMachine (xefis, xefis.logger().with_context ("radio").with_context ("machine"), 120_Hz, u8"Aircraft/Radio machine"),
	_xefis_clock (xefis_clock),
	_steady_clock (steady_clock),
	_modules (loop(), xefis.logger().with_context ("radio").with_context ("modules"))
{
	initialize();

	if (!inside_simulation)
		start();
}


void
RadioMachine::connect_modules()
{
	// TODO
}

} // namespace sim1::aircraft

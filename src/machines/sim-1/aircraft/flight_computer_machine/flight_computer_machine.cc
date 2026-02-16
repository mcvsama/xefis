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
#include "flight_computer_machine.h"

// Sim-1:
#include <machines/sim-1/common/common.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/xefis.h>

// Standard:
#include <cstddef>


namespace sim1::aircraft {

FlightComputerMachine::FlightComputerMachine (xf::Xefis& xefis,
											  xf::Clock& xefis_clock,
											  xf::Clock& steady_clock,
											  bool const inside_simulation):
	xf::SingleLoopMachine (xefis, xefis.logger().with_context ("flight computer"), 120_Hz, u8"Aircraft/Flight computer machine"),
	_xefis_clock (xefis_clock),
	_steady_clock (steady_clock),
	_modules (loop(), logger())
{
	initialize();

	if (!inside_simulation)
		start();
}


void
FlightComputerMachine::connect_modules()
{
	// TODO
}

} // namespace sim1::aircraft

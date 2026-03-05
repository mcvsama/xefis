/* vim:ts=4
 *
 * Copyleft 2008…2023  Michał Gawron
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
#include "hardware_machine.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/xefis_machine.h>

// Standard:
#include <cstddef>


namespace sim1::ground_station {

HardwareMachine::HardwareMachine (xf::Xefis& xefis):
	xf::SingleLoopMachine (xefis, xefis.logger(), 120_Hz, u8"Ground station/Hardware machine")
{
	start();
}

} // namespace sim1::ground_station

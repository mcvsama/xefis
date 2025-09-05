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

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>


std::unique_ptr<xf::Machine>
make_xefis_machine (xf::Xefis& xefis)
{
	static auto utc_clock = xf::UTCClock();
	static auto steady_clock = xf::SteadyClock();
	return std::make_unique<sim1::aircraft::FlightComputerMachine> (xefis, utc_clock, steady_clock, false);
}

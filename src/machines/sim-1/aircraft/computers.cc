/* vim:ts=4
 *
 * Copyleft 2012  Michał Gawron
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
#include "computers.h"

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>


namespace sim1::aircraft {

Computers::Computers (xf::ProcessingLoop& loop, nu::Logger const& logger):
	_loop (loop),
	_logger (logger)
{
	this->air_data_computer.ias_valid_minimum = 1_mps;
	this->air_data_computer.ias_valid_maximum = 100_mps;
}

} // namespace sim1::ground_station


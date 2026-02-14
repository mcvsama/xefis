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
#include "hardware_modules.h"

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>


namespace sim1::aircraft {

HardwareModules::HardwareModules (xf::ProcessingLoop& loop, nu::Logger const& logger):
	_logger (logger),
	_loop (loop)
{
	// Connect with the Flight Computer machine:
	this->flight_computer_machine_transceiver.send << this->hardware_to_flight_computer_data_encoder.encoded_output;
	this->flight_computer_to_hardware_data_decoder.encoded_input << this->flight_computer_machine_transceiver.receive;
}

} // namespace sim1::aircraft

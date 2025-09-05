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
#include "flight_computer_modules.h"

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>


namespace sim1::aircraft {

FlightComputerModules::FlightComputerModules (xf::ProcessingLoop& loop, nu::Logger const& logger):
	_logger (logger),
	_loop (loop)
{
	this->air_data_computer.ias_valid_minimum = 1_mps;
	this->air_data_computer.ias_valid_maximum = 100_mps;

	// Connect with the Hardware machine:
	this->hardware_machine_transceiver.send << this->flight_computer_to_hardware_data_encoder.link_output;
	this->hardware_to_flight_computer_data_decoder.link_input << this->hardware_machine_transceiver.receive;

	this->air_data_computer.pressure_use_std		<< false;
	this->air_data_computer.pressure_qnh			<< 1013.25_hPa;
	this->air_data_computer.pressure_static			<< _hardware_to_flight_computer_data.air_static_pressure;
	this->air_data_computer.pressure_total			<< _hardware_to_flight_computer_data.air_total_pressure;
	this->air_data_computer.total_air_temperature	<< _hardware_to_flight_computer_data.air_total_temperature;
}

} // namespace sim1::aircraft

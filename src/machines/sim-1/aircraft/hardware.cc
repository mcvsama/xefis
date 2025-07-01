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
#include "hardware.h"

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>


namespace sim1::aircraft {

Hardware::Hardware (SimulatedAircraft& aircraft, xf::ProcessingLoop& loop, nu::Logger const& logger):
	_logger (logger),
	_loop (loop),
	_aircraft (aircraft)
{
	this->udp_link.send << this->air_to_ground_link.link_output;
	this->ground_to_air_link.link_input << this->udp_link.receive;

	this->total_air_temperature_sensor.update_interval = 10_ms;
	this->total_air_temperature_sensor.noise = { 1_Pa, 1_Pa };
	this->total_air_temperature_sensor.resolution = 0.1_Pa;

	this->static_air_temperature_sensor.update_interval = 10_ms;
	this->static_air_temperature_sensor.noise = { 1_Pa, 1_Pa };
	this->static_air_temperature_sensor.resolution = 0.1_Pa;
}

} // namespace sim1::ground_station


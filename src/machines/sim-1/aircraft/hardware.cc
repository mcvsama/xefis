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
	this->air_total_pressure_sensor.update_interval = 10_ms;
	this->air_total_pressure_sensor.noise = { 1_Pa, 1_Pa };
	this->air_total_pressure_sensor.resolution = 0.1_Pa;

	this->air_static_pressure_sensor.update_interval = 10_ms;
	this->air_static_pressure_sensor.noise = { 1_Pa, 1_Pa };
	this->air_static_pressure_sensor.resolution = 0.1_Pa;

	this->air_temperature_sensor.update_interval = 10_ms;
	this->air_temperature_sensor.noise = { 0.1_K, 0.1_K };
	this->air_temperature_sensor.resolution = 0.1_K;
}

} // namespace sim1::ground_station


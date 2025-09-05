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
#include "virtual_hardware_modules.h"

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>


namespace sim1::simulation::aircraft {

VirtualHardwareModules::VirtualHardwareModules (simulation::SimulatedAircraft& aircraft, xf::ProcessingLoop& loop, nu::Logger const& logger):
	HardwareModules (loop, logger),
	_aircraft (aircraft)
{
	// Let's simulate Bosch BMP285 sensors, which have precision of 0.18 Pa
	// (or at most 0.16 Pa depending on configuration) and noise ~0.08 Pa.
	// Minimum sampling interval is about 6.4 ms (156 Hz).

	this->air_total_pressure_sensor.update_interval = 6.4_ms;
	this->air_total_pressure_sensor.noise = { 0_Pa, 0.08_Pa };
	this->air_total_pressure_sensor.resolution = 0.18_Pa;

	this->air_static_pressure_sensor.update_interval = 6.4_ms;
	this->air_static_pressure_sensor.noise = { 0_Pa, 0.08_Pa };
	this->air_static_pressure_sensor.resolution = 0.18_Pa;

	this->air_temperature_sensor.update_interval = 10_ms;
	this->air_temperature_sensor.noise = { 0_K, 0.1_K };
	this->air_temperature_sensor.resolution = 0.1_K;

	// Setup connections

	auto& h2f = this->hardware_to_flight_computer_data;
	auto& f2h = this->flight_computer_to_hardware_data;

	h2f.air_total_temperature	<< this->air_temperature_sensor.temperature;
	h2f.air_total_pressure		<< this->air_total_pressure_sensor.pressure;
	h2f.air_static_pressure		<< this->air_static_pressure_sensor.pressure;

	this->servo_controller.socket_for (_aircraft.aileron_l_servo, "aileron-left")	<< f2h.left_aileron;
	this->servo_controller.socket_for (_aircraft.aileron_r_servo, "aileron-right")	<< f2h.right_aileron;
	this->servo_controller.socket_for (_aircraft.elevator_servo, "elevator")		<< f2h.elevator;
	this->servo_controller.socket_for (_aircraft.rudder_servo, "rudder")			<< f2h.rudder;
}

} // namespace sim1::simulation::aircraft

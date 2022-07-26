/* vim:ts=4
 *
 * Copyleft 2012…2016  Michał Gawron
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
#include "virtual_temperature_sensor.h"

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/numeric.h>

// Standard:
#include <cstddef>


VirtualTemperatureSensor::VirtualTemperatureSensor (xf::sim::FlightSimulation const& flight_simulation,
													xf::SpaceVector<si::Length, xf::AirframeFrame> const& mount_location,
													xf::Logger const& logger,
													std::string_view const& instance):
	VirtualTemperatureSensorIO (instance),
	_logger (logger.with_scope (std::string (kLoggerScope) + "#" + instance)),
	_flight_simulation (flight_simulation),
	_mount_location (mount_location),
	_noise (*_io.noise)
{ }


void
VirtualTemperatureSensor::process (xf::Cycle const& cycle)
{
	auto const atmstate = _flight_simulation.complete_atmosphere_state_at (_mount_location);
	auto const& air = atmstate.air;

	_io.serviceable = true;

	if (_last_measure_time + *_io.update_interval < cycle.update_time())
	{
		_io.temperature = xf::quantized (_noise (_random_generator) + air.temperature, *_io.resolution);
		_last_measure_time = cycle.update_time();
	}
}


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


VirtualTemperatureSensor::VirtualTemperatureSensor (xf::ProcessingLoop& loop,
													nu::NonTemporary<xf::sim::TemperatureSensor const&> sensor,
													nu::Logger const& logger,
													std::string_view const instance):
	VirtualTemperatureSensorIO (loop, instance),
	_logger (logger.with_context (std::string (kLoggerScope) + "#" + instance)),
	_sensor (sensor)
{ }


void
VirtualTemperatureSensor::process (xf::Cycle const& cycle)
{
	_noise = *_io.noise;
	_io.serviceable = true; // TODO until sigmoidal temperature failure (_sensor.serviceable())

	if (_last_measure_time + *_io.update_interval < cycle.update_time())
	{
		_io.temperature = nu::quantized (_noise (_random_generator) + _sensor.measured_temperature(), *_io.resolution);
		_last_measure_time = cycle.update_time();
	}
}


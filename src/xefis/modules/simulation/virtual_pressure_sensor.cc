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
#include "virtual_pressure_sensor.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/aerodynamics/reynolds_number.h>

// Neutrino:
#include <neutrino/numeric.h>

// Standard:
#include <cstddef>


VirtualPressureSensor::VirtualPressureSensor (xf::ProcessingLoop& loop,
											  xf::sim::PrandtlTube const& prandtl_tube,
											  Probe probe,
											  nu::Logger const& logger,
											  std::string_view const instance):
	VirtualPressureSensorIO (loop, instance),
	_logger (logger.with_context (std::string (kLoggerScope) + "#" + instance)),
	_prandtl_tube (prandtl_tube),
	_probe (probe)
{ }


void
VirtualPressureSensor::process (xf::Cycle const& cycle)
{
	_noise = *_io.noise;
	_io.serviceable = true;

	if (_last_measure_time + *_io.update_interval < cycle.update_time())
	{
		si::Pressure pressure = _noise (_random_generator);

		switch (_probe)
		{
			case Pitot:
				pressure += _prandtl_tube.total_pressure();
				break;

			case Static:
				pressure += _prandtl_tube.static_pressure();
				break;
		}

		_io.pressure = nu::quantized (pressure, *_io.resolution);
		_last_measure_time = cycle.update_time();
	}
}


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
#include "temperature_sensor_widget.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/simulation/devices/temperature_sensor.h>

// Standard:
#include <cstddef>


namespace xf::sim {

TemperatureSensorWidget::TemperatureSensorWidget (TemperatureSensor& temperature_sensor):
	ObservationWidget (&temperature_sensor),
	_temperature_sensor (temperature_sensor)
{
	auto group = ObservationWidget::add_group (u8"Readings");
	group.add_observable ("Static temperature", _static_temperature);
	group.add_observable ("Measured temperature", _measured_temperature);
	group.add_observable ("Stagnation temperature", _stagnation_temperature);
}


void
TemperatureSensorWidget::update_observed_values (rigid_body::Body const* planet_body)
{
	_static_temperature = std::format ("{:.3f}", _temperature_sensor.static_temperature().to<si::Celsius>());
	_measured_temperature = std::format ("{:.3f}", _temperature_sensor.measured_temperature().to<si::Celsius>());
	_stagnation_temperature = std::format ("{:.3f}", _temperature_sensor.stagnation_temperature().to<si::Celsius>());

	ObservationWidget::update_observed_values (planet_body);
}

} // namespace xf::sim

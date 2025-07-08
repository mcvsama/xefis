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

#ifndef XEFIS__SUPPORT__SIMULATION__DEVICES__TEMPERATURE_SENSOR_WIDGET_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__DEVICES__TEMPERATURE_SENSOR_WIDGET_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/ui/observation_widget.h>

// Standard:
#include <cstddef>


namespace xf::sim {

class TemperatureSensor;


class TemperatureSensorWidget: public ObservationWidget
{
  public:
	// Ctor
	explicit
	// TODO nu::ConstLValueReference auto&& or rather nu::NonTemporaryReference auto&&
	TemperatureSensorWidget (TemperatureSensor&);

	// ObservationWidget API
	void
	update_observed_values (rigid_body::Body const* planet_body) override;

  private:
	TemperatureSensor&	_temperature_sensor;
	std::string			_static_temperature;
	std::string			_measured_temperature;
	std::string			_stagnation_temperature;
};

} // namespace xf::sim

#endif


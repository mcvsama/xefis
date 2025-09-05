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

#ifndef XEFIS__MACHINES__SIM_1__SIMULATION__AIRCRAFT__HARDWARE_MACHINE__VIRTUAL_HARDWARE_MODULES_H__INCLUDED
#define XEFIS__MACHINES__SIM_1__SIMULATION__AIRCRAFT__HARDWARE_MACHINE__VIRTUAL_HARDWARE_MODULES_H__INCLUDED

// Sim-1:
#include <machines/sim-1/aircraft/common/link/flight_computer_to_hardware_data.h>
#include <machines/sim-1/aircraft/common/link/hardware_to_flight_computer_data.h>
#include <machines/sim-1/aircraft/hardware_machine/hardware_modules.h>
#include <machines/sim-1/common/link/air_to_ground.h>
#include <machines/sim-1/common/link/crypto.h>
#include <machines/sim-1/common/link/ground_to_air.h>
#include <machines/sim-1/simulation/simulated_aircraft.h>

// Xefis:
#include <xefis/core/sockets/module_in.h>
#include <xefis/core/sockets/module_out.h>
#include <xefis/modules/comm/link/helpers.h>
#include <xefis/modules/comm/link/input_link.h>
#include <xefis/modules/comm/link/output_link.h>
#include <xefis/modules/comm/udp.h>
#include <xefis/modules/comm/xle_transceiver.h>
#include <xefis/modules/simulation/virtual_pressure_sensor.h>
#include <xefis/modules/simulation/virtual_servo_controller.h>
#include <xefis/modules/simulation/virtual_temperature_sensor.h>

// Standard:
#include <cstddef>


namespace sim1::simulation::aircraft {

class VirtualHardwareModules: public sim1::aircraft::HardwareModules
{
  public:
	// Ctor
	explicit
	VirtualHardwareModules (simulation::SimulatedAircraft& aircraft, xf::ProcessingLoop&, nu::Logger const&);

  private:
	simulation::SimulatedAircraft&	_aircraft;

  public:
	VirtualPressureSensor air_total_pressure_sensor {
		loop(),
		_aircraft.prandtl_tube,
		VirtualPressureSensor::Pitot,
		logger().with_context ("air total pressure sensor"),
		"total"
	};

	VirtualPressureSensor air_static_pressure_sensor {
		loop(),
		_aircraft.prandtl_tube,
		VirtualPressureSensor::Static,
		logger().with_context ("air static pressure sensor"),
		"static"
	};

	VirtualTemperatureSensor air_temperature_sensor {
		loop(),
		_aircraft.temperature_sensor,
		logger().with_context ("temperature sensor")
	};

	VirtualServoController servo_controller { loop() };
};

} // namespace sim1::simulation::aircraft

#endif

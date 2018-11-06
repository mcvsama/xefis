/* vim:ts=4
 *
 * Copyleft 2008…2017  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__MACHINES__SIMULATION__SIMULATION_MACHINE_H__INCLUDED
#define XEFIS__MACHINES__SIMULATION__SIMULATION_MACHINE_H__INCLUDED

// Standard:
#include <cstddef>
#include <optional>
#include <memory>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/logger.h>
#include <xefis/core/machine.h>
#include <xefis/core/processing_loop.h>
#include <xefis/core/xefis.h>
#include <xefis/modules/simulation/virtual_joystick.h>
#include <xefis/modules/simulation/virtual_pressure_sensor.h>
#include <xefis/modules/simulation/virtual_temperature_sensor.h>
#include <xefis/modules/systems/adc.h>
#include <xefis/modules/test/test_generator.h>
#include <xefis/support/airframe/airframe.h>
#include <xefis/support/earth/navigation/navaid_storage.h>
#include <xefis/support/system/work_performer.h>
#include <xefis/utility/tracker.h>

// Local:
#include "airplanes/sim_airplane.h"
#include "screens/backup.h"
#include "screens/pfd.h"


class SimulationMachine: public xf::Machine
{
  public:
	// Ctor
	SimulationMachine (xf::Xefis&);

  private:
	xf::Logger												_logger;
	std::unique_ptr<xf::NavaidStorage>						_navaid_storage;
	std::unique_ptr<xf::Airframe>							_airframe;
	// Other:
	std::optional<xf::Registrant<xf::ProcessingLoop>>		_loop;
	std::optional<xf::Registrant<SimAirplane>>				_sim_airplane;
	std::optional<xf::Registrant<VirtualJoystick>>			_virtual_joystick;
	// Sensors:
	std::optional<xf::Registrant<VirtualPressureSensor>>	_pressure_sensor_static;
	std::optional<xf::Registrant<VirtualPressureSensor>>	_pressure_sensor_total;
	std::optional<xf::Registrant<VirtualTemperatureSensor>>	_temperature_sensor_total;
	// Systems:
	std::optional<xf::Registrant<AirDataComputer>>			_air_data_computer;
	// Instruments:
	std::optional<xf::Registrant<PrimaryFlightDisplay>>		_screen_pfd;
	std::optional<xf::Registrant<BackupDisplay>>			_screen_backup;
	// WorkPerformer must finish async tasks belonging to other objects first, so it should be first to destroy:
	std::unique_ptr<xf::WorkPerformer>						_work_performer;
};

#endif


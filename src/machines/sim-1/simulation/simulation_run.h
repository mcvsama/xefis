/* vim:ts=4
 *
 * Copyleft 2024  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__MACHINES__SIM_1__SIMULATION__SIMULATION_RUN_H__INCLUDED
#define XEFIS__MACHINES__SIM_1__SIMULATION__SIMULATION_RUN_H__INCLUDED

// Local:
#include "simulated_aircraft.h"

// Sim-1:
#include <machines/sim-1/aircraft/flight_computer_machine/flight_computer_machine.h>
#include <machines/sim-1/aircraft/hardware_machine/hardware_machine.h>
#include <machines/sim-1/aircraft/radio_machine/radio_machine.h>
#include <machines/sim-1/common/common.h>
#include <machines/sim-1/ground_station/control_machine/control_machine.h>
#include <machines/sim-1/ground_station/hardware_machine/hardware_machine.h>
#include <machines/sim-1/ground_station/radio_machine/radio_machine.h>
#include <machines/sim-1/ground_station/screens_machine/screens_machine.h>
#include <machines/sim-1/simulation/aircraft/hardware_machine/virtual_hardware_modules.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/components/simulator/simulator_widget.h>
#include <xefis/core/clock.h>
#include <xefis/core/machine.h>
#include <xefis/core/machine_manager.h>
#include <xefis/core/xefis.h>
#include <xefis/support/atmosphere/simulated_atmosphere.h>
#include <xefis/support/atmosphere/standard_atmosphere.h>
#include <xefis/support/simulation/antennas/antenna_system.h>
#include <xefis/support/simulation/constraints/angular_servo_constraint.h>
#include <xefis/support/simulation/electrical/network.h>
#include <xefis/support/simulation/electrical/node_voltage_solver.h>
#include <xefis/support/simulation/rigid_body/impulse_solver.h>
#include <xefis/support/simulation/rigid_body/system.h>
#include <xefis/support/simulation/simulator.h>
#include <xefis/support/ui/rigid_body_viewer.h>

// Neutrino:
#include <neutrino/logger.h>
#include <neutrino/noncopyable.h>

// Standard:
#include <cstddef>
#include <optional>


namespace sim1::simulation {

/**
 * Single simulation run (session). These can be restarted from Simulator.
 */
class SimulationRun
{
  public:
	// Ctor
	explicit
	SimulationRun (xf::Xefis&, si::LonLatRadius<> const location, nu::Logger const&);

	xf::Simulator&
	simulator() noexcept
		{ return *_simulator; }

	xf::SimulatorWidget&
	simulator_widget() noexcept
		{ return *_simulator_widget; }

  private:
	xf::Xefis&							_xefis;
	nu::Logger							_logger;

	// These are controlled (advanced) by _simulator:
	xf::SimulatedClock					_xefis_clock				{ nu::utc_now() };
	xf::SimulatedClock					_steady_clock				{ 0_s };

	xf::SimulatedAtmosphere				_simulated_atmosphere;
	std::optional<xf::Simulator>		_simulator;
	std::optional<xf::SimulatorWidget>	_simulator_widget;
	xf::rigid_body::System				_rigid_body_system			{ _simulated_atmosphere };
	xf::rigid_body::ImpulseSolver		_rigid_body_solver			{ _rigid_body_system, 1 };
	xf::AntennaSystem					_antenna_system				{ 2_s };
	xf::electrical::Network				_electrical_network;
	xf::electrical::NodeVoltageSolver	_electrical_network_solver	{ _electrical_network, 1e-3 };
	SimulatedAircraft					_aircraft					{ make_aircraft (_rigid_body_system, _simulated_atmosphere, _antenna_system) };

	xf::MachineManager<sim1::aircraft::HardwareMachine> _aircraft_hardware_machine {
		u8"Aircraft/Hardware machine",
		[this] {
			return std::make_unique<sim1::aircraft::HardwareMachine> (
				_xefis,
				_xefis_clock,
				_steady_clock,
				[this] (xf::ProcessingLoop& loop) {
					return std::make_unique<aircraft::VirtualHardwareModules> (_aircraft, loop, _logger.with_context ("virtual hardware modules"));
				}
			);
		},
	};

	xf::MachineManager<sim1::aircraft::RadioMachine> _aircraft_radio_machine {
		u8"Aircraft/Radio machine",
		[this] {
			return std::make_unique<sim1::aircraft::RadioMachine> (
				_xefis,
				_xefis_clock,
				_steady_clock,
				true
			);
		},
	};

	xf::MachineManager<sim1::aircraft::FlightComputerMachine> _aircraft_flight_computer_machine {
		u8"Aircraft/Flight Computer machine",
		[this] {
			return std::make_unique<sim1::aircraft::FlightComputerMachine> (
				_xefis,
				_xefis_clock,
				_steady_clock,
				true
			);
		},
	};

	xf::MachineManager<sim1::ground_station::ControlMachine> _groundstation_control_machine {
		u8"Ground station/Control machine",
		[this] {
			return std::make_unique<sim1::ground_station::ControlMachine> (_xefis);
		},
	};

	xf::MachineManager<sim1::ground_station::HardwareMachine> _groundstation_hardware_machine {
		u8"Ground station/Hardware machine",
		[this] {
			return std::make_unique<sim1::ground_station::HardwareMachine> (_xefis);
		},
	};

	xf::MachineManager<sim1::ground_station::RadioMachine> _groundstation_radio_machine {
		u8"Ground station/Radio machine",
		[this] {
			return std::make_unique<sim1::ground_station::RadioMachine> (_xefis);
		},
	};

	xf::MachineManager<sim1::ground_station::ScreensMachine> _groundstation_screens_machine {
		u8"Ground station/Screens machine",
		[this] {
			return std::make_unique<sim1::ground_station::ScreensMachine> (_xefis);
		},
	};
};

} // namespace sim1::simulation

#endif

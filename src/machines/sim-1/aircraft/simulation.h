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

#ifndef XEFIS__MACHINES__SIM_1__AIRCRAFT__SIMULATION_H__INCLUDED
#define XEFIS__MACHINES__SIM_1__AIRCRAFT__SIMULATION_H__INCLUDED

// Local:
#include "simulated_aircraft.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/components/simulator/simulator_widget.h>
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


namespace sim1::aircraft {

class Machine;
class Models;


class Simulation: public neutrino::Noncopyable
{
  public:
	// Ctor
	explicit
	Simulation (Machine&, Models&, neutrino::Logger const&);

	SimulatedAircraft&
	aircraft() noexcept
		{ return _aircraft; }

	SimulatedAircraft const&
	aircraft() const noexcept
		{ return _aircraft; }

  private:
	xf::Logger							_logger;
	Models&								_models;
	xf::rigid_body::System				_rigid_body_system			{ _models.standard_atmosphere };
	xf::rigid_body::ImpulseSolver		_rigid_body_solver			{ _rigid_body_system, 30 };
	xf::electrical::Network				_electrical_network;
	xf::electrical::NodeVoltageSolver	_electrical_network_solver	{ _electrical_network, 1e-3 };
	SimulatedAircraft					_aircraft					{ make_aircraft (_rigid_body_system, _models) };
	std::optional<xf::Simulator>		_simulator;
	std::optional<xf::SimulatorWidget>	_simulator_widget;
};

} // namespace sim1::aircraft

#endif


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

#ifndef XEFIS__MACHINES__SIM_1__SIM_1_MACHINE_H__INCLUDED
#define XEFIS__MACHINES__SIM_1__SIM_1_MACHINE_H__INCLUDED

// Machine:
#include <machines/sim-1/common.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/machine.h>
#include <xefis/support/simulation/constraints/angular_servo_constraint.h>
#include <xefis/support/simulation/electrical/network.h>
#include <xefis/support/simulation/electrical/node_voltage_solver.h>
#include <xefis/support/simulation/rigid_body/group.h>
#include <xefis/support/simulation/rigid_body/impulse_solver.h>
#include <xefis/support/simulation/rigid_body/system.h>
#include <xefis/support/simulation/simulation.h>
#include <xefis/support/ui/rigid_body_viewer.h>

// Neutrino:
#include <neutrino/logger.h>

// Standard:
#include <cstddef>
#include <optional>


class Aircraft
{
  public:
	xf::rigid_body::Group					rigid_group;
	xf::rigid_body::Body*					center_body;
	xf::rigid_body::AngularServoConstraint*	aileron_l_servo;
	xf::rigid_body::AngularServoConstraint*	aileron_r_servo;
};


class Sim1Machine: public xf::Machine
{
  public:
	// Ctor
	Sim1Machine (xf::Xefis&);

  private:
	/**
	 * Return body group and the body to follow in UI widget.
	 */
	Aircraft
	construct_aircraft();

  private:
	xf::Logger							_logger;
	xf::rigid_body::System				_rigid_body_system;
	xf::rigid_body::ImpulseSolver		_rigid_body_solver			{ _rigid_body_system, 5 };
	xf::electrical::Network				_electrical_network;
	xf::electrical::NodeVoltageSolver	_electrical_network_solver	{ _electrical_network, 1e-3 };
	std::optional<xf::RigidBodyViewer>	_rigid_body_viewer;
	std::optional<xf::Simulation>		_simulation;
};

#endif


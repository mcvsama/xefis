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

#ifndef XEFIS__MACHINES__SIM_1__AIRCRAFT__SIMULATED_AIRCRAFT_H__INCLUDED
#define XEFIS__MACHINES__SIM_1__AIRCRAFT__SIMULATED_AIRCRAFT_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/simulation/devices/angular_servo.h>
#include <xefis/support/simulation/electrical/network.h>
#include <xefis/support/simulation/rigid_body/system.h>

// Standard:
#include <cstddef>


namespace sim1::aircraft {

class Models;

class SimulatedAircraft
{
  public:
	xf::rigid_body::Group		rigid_group;
	xf::rigid_body::Body&		primary_body;

//	xf::rigid_body::Body&		left_engine;
//	xf::rigid_body::Body&		right_engine;

	xf::sim::AngularServo&		aileron_l_servo;
	xf::sim::AngularServo&		aileron_r_servo;
	xf::sim::AngularServo&		elevator_servo;
	xf::sim::AngularServo&		rudder_servo;
};


/**
 * Make the aircraft body, constraints, etc.
 * The aircraft uses the standard airframe coordinates (X - nose, Y - right wing, Z - down).
 */
SimulatedAircraft
make_aircraft (xf::rigid_body::System&, Models&);

} // namespace sim1::aircraft

#endif


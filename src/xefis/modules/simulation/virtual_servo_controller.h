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

#ifndef XEFIS__MODULES__SIMULATION__VIRTUAL_SERVO_CONTROLLER_H__INCLUDED
#define XEFIS__MODULES__SIMULATION__VIRTUAL_SERVO_CONTROLLER_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/setting.h>
#include <xefis/core/sockets/module_socket.h>
#include <xefis/support/simulation/devices/interfaces/angular_servo.h>

// Standard:
#include <cstddef>
#include <string_view>


namespace si = neutrino::si;


/**
 * Simulates servo PWM generator.
 * Can couple IO sockets with provided xf::sim::AngularServos to control
 * the simulated servos.
 */
class VirtualServoController: public xf::Module
{
  public:
	// Ctor
	explicit
	VirtualServoController (xf::ProcessingLoop&, std::string_view const instance = {});

	/**
	 * Allocate/return ModuleIn for the servo (named automatically).
	 */
	xf::ModuleIn<si::Angle>&
	socket_for (xf::sim::interfaces::AngularServo&);

	/**
	 * Allocate/return new ModuleIn for the servo.
	 */
	xf::ModuleIn<si::Angle>&
	socket_for (xf::sim::interfaces::AngularServo&, std::string_view name);

  protected:
	// Module API
	void
	process (xf::Cycle const&) override;

  private:
	std::map<xf::sim::interfaces::AngularServo*, std::unique_ptr<xf::ModuleIn<si::Angle>>> _angular_servo_sockets;
};

#endif


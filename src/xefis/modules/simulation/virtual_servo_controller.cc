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
#include "virtual_servo_controller.h"

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>


VirtualServoController::VirtualServoController (xf::ProcessingLoop& loop, std::string_view const instance):
	Module (loop, instance)
{ }


xf::ModuleIn<si::Angle>&
VirtualServoController::socket_for (xf::sim::interfaces::AngularServo& servo)
{
	return socket_for (servo, std::format ("servo@{:#016x}", reinterpret_cast<uintptr_t> (&servo)));
}


xf::ModuleIn<si::Angle>&
VirtualServoController::socket_for (xf::sim::interfaces::AngularServo& servo, std::string_view const name)
{
	if (auto it = _angular_servo_sockets.find (&servo);
		it != _angular_servo_sockets.end())
	{
		return *it->second;
	}
	else
	{
		auto& ref = _angular_servo_sockets[&servo] = std::make_unique<xf::ModuleIn<si::Angle>> (this, name);
		return *ref;
	}
}


void
VirtualServoController::process (xf::Cycle const&)
{
	for (auto& [servo, socket_ptr]: _angular_servo_sockets)
		if (*socket_ptr)
			servo->set_setpoint (**socket_ptr);
}


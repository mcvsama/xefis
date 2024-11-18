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

#ifndef XEFIS__MACHINES__SIM_1__AIRCRAFT__DATA_CENTER_H__INCLUDED
#define XEFIS__MACHINES__SIM_1__AIRCRAFT__DATA_CENTER_H__INCLUDED

// Machine:
#include <machines/sim-1/common/common.h>
#include <machines/sim-1/common/link/ground_to_air.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/sockets/module_socket.h>

// Neutrino:
#include <neutrino/si/si.h>

// Standard
#include <atomic>


namespace sim1::aircraft {

class DataCenter: public xf::Module
{
  public:
	xf::ModuleIn<double>			joystick_pitch			{ this, "joystick/pitch" };
	xf::ModuleIn<double>			joystick_roll			{ this, "joystick/roll" };
	xf::ModuleIn<double>			joystick_yaw			{ this, "joystick/yaw" };
	xf::ModuleIn<double>			trim_pitch				{ this, "trim/pitch" };
	xf::ModuleIn<double>			trim_roll				{ this, "trim/roll" };
	xf::ModuleIn<double>			trim_yaw				{ this, "trim/yaw" };
	xf::ModuleIn<double>			throttle_left			{ this, "throttle/left" };
	xf::ModuleIn<double>			throttle_right			{ this, "throttle/right" };
	// If enabled, both engines throttle will be the same (averaged):
	// TODO xf::ModuleIn<bool>				throttle_gang			{ this, "throttle/gang" };

	xf::ModuleIn<si::Angle>			elevator_angle			{ this, "control surfaces/elevator/angle" };
	xf::ModuleIn<si::Angle>			aileron_left_angle		{ this, "control surfaces/aileron-left/angle" };
	xf::ModuleIn<si::Angle>			aileron_right_angle		{ this, "control surfaces/aileron-right/angle" };
	xf::ModuleIn<si::Angle>			rudder_angle			{ this, "control surfaces/rudder/angle" };
	xf::ModuleIn<si::Power>			engine_left_power		{ this, "engines/left/power" };
	xf::ModuleIn<si::Power>			engine_right_power		{ this, "engines/right/power" };

  public:
	using xf::Module::Module;
};

} // namespace sim1::aircraft

#endif


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

// Local:
#include "control_machine.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/xefis.h>


namespace sim1::ground_station {

ControlMachine::ControlMachine (xf::Xefis& xefis):
	xf::SingleLoopMachine (xefis, xefis.logger(), 120_Hz, u8"Ground station/Control machine")
{
	start();

	// TODO this should happen upon pressing a virtual button:
	_modules.master_secure_channel.start_handshake();
}


void
ControlMachine::connect_modules()
{
	_modules.ground_to_air_data.encryption_handshake_request	<< _modules.master_secure_channel.handshake_request;
	_modules.master_secure_channel.handshake_response			<< _modules.air_to_ground_data.encryption_handshake_response;

	_data_center.joystick_pitch					<< _modules.joystick.y_axis;
	_data_center.joystick_roll					<< _modules.joystick.x_axis;
	_data_center.joystick_yaw					<< _modules.joystick.pedals;
	_data_center.trim_pitch						<< 0.0;
	_data_center.trim_roll						<< 0.0;
	_data_center.trim_yaw						<< 0.0;
	// TODO if pass through function that, if ganged, calculates average of both, and raises an alert if difference is too big:
	_data_center.throttle_left					<< _modules.joystick.throttle;
	_data_center.throttle_right					<< _modules.joystick.throttle;
	_data_center.throttle_gang					<< true; // TODO unhardcode

	_modules.ground_to_air_data.joystick_pitch	<< _data_center.joystick_pitch;
	_modules.ground_to_air_data.joystick_roll	<< _data_center.joystick_roll;
	_modules.ground_to_air_data.joystick_yaw	<< _data_center.joystick_yaw;
	_modules.ground_to_air_data.trim_pitch		<< _data_center.trim_pitch;
	_modules.ground_to_air_data.trim_roll		<< _data_center.trim_roll;
	_modules.ground_to_air_data.trim_yaw		<< _data_center.trim_yaw;
	_modules.ground_to_air_data.throttle_left	<< _data_center.throttle_left;
	_modules.ground_to_air_data.throttle_right	<< _data_center.throttle_right;
}

} // namespace sim1::ground_station

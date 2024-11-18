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

// Local:
#include "machine.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/xefis_machine.h>

// Standard:
#include <cstddef>


namespace sim1::ground_station::control_machine {

Machine::Machine (xf::Xefis& xefis):
	xf::SingleLoopMachine (xefis, xefis.logger(), 120_Hz)
{
	start();

	// TODO this should happen upon pressing a virtual button:
	_hardware.master_transceiver.start_handshake();
}


void
Machine::connect_modules()
{
	_hardware.ground_to_air_data.encryption_handshake_request	<< _hardware.master_transceiver.handshake_request;
	_hardware.master_transceiver.handshake_response				<< _hardware.air_to_ground_data.encryption_handshake_response;

	_data_center.joystick_pitch					<< _hardware.joystick.y_axis;
	_data_center.joystick_roll					<< _hardware.joystick.x_axis;
	_data_center.joystick_yaw					<< _hardware.joystick.pedals;
	_data_center.trim_pitch						<< 0.0;
	_data_center.trim_roll						<< 0.0;
	_data_center.trim_yaw						<< 0.0;
	// TODO if pass through function that, if ganged, calculates average of both:
	_data_center.throttle_left					<< _hardware.joystick.throttle;
	_data_center.throttle_right					<< _hardware.joystick.throttle;

	_hardware.ground_to_air_data.joystick_pitch	<< _data_center.joystick_pitch;
	_hardware.ground_to_air_data.joystick_roll	<< _data_center.joystick_roll;
	_hardware.ground_to_air_data.joystick_yaw	<< _data_center.joystick_yaw;
	_hardware.ground_to_air_data.trim_pitch		<< _data_center.trim_pitch;
	_hardware.ground_to_air_data.trim_roll		<< _data_center.trim_roll;
	_hardware.ground_to_air_data.trim_yaw		<< _data_center.trim_yaw;
	_hardware.ground_to_air_data.throttle_left	<< _data_center.throttle_left;
	_hardware.ground_to_air_data.throttle_right	<< _data_center.throttle_right;
}

} // namespace sim1::ground_station::control_machine


std::unique_ptr<xf::Machine>
xefis_machine (xf::Xefis& xefis)
{
	return std::make_unique<sim1::ground_station::control_machine::Machine> (xefis);
}



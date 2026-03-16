/* vim:ts=4
 *
 * Copyleft 2025  Michał Gawron
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
#include "radio_modules.h"

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>


namespace sim1::aircraft {

RadioModules::RadioModules (xf::ProcessingLoop& loop, nu::Logger const& logger):
	_logger (logger),
	_loop (loop)
{
	_udp_link_to_ground_station.send							<< _air_to_ground_link.encoded_output;
	_ground_to_air_link.encoded_input							<< _udp_link_to_ground_station.receive;

	_slave_secure_channel.handshake_request						<< _ground_to_air_data.encryption_handshake_request;
	_air_to_ground_data.encryption_handshake_response			<< _slave_secure_channel.handshake_response;

	this->radio_to_flight_computer_data.joystick_pitch			<< _ground_to_air_data.joystick_pitch;
	this->radio_to_flight_computer_data.joystick_roll			<< _ground_to_air_data.joystick_roll;
	this->radio_to_flight_computer_data.joystick_yaw			<< _ground_to_air_data.joystick_yaw;
	this->radio_to_flight_computer_data.trim_pitch				<< _ground_to_air_data.trim_pitch;
	this->radio_to_flight_computer_data.trim_roll				<< _ground_to_air_data.trim_roll;
	this->radio_to_flight_computer_data.trim_yaw				<< _ground_to_air_data.trim_yaw;
	this->radio_to_flight_computer_data.throttle_left			<< _ground_to_air_data.throttle_left;
	this->radio_to_flight_computer_data.throttle_right			<< _ground_to_air_data.throttle_right;

	_air_to_ground_data.static_pressure							<< this->flight_computer_to_radio_data.static_pressure;
	_air_to_ground_data.total_pressure							<< this->flight_computer_to_radio_data.total_pressure;
}

} // namespace sim1::aircraft

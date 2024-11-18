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


namespace sim1::aircraft {

namespace rb = xf::rigid_body;


Machine::Machine (xf::Xefis& xefis):
	xf::SingleLoopMachine (xefis, xefis.logger(), 120_Hz)
{
	start();
}


void
Machine::connect_modules()
{
	_hardware.slave_transceiver.handshake_request				<< _hardware.ground_to_air_data.encryption_handshake_request;
	_hardware.air_to_ground_data.encryption_handshake_response	<< _hardware.slave_transceiver.handshake_response;

	constexpr auto kAileronsLimits = xf::Range { -30_deg, +30_deg };
	constexpr auto kElevatorLimits = xf::Range { -30_deg, +30_deg };
	constexpr auto kRudderLimits = xf::Range { -30_deg, +30_deg };

	auto const throttle_to_power = std::function ([](double v) {
		return 1_W * v;
	});
	auto const joystick_to_elevator = std::function ([&] (double const angle) -> si::Angle {
		return neutrino::renormalize (angle, xf::Range { -1.0, +1.0 }, kElevatorLimits);
	});
	auto const joystick_to_ailerons = std::function ([&] (double const angle) -> si::Angle {
		return neutrino::renormalize (angle, xf::Range { -1.0, +1.0 }, kAileronsLimits);
	});
	auto const joystick_to_rudder = std::function ([&] (double const angle) -> si::Angle {
		return neutrino::renormalize (angle, xf::Range { -1.0, +1.0 }, kRudderLimits);
	});

	_data_center.joystick_pitch	<< _hardware.ground_to_air_data.joystick_pitch;
	_data_center.joystick_roll	<< _hardware.ground_to_air_data.joystick_roll;
	_data_center.joystick_yaw	<< _hardware.ground_to_air_data.joystick_yaw;
	_data_center.trim_pitch		<< _hardware.ground_to_air_data.trim_pitch;
	_data_center.trim_roll		<< _hardware.ground_to_air_data.trim_roll;
	_data_center.trim_yaw		<< _hardware.ground_to_air_data.trim_yaw;
	_data_center.throttle_left	<< _hardware.ground_to_air_data.throttle_left;
	_data_center.throttle_right	<< _hardware.ground_to_air_data.throttle_right;

	// TODO won't this give 1-cycle delay?
	_data_center.elevator_angle			<< joystick_to_elevator	<< _data_center.joystick_pitch;
	_data_center.aileron_left_angle		<< joystick_to_ailerons	<< _data_center.joystick_roll;
	_data_center.aileron_right_angle	<< joystick_to_ailerons	<< _data_center.joystick_roll;
	_data_center.rudder_angle			<< joystick_to_rudder	<< _data_center.joystick_yaw;

	// TODO won't this give 1-cycle delay?
	_data_center.engine_left_power	<< throttle_to_power << _data_center.throttle_left;
	_data_center.engine_right_power	<< throttle_to_power << _data_center.throttle_right;

	auto& aircraft = _simulation.aircraft();
	_hardware.servo_controller.socket_for (aircraft.elevator_servo)		<< _data_center.elevator_angle;
	_hardware.servo_controller.socket_for (aircraft.aileron_l_servo)	<< _data_center.aileron_left_angle;
	_hardware.servo_controller.socket_for (aircraft.aileron_r_servo)	<< _data_center.aileron_right_angle;
	_hardware.servo_controller.socket_for (aircraft.rudder_servo)		<< _data_center.rudder_angle;
}

} // namespace sim1::aircraft


std::unique_ptr<xf::Machine>
xefis_machine (xf::Xefis& xefis)
{
	return std::make_unique<sim1::aircraft::Machine> (xefis);
}



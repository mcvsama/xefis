/* vim:ts=4
 *
 * Copyleft 2012  Michał Gawron
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
#include "flight_computer_modules.h"

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <algorithm>
#include <cstddef>


namespace sim1::aircraft {
namespace {

constexpr si::Angle kMaximumManualControlDeflection = 45_deg;

std::function<si::Angle (double)> const kToSurfaceDeflection =
	[](double const normalized_deflection) {
		return std::clamp (normalized_deflection, -1.0, +1.0) * kMaximumManualControlDeflection;
	};

} // namespace


FlightComputerModules::FlightComputerModules (xf::ProcessingLoop& loop, nu::Logger const& logger):
	_logger (logger.with_context ("flight computer modules")),
	_loop (loop)
{
	_pitch_control_mixer.output_minimum = -1.0;
	_pitch_control_mixer.output_maximum = +1.0;
	_roll_control_mixer.output_minimum = -1.0;
	_roll_control_mixer.output_maximum = +1.0;
	_yaw_control_mixer.output_minimum = -1.0;
	_yaw_control_mixer.output_maximum = +1.0;

	_air_data_computer.ias_valid_minimum = 1_mps;
	_air_data_computer.ias_valid_maximum = 100_mps;

	_air_data_computer.pressure_use_std						<< false;
	_air_data_computer.pressure_qnh							<< 1013.25_hPa;
	_air_data_computer.pressure_static						<< this->hardware_to_flight_computer_data.air_static_pressure;
	_air_data_computer.pressure_total						<< this->hardware_to_flight_computer_data.air_total_pressure;
	_air_data_computer.total_air_temperature				<< this->hardware_to_flight_computer_data.air_total_temperature;

	this->flight_computer_to_radio_data.static_pressure		<< this->hardware_to_flight_computer_data.air_static_pressure;
	this->flight_computer_to_radio_data.total_pressure		<< this->hardware_to_flight_computer_data.air_total_pressure;

	_pitch_control_mixer.input_a_value						<< this->radio_to_flight_computer_data.joystick_pitch;
	_pitch_control_mixer.input_b_value						<< this->radio_to_flight_computer_data.trim_pitch;
	_roll_control_mixer.input_a_value						<< this->radio_to_flight_computer_data.joystick_roll;
	_roll_control_mixer.input_b_value						<< this->radio_to_flight_computer_data.trim_roll;
	_yaw_control_mixer.input_a_value						<< this->radio_to_flight_computer_data.joystick_yaw;
	_yaw_control_mixer.input_b_value						<< this->radio_to_flight_computer_data.trim_yaw;

	this->flight_computer_to_hardware_data.left_aileron		<< kToSurfaceDeflection << _roll_control_mixer.output_value;
	this->flight_computer_to_hardware_data.right_aileron	<< kToSurfaceDeflection << _roll_control_mixer.output_value;
	this->flight_computer_to_hardware_data.elevator			<< kToSurfaceDeflection << _pitch_control_mixer.output_value;
	this->flight_computer_to_hardware_data.rudder			<< kToSurfaceDeflection << _yaw_control_mixer.output_value;
}

} // namespace sim1::aircraft

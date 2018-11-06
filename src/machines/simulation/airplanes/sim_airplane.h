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

#ifndef XEFIS__MACHINES__SIMULATION__AIRPLANES__SIM_AIRPLANE_H__INCLUDED
#define XEFIS__MACHINES__SIMULATION__AIRPLANES__SIM_AIRPLANE_H__INCLUDED

// Standard:
#include <cstddef>
#include <memory>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/property.h>
#include <xefis/core/setting.h>
#include <xefis/support/simulation/airfoil.h>
#include <xefis/support/simulation/engine.h>
#include <xefis/support/simulation/flight_simulation.h>


class SimAirplaneIO: public xf::ModuleIO
{
  public:
	/*
	 * Settings
	 */


	/*
	 * Input
	 */

	xf::PropertyIn<double>				joystick_x_axis					{ this, "sim_airplane/joystick/x-axis" };
	xf::PropertyIn<double>				joystick_y_axis					{ this, "sim_airplane/joystick/y-axis" };
	xf::PropertyIn<double>				joystick_throttle				{ this, "sim_airplane/joystick/throttle" };
	xf::PropertyIn<double>				joystick_rudder					{ this, "sim_airplane/joystick/rudder" };

	/*
	 * Output
	 */

	// True values
	xf::PropertyOut<si::Velocity>		real_cas						{ this, "sim_airplane/real/speeds/cas" };
	xf::PropertyOut<si::Velocity>		real_ground_speed				{ this, "sim_airplane/real/speeds/ground" };
	xf::PropertyOut<si::Velocity>		real_vertical_speed				{ this, "sim_airplane/real/speeds/vertical" };
	xf::PropertyOut<si::Temperature>	real_sat						{ this, "sim_airplane/real/air/temperature.static" };
	xf::PropertyOut<si::Angle>			real_orientation_pitch			{ this, "sim_airplane/real/orientation/pitch" };
	xf::PropertyOut<si::Angle>			real_orientation_roll			{ this, "sim_airplane/real/orientation/roll" };
	xf::PropertyOut<si::Angle>			real_orientation_heading_true	{ this, "sim_airplane/real/orientation/heading.true" };
	xf::PropertyOut<si::Angle>			real_track_lateral_true			{ this, "sim_airplane/real/track/lateral.true" };
	xf::PropertyOut<si::Angle>			real_track_vertical				{ this, "sim_airplane/real/track/vertical" };
	xf::PropertyOut<si::Length>			real_altitude_amsl				{ this, "sim_airplane/real/altitude/amsl" };
	xf::PropertyOut<si::Length>			real_altitude_agl				{ this, "sim_airplane/real/altitude/agl" };
	xf::PropertyOut<si::Angle>			real_aoa_alpha					{ this, "sim_airplane/real/aoa/alpha" };
	xf::PropertyOut<si::Angle>			real_aoa_alpha_maximum			{ this, "sim_airplane/real/aoa/alpha.maximum" };
	xf::PropertyOut<si::Angle>			real_aoa_beta					{ this, "sim_airplane/real/aoa/beta" };
	xf::PropertyOut<si::Angle>			real_position_longitude			{ this, "sim_airplane/real/position/longitude" };
	xf::PropertyOut<si::Angle>			real_position_latitude			{ this, "sim_airplane/real/position/latitude" };
	xf::PropertyOut<si::Angle>			real_slip_skid					{ this, "sim_airplane/real/slip-skid/angle" };

	// TODO
	xf::PropertyOut<si::Power>			requested_engine_left_power		{ this, "sim_airplane/engine-left/requested-power" };
	xf::PropertyOut<si::Power>			engine_left_power				{ this, "sim_airplane/engine-left/power" };
	xf::PropertyOut<si::Force>			engine_left_thrust				{ this, "sim_airplane/engine-left/thrust" };
	xf::PropertyOut<si::Power>			requested_engine_right_power	{ this, "sim_airplane/engine-right/requested-power" };
	xf::PropertyOut<si::Power>			engine_right_power				{ this, "sim_airplane/engine-right/power" };
	xf::PropertyOut<si::Force>			engine_right_thrust				{ this, "sim_airplane/engine-right/thrust" };
};


class SimAirplane: public xf::Module<SimAirplaneIO>
{
	struct Controls
	{
		xf::sim::Airfoil*	wing_L		{ nullptr };
		xf::sim::Airfoil*	wing_R		{ nullptr };
		xf::sim::Airfoil*	aileron_L	{ nullptr };
		xf::sim::Airfoil*	aileron_R	{ nullptr };
		xf::sim::Airfoil*	elevator	{ nullptr };
		xf::sim::Airfoil*	rudder		{ nullptr };
		xf::sim::Engine*	engine_L	{ nullptr };
		xf::sim::Engine*	engine_R	{ nullptr };
	};

  public:
	// Ctor
	explicit
	SimAirplane (std::unique_ptr<SimAirplaneIO>, xf::Logger const& logger, std::string_view const& instance = {});

	xf::sim::FlightSimulation const&
	simulation() const noexcept
		{ return *_simulation; }

  protected:
	// Module API
	void
	process (xf::Cycle const&) override;

  private:
	void
	set_inputs();

	void
	set_outputs();

	xf::sim::BodyShape
	make_airframe_shape (xf::SpaceVector<si::Length, xf::BodyFrame> const& center_of_mass);

  private:
	std::unique_ptr<xf::sim::FlightSimulation>	_simulation;
	Controls									_controls;
};

#endif


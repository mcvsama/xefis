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

#ifndef XEFIS__MODULES__SYSTEMS__NC_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__NC_H__INCLUDED

// Standard:
#include <cstddef>

// Boost:
#include <boost/circular_buffer.hpp>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/sockets/module_socket.h>
#include <xefis/support/sockets/socket_observer.h>
#include <xefis/utility/smoother.h>
#include <xefis/utility/range_smoother.h>


namespace si = neutrino::si;
using namespace neutrino::si::literals;


class NavigationComputerIO: public xf::ModuleIO
{
  public:
	/*
	 * Input
	 */

	xf::ModuleIn<si::Angle>				input_position_longitude			{ this, "position/longitude" };
	xf::ModuleIn<si::Angle>				input_position_latitude				{ this, "position/latitude" };
	xf::ModuleIn<si::Length>			input_position_altitude_amsl		{ this, "position/altitude.amsl" };
	xf::ModuleIn<si::Length>			input_position_lateral_stddev		{ this, "position/lateral.standard-deviation" };
	xf::ModuleIn<si::Length>			input_position_vertical_stddev		{ this, "position/vertical.standard-deviation" };
	xf::ModuleIn<std::string>			input_position_source				{ this, "position/source" };
	xf::ModuleIn<si::Angle>				input_orientation_pitch				{ this, "orientation/pitch" };
	xf::ModuleIn<si::Angle>				input_orientation_roll				{ this, "orientation/roll" };
	xf::ModuleIn<si::Angle>				input_orientation_heading_magnetic	{ this, "orientation/heading.magnetic" };

	/*
	 * Output
	 */

	xf::ModuleOut<si::Angle>			position_longitude					{ this, "position/longitude" };
	xf::ModuleOut<si::Angle>			position_latitude					{ this, "position/latitude" };
	xf::ModuleOut<si::Length>			position_altitude_amsl				{ this, "position/altitude.amsl" };
	xf::ModuleOut<si::Length>			position_lateral_stddev				{ this, "position/lateral.standard-deviation" };
	xf::ModuleOut<si::Length>			position_vertical_stddev			{ this, "position/vertical.standard-deviation" };
	xf::ModuleOut<si::Length>			position_stddev						{ this, "position/standard-deviation" };
	xf::ModuleOut<std::string>			position_source						{ this, "position/source" };
	xf::ModuleOut<si::Angle>			orientation_pitch					{ this, "orientation/pitch" };
	xf::ModuleOut<si::Angle>			orientation_roll					{ this, "orientation/roll" };
	xf::ModuleOut<si::Angle>			orientation_heading_magnetic		{ this, "orientation/heading.magnetic" };
	xf::ModuleOut<si::Angle>			orientation_heading_true			{ this, "orientation/heading.true" };
	xf::ModuleOut<si::Angle>			track_vertical						{ this, "track/vertical" };
	xf::ModuleOut<si::Angle>			track_lateral_magnetic				{ this, "track/lateral.magnetic" };
	xf::ModuleOut<si::Angle>			track_lateral_true					{ this, "track/lateral.true" };
	xf::ModuleOut<si::AngularVelocity>	track_lateral_rotation				{ this, "track/rotation" };
	xf::ModuleOut<si::Velocity>			track_ground_speed					{ this, "track/ground-speed" };
	xf::ModuleOut<si::Angle>			magnetic_declination				{ this, "magnetic-declination" };
	xf::ModuleOut<si::Angle>			magnetic_inclination				{ this, "magnetic-inclination" };
};


class NavigationComputer: public xf::Module<NavigationComputerIO>
{
  private:
	struct Position
	{
		si::LonLat	lateral_position;
		si::Length	lateral_position_stddev;
		si::Length	altitude;
		si::Length	altitude_stddev;
		si::Time	time;
		bool		valid = false;
	};

	typedef boost::circular_buffer<Position> Positions;

  public:
	// Ctor
	explicit
	NavigationComputer (std::unique_ptr<NavigationComputerIO>, std::string_view const& instance);

  protected:
	// Module API
	void
	process (xf::Cycle const&) override;

	void
	compute_position();

	void
	compute_magnetic_variation();

	void
	compute_headings();

	void
	compute_track();

	void
	compute_ground_speed();

  private:
	Positions							_positions								{ 3 };
	Positions							_positions_accurate_2_times				{ 3 };
	Positions							_positions_accurate_9_times				{ 3 };
	// Note: SocketObservers depend on Smoothers, so first Smoothers must be defined,
	// then SocketObservers, to ensure correct order of destruction.
	xf::RangeSmoother<si::Angle>		_orientation_pitch_smoother				{ { -180.0_deg, +180.0_deg }, 25_ms };
	xf::RangeSmoother<si::Angle>		_orientation_roll_smoother				{ { -180.0_deg, +180.0_deg }, 25_ms };
	xf::RangeSmoother<si::Angle>		_orientation_heading_magnetic_smoother	{ { 0.0_deg, 360.0_deg }, 200_ms };
	xf::Smoother<si::Angle>				_track_vertical_smoother				{ 500_ms };
	xf::RangeSmoother<si::Angle>		_track_lateral_true_smoother			{ { 0.0_deg, 360.0_deg }, 500_ms };
	xf::Smoother<si::AngularVelocity>	_track_lateral_rotation_smoother		{ 1500_ms };
	xf::Smoother<si::Velocity>			_track_ground_speed_smoother			{ 2_s };
	si::Time							_track_accumulated_dt					{ 0_s };
	xf::SocketObserver					_position_computer;
	xf::SocketObserver					_magnetic_variation_computer;
	xf::SocketObserver					_headings_computer;
	xf::SocketObserver					_track_computer;
	xf::SocketObserver					_ground_speed_computer;
};

#endif

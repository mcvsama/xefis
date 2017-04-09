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
#include <xefis/core/v2/module.h>
#include <xefis/core/v2/property.h>
#include <xefis/core/v2/property_observer.h>
#include <xefis/utility/smoother.h>
#include <xefis/utility/range_smoother.h>


class NavigationComputer: public v2::Module
{
  public:
	/*
	 * Input
	 */

	v2::PropertyIn<si::Angle>				input_position_longitude			{ this, "/position/longitude" };
	v2::PropertyIn<si::Angle>				input_position_latitude				{ this, "/position/latitude" };
	v2::PropertyIn<si::Length>				input_position_altitude_amsl		{ this, "/position/altitude.amsl" };
	v2::PropertyIn<si::Length>				input_position_lateral_stddev		{ this, "/position/lateral.standard-deviation" };
	v2::PropertyIn<si::Length>				input_position_vertical_stddev		{ this, "/position/vertical.standard-deviation" };
	v2::PropertyIn<std::string>				input_position_source				{ this, "/position/source" };
	v2::PropertyIn<si::Angle>				input_orientation_pitch				{ this, "/orientation/pitch" };
	v2::PropertyIn<si::Angle>				input_orientation_roll				{ this, "/orientation/roll" };
	v2::PropertyIn<si::Angle>				input_orientation_heading_magnetic	{ this, "/orientation/heading.magnetic" };

	/*
	 * Output
	 */

	v2::PropertyOut<si::Angle>				output_position_longitude			{ this, "/position/longitude" };
	v2::PropertyOut<si::Angle>				output_position_latitude			{ this, "/position/latitude" };
	v2::PropertyOut<si::Length>				output_position_altitude_amsl		{ this, "/position/altitude.amsl" };
	v2::PropertyOut<si::Length>				output_position_lateral_stddev		{ this, "/position/lateral.standard-deviation" };
	v2::PropertyOut<si::Length>				output_position_vertical_stddev		{ this, "/position/vertical.standard-deviation" };
	v2::PropertyOut<si::Length>				output_position_stddev				{ this, "/position/standard-deviation" };
	v2::PropertyOut<std::string>			output_position_source				{ this, "/position/source" };
	v2::PropertyOut<si::Angle>				output_orientation_pitch			{ this, "/orientation/pitch" };
	v2::PropertyOut<si::Angle>				output_orientation_roll				{ this, "/orientation/roll" };
	v2::PropertyOut<si::Angle>				output_orientation_heading_magnetic	{ this, "/orientation/heading.magnetic" };
	v2::PropertyOut<si::Angle>				output_orientation_heading_true		{ this, "/orientation/heading.true" };
	v2::PropertyOut<si::Angle>				output_track_vertical				{ this, "/track/vertical" };
	v2::PropertyOut<si::Angle>				output_track_lateral_magnetic		{ this, "/track/lateral.magnetic" };
	v2::PropertyOut<si::Angle>				output_track_lateral_true			{ this, "/track/lateral.true" };
	v2::PropertyOut<si::AngularVelocity>	output_track_lateral_rotation		{ this, "/track/rotation" };
	v2::PropertyOut<si::Velocity>			output_track_ground_speed			{ this, "/track/ground-speed" };
	v2::PropertyOut<si::Angle>				output_magnetic_declination			{ this, "/magnetic-declination" };
	v2::PropertyOut<si::Angle>				output_magnetic_inclination			{ this, "/magnetic-inclination" };

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
	NavigationComputer (std::string const& instance);

  protected:
	// Module API
	void
	process (v2::Cycle const&) override;

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
	// Note: PropertyObservers depend on Smoothers, so first Smoothers must be defined,
	// then PropertyObservers, to ensure correct order of destruction.
	xf::RangeSmoother<si::Angle>		_orientation_pitch_smoother				{ { -180.0_deg, +180.0_deg }, 25_ms };
	xf::RangeSmoother<si::Angle>		_orientation_roll_smoother				{ { -180.0_deg, +180.0_deg }, 25_ms };
	xf::RangeSmoother<si::Angle>		_orientation_heading_magnetic_smoother	{ { 0.0_deg, 360.0_deg }, 200_ms };
	xf::Smoother<si::Angle>				_track_vertical_smoother				{ 500_ms };
	xf::RangeSmoother<si::Angle>		_track_lateral_true_smoother			{ { 0.0_deg, 360.0_deg }, 500_ms };
	xf::Smoother<si::AngularVelocity>	_track_lateral_rotation_smoother		{ 1500_ms };
	xf::Smoother<si::Velocity>			_track_ground_speed_smoother			{ 2_s };
	si::Time							_track_accumulated_dt					{ 0_s };
	v2::PropertyObserver				_position_computer;
	v2::PropertyObserver				_magnetic_variation_computer;
	v2::PropertyObserver				_headings_computer;
	v2::PropertyObserver				_track_computer;
	v2::PropertyObserver				_ground_speed_computer;
};

#endif

/* vim:ts=4
 *
 * Copyleft 2012…2015  Michał Gawron
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

// Qt:
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/property.h>
#include <xefis/core/property_observer.h>
#include <xefis/utility/smoother.h>


class NavigationComputer: public xf::Module
{
  private:
	struct Position
	{
		LonLat		lateral_position;
		Length		lateral_position_stddev;
		Length		altitude;
		Length		altitude_stddev;
		Time		time;
		bool		valid = false;
	};

	typedef boost::circular_buffer<Position> Positions;

  public:
	// Ctor
	NavigationComputer (xf::ModuleManager*, QDomElement const& config);

  private:
	void
	data_updated() override;

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
	Positions				_positions;
	Positions				_positions_accurate_2_times;
	Positions				_positions_accurate_9_times;
	// Note: PropertyObservers depend on Smoothers, so first Smoothers must be defined,
	// then PropertyObservers, to ensure correct order of destruction.
	xf::Smoother<double>	_orientation_pitch_smoother				= 25_ms;
	xf::Smoother<double>	_orientation_roll_smoother				= 25_ms;
	xf::Smoother<double>	_orientation_heading_magnetic_smoother	= 200_ms;
	xf::Smoother<double>	_track_vertical_smoother				= 500_ms;
	xf::Smoother<double>	_track_lateral_true_smoother			= 500_ms;
	xf::Smoother<double>	_track_lateral_rotation_smoother		= 1500_ms;
	xf::Smoother<double>	_track_ground_speed_smoother			= 2_s;
	Time					_track_accumulated_dt					= 0_s;
	// Input:
	xf::PropertyAngle		_position_input_longitude;
	xf::PropertyAngle		_position_input_latitude;
	xf::PropertyLength		_position_input_altitude_amsl;
	xf::PropertyLength		_position_input_lateral_stddev;
	xf::PropertyLength		_position_input_vertical_stddev;
	xf::PropertyString		_position_input_source;
	xf::PropertyAngle		_orientation_input_pitch;
	xf::PropertyAngle		_orientation_input_roll;
	xf::PropertyAngle		_orientation_input_heading_magnetic;
	// Output:
	xf::PropertyAngle		_position_longitude;
	xf::PropertyAngle		_position_latitude;
	xf::PropertyLength		_position_altitude_amsl;
	xf::PropertyLength		_position_lateral_stddev;
	xf::PropertyLength		_position_vertical_stddev;
	xf::PropertyLength		_position_stddev;
	xf::PropertyString		_position_source;
	xf::PropertyAngle		_orientation_pitch;
	xf::PropertyAngle		_orientation_roll;
	xf::PropertyAngle		_orientation_heading_magnetic;
	xf::PropertyAngle		_orientation_heading_true;
	xf::PropertyAngle		_track_vertical;
	xf::PropertyAngle		_track_lateral_magnetic;
	xf::PropertyAngle		_track_lateral_true;
	xf::PropertyFrequency	_track_lateral_rotation;
	xf::PropertySpeed		_track_ground_speed;
	xf::PropertyAngle		_magnetic_declination;
	xf::PropertyAngle		_magnetic_inclination;
	// Other:
	xf::PropertyObserver	_position_computer;
	xf::PropertyObserver	_magnetic_variation_computer;
	xf::PropertyObserver	_headings_computer;
	xf::PropertyObserver	_track_computer;
	xf::PropertyObserver	_ground_speed_computer;
};

#endif

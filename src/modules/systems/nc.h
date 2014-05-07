/* vim:ts=4
 *
 * Copyleft 2012…2014  Michał Gawron
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


class NavigationComputer: public Xefis::Module
{
  private:
	struct Position
	{
		LonLat		lateral_position;
		Length		altitude;
		Length		lateral_accuracy;
		Length		vertical_accuracy;
		Time		time;
		bool		valid = false;
	};

	typedef boost::circular_buffer<Position> Positions;

  public:
	// Ctor
	NavigationComputer (Xefis::ModuleManager*, QDomElement const& config);

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
	Positions					_positions;
	Positions					_positions_accurate_2_times;
	Positions					_positions_accurate_9_times;
	// Note: PropertyObservers depend on Smoothers, so first Smoothers must be defined,
	// then PropertyObservers, to ensure correct order of destruction.
	Xefis::Smoother<double>		_orientation_pitch_smoother				= 25_ms;
	Xefis::Smoother<double>		_orientation_roll_smoother				= 25_ms;
	Xefis::Smoother<double>		_orientation_heading_magnetic_smoother	= 200_ms;
	Xefis::Smoother<double>		_track_vertical_smoother				= 500_ms;
	Xefis::Smoother<double>		_track_lateral_true_smoother			= 500_ms;
	Xefis::Smoother<double>		_track_heading_delta_smoother			= 500_ms;
	Xefis::Smoother<double>		_track_ground_speed_smoother			= 2_s;
	Time						_track_accumulated_dt					= 0_s;
	// Input:
	Xefis::PropertyAngle		_position_input_longitude;
	Xefis::PropertyAngle		_position_input_latitude;
	Xefis::PropertyLength		_position_input_altitude_amsl;
	Xefis::PropertyLength		_position_input_accuracy_lateral;
	Xefis::PropertyLength		_position_input_accuracy_vertical;
	Xefis::PropertyString		_position_input_source;
	Xefis::PropertyAngle		_orientation_input_pitch;
	Xefis::PropertyAngle		_orientation_input_roll;
	Xefis::PropertyAngle		_orientation_input_heading_magnetic;
	// Output:
	Xefis::PropertyAngle		_position_longitude;
	Xefis::PropertyAngle		_position_latitude;
	Xefis::PropertyLength		_position_altitude_amsl;
	Xefis::PropertyLength		_position_accuracy_lateral;
	Xefis::PropertyLength		_position_accuracy_vertical;
	Xefis::PropertyString		_position_source;
	Xefis::PropertyAngle		_orientation_pitch;
	Xefis::PropertyAngle		_orientation_roll;
	Xefis::PropertyAngle		_orientation_heading_magnetic;
	Xefis::PropertyAngle		_orientation_heading_true;
	Xefis::PropertyAngle		_track_vertical;
	Xefis::PropertyAngle		_track_lateral_magnetic;
	Xefis::PropertyAngle		_track_lateral_true;
	Xefis::PropertyAngle		_track_lateral_delta_dpm;
	Xefis::PropertySpeed		_track_ground_speed;
	Xefis::PropertyAngle		_magnetic_declination;
	Xefis::PropertyAngle		_magnetic_inclination;
	// Other:
	Xefis::PropertyObserver		_position_computer;
	Xefis::PropertyObserver		_magnetic_variation_computer;
	Xefis::PropertyObserver		_headings_computer;
	Xefis::PropertyObserver		_track_computer;
	Xefis::PropertyObserver		_ground_speed_computer;
};

#endif

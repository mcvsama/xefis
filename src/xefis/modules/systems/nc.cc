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

// Local:
#include "nc.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/earth/earth.h>
#include <xefis/support/earth/navigation/magnetic_variation.h>

// Neutrino:
#include <neutrino/exception.h>
#include <neutrino/time_helper.h>

// Qt:
#include <QtCore/QDate>
#include <QtCore/QDateTime>

// Standard:
#include <cstddef>


NavigationComputer::NavigationComputer (std::string_view const& instance):
	NavigationComputerIO (instance)
{
	// Initialize _positions* with invalid vals, to get them non-empty:
	for (Positions* positions: { &_positions, &_positions_accurate_2_times, &_positions_accurate_9_times })
		for (std::size_t i = 0; i < positions->capacity(); ++i)
			positions->push_back (Position());

	_position_computer.set_callback (std::bind (&NavigationComputer::compute_position, this));
	_position_computer.observe ({
		&_io.input_position_longitude,
		&_io.input_position_latitude,
		&_io.input_position_altitude_amsl,
		&_io.input_position_lateral_stddev,
		&_io.input_position_vertical_stddev,
		&_io.input_position_source,
	});

	_magnetic_variation_computer.set_callback (std::bind (&NavigationComputer::compute_magnetic_variation, this));
	_magnetic_variation_computer.observe ({
		&_io.position_longitude,
		&_io.position_latitude,
		&_io.position_altitude_amsl
	});

	_headings_computer.set_callback (std::bind (&NavigationComputer::compute_headings, this));
	_headings_computer.add_depending_smoothers ({
		&_orientation_heading_magnetic_smoother,
		&_orientation_pitch_smoother,
		&_orientation_roll_smoother,
	});
	_headings_computer.observe ({
		&_io.input_orientation_heading_magnetic,
		&_io.input_orientation_pitch,
		&_io.input_orientation_roll,
		&_io.magnetic_declination,
	});

	_track_computer.set_callback (std::bind (&NavigationComputer::compute_track, this));
	_track_computer.add_depending_smoothers ({
		&_track_vertical_smoother,
		&_track_lateral_true_smoother,
		&_track_lateral_rotation_smoother,
	});
	_track_computer.observe ({
		&_position_computer,
		&_io.magnetic_declination,
	});

	_ground_speed_computer.set_callback (std::bind (&NavigationComputer::compute_ground_speed, this));
	_ground_speed_computer.add_depending_smoothers ({
		&_track_ground_speed_smoother,
	});
	_ground_speed_computer.observe ({
		&_position_computer
	});
}


void
NavigationComputer::process (xf::Cycle const& cycle)
{
	xf::SocketObserver* computers[] = {
		// Order is important:
		&_position_computer,
		&_magnetic_variation_computer,
		&_headings_computer,
		&_track_computer,
		&_ground_speed_computer,
	};

	for (auto* o: computers)
		o->process (cycle.update_time());
}


void
NavigationComputer::compute_position()
{
	si::Time update_time = _position_computer.update_time();

	// Larger of the two:
	if (_io.position_lateral_stddev && _io.position_vertical_stddev)
		_io.position_stddev = std::max (*_io.position_lateral_stddev, *_io.position_vertical_stddev);
	else
		_io.position_stddev = xf::nil;

	si::Length const failed_accuracy = 100_nmi;

	Position position;
	position.lateral_position = si::LonLat (*_io.position_longitude, *_io.position_latitude);
	position.lateral_position_stddev = _io.position_lateral_stddev.value_or (failed_accuracy);
	position.altitude = _io.position_altitude_amsl.value_or (0.0_ft);
	position.altitude_stddev = _io.position_vertical_stddev.value_or (failed_accuracy);
	position.time = update_time;
	position.valid = _io.position_longitude &&
					 _io.position_latitude &&
					 _io.position_altitude_amsl &&
					 _io.position_lateral_stddev &&
					 _io.position_vertical_stddev;
	_positions.push_back (position);

	// Delayed positioning (after enough distance has been traveled):
	if (_positions.back().valid)
	{
		auto add_accurate_position = [&](Positions& accurate_positions, float accuracy_times, si::Time max_time_difference) -> void
		{
			Position const& new_position = _positions.back();
			si::Length worse_accuracy = std::max (new_position.lateral_position_stddev, accurate_positions.back().lateral_position_stddev);

			if (!accurate_positions.back().valid ||
				xf::haversine_earth (new_position.lateral_position, accurate_positions.back().lateral_position) > accuracy_times * worse_accuracy ||
				new_position.time - accurate_positions.back().time > max_time_difference)
			{
				accurate_positions.push_back (new_position);
			}
		};

		add_accurate_position (_positions_accurate_2_times, 2.f, 1_s);
		add_accurate_position (_positions_accurate_9_times, 9.f, 2_s);
	}
	else
	{
		_positions_accurate_2_times.back().valid = false;
		_positions_accurate_9_times.back().valid = false;
	}
}


void
NavigationComputer::compute_magnetic_variation()
{
	if (_io.position_longitude && _io.position_latitude)
	{
		xf::MagneticVariation mv;
		mv.set_position (si::LonLat (*_io.position_longitude, *_io.position_latitude));
		if (_io.position_altitude_amsl)
			mv.set_altitude_amsl (*_io.position_altitude_amsl);
		else
			mv.set_altitude_amsl (0_ft);
		QDate today = QDateTime::fromTime_t (xf::TimeHelper::now().in<si::Second>()).date();
		mv.set_date (today.year(), today.month(), today.day());
		mv.update();
		_io.magnetic_declination = mv.magnetic_declination();
		_io.magnetic_inclination = mv.magnetic_inclination();
	}
	else
	{
		_io.magnetic_declination = xf::nil;
		_io.magnetic_inclination = xf::nil;
	}
}


void
NavigationComputer::compute_headings()
{
	si::Time update_dt = _headings_computer.update_dt();

	if (_io.input_orientation_heading_magnetic)
	{
		_io.orientation_heading_magnetic = _orientation_heading_magnetic_smoother (*_io.input_orientation_heading_magnetic, update_dt);

		if (_io.magnetic_declination)
			_io.orientation_heading_true = xf::magnetic_to_true (*_io.orientation_heading_magnetic, *_io.magnetic_declination);
		else
			_io.orientation_heading_true = xf::nil;
	}
	else
	{
		_io.orientation_heading_magnetic = xf::nil;
		_io.orientation_heading_true = xf::nil;
		_orientation_heading_magnetic_smoother.invalidate();
	}

	// Smoothed pitch:
	if (_io.input_orientation_pitch)
		_io.orientation_pitch = _orientation_pitch_smoother (*_io.input_orientation_pitch, update_dt);
	else
	{
		_io.orientation_pitch = xf::nil;
		_orientation_pitch_smoother.invalidate();
	}

	// Smoothed roll:
	if (_io.input_orientation_roll)
		_io.orientation_roll = _orientation_roll_smoother (*_io.input_orientation_roll, update_dt);
	else
	{
		_io.orientation_roll = xf::nil;
		_orientation_roll_smoother.invalidate();
	}
}


void
NavigationComputer::compute_track()
{
	si::Time update_dt = _track_computer.update_dt();

	Position const& pos_last = *(_positions_accurate_2_times.rbegin() + 0);
	Position const& pos_prev = *(_positions_accurate_2_times.rbegin() + 1);
	Position const& pos_prev_prev = *(_positions_accurate_2_times.rbegin() + 2);

	if (pos_last.valid && pos_prev.valid)
	{
		si::Length distance = xf::haversine_earth (pos_last.lateral_position, pos_prev.lateral_position);

		if (distance > 2.0 * pos_last.lateral_position_stddev)
		{
			si::Length altitude_diff = pos_last.altitude - pos_prev.altitude;
			_io.track_vertical = _track_vertical_smoother (1_rad * std::atan (altitude_diff / distance), update_dt);

			si::Angle initial_true_heading = xf::initial_bearing (pos_last.lateral_position, pos_prev.lateral_position);
			si::Angle true_heading = xf::floored_mod (initial_true_heading + 180_deg, 360_deg);
			_io.track_lateral_true = _track_lateral_true_smoother (true_heading, update_dt);

			if (_io.magnetic_declination)
				_io.track_lateral_magnetic = xf::true_to_magnetic (*_io.track_lateral_true, *_io.magnetic_declination);
			else
				_io.track_lateral_magnetic = xf::nil;
		}
		else
		{
			_io.track_vertical = xf::nil;
			_io.track_lateral_true = xf::nil;
			_io.track_lateral_magnetic = xf::nil;
			_track_vertical_smoother.invalidate();
			_track_lateral_true_smoother.invalidate();
		}
	}
	else
	{
		_track_lateral_true_smoother.reset (*_io.orientation_heading_true);
		_io.track_vertical = xf::nil;
		_io.track_lateral_true = xf::nil;
		_io.track_lateral_magnetic = xf::nil;
	}

	std::optional<si::AngularVelocity> result_rotation_speed;

	if (pos_last.valid && pos_prev.valid && pos_prev_prev.valid)
	{
		si::Length len_from_prev = xf::haversine_earth (pos_prev.lateral_position, pos_last.lateral_position);

		if (len_from_prev >= *_io.position_lateral_stddev)
		{
			si::Time dt = pos_last.time - pos_prev.time;
			si::Angle alpha = -180.0_deg + xf::great_arcs_angle (pos_prev_prev.lateral_position,
																 pos_prev.lateral_position,
																 pos_last.lateral_position);
			// Lateral (parallel to the ground) rotation:
			si::AngularVelocity rotation_speed = alpha / dt;

			if (si::isfinite (rotation_speed))
			{
				rotation_speed = _track_lateral_rotation_smoother (rotation_speed, update_dt);
				result_rotation_speed = xf::clamped<si::AngularVelocity> (rotation_speed, si::convert (-1_Hz), si::convert (+1_Hz));
			}
			else
				_track_lateral_rotation_smoother.invalidate();
		}
	}
	else
		_track_lateral_rotation_smoother.invalidate();

	_io.track_lateral_rotation = result_rotation_speed;
}


void
NavigationComputer::compute_ground_speed()
{
	Position const& pos_last = *(_positions_accurate_2_times.rbegin() + 0);
	Position const& pos_prev = *(_positions_accurate_2_times.rbegin() + 1);

	if (pos_last.valid && pos_prev.valid)
	{
		si::Time update_dt = _ground_speed_computer.update_dt();

		si::Time dt = pos_last.time - pos_prev.time;
		si::Length dl = xf::haversine_earth (pos_last.lateral_position, pos_prev.lateral_position);
		_io.track_ground_speed = _track_ground_speed_smoother (dl / dt, update_dt);
	}
	else
	{
		_io.track_ground_speed = xf::nil;
		_track_ground_speed_smoother.invalidate();
	}
}


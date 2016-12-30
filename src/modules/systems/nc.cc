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

// Standard:
#include <cstddef>

// Qt:
#include <QtCore/QDate>
#include <QtCore/QDateTime>
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/config/exception.h>
#include <xefis/support/navigation/magnetic_variation.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/time_helper.h>

// Local:
#include "nc.h"


XEFIS_REGISTER_MODULE_CLASS ("systems/nc", NavigationComputer)


NavigationComputer::NavigationComputer (xf::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config),
	_positions (3),
	_positions_accurate_2_times (3),
	_positions_accurate_9_times (3)
{
	_track_lateral_true_smoother.set_winding ({ 0.0, 360.0 });
	_orientation_heading_magnetic_smoother.set_winding ({ 0.0, 360.0 });
	_orientation_pitch_smoother.set_winding ({ -180.0, 180.0 });
	_orientation_roll_smoother.set_winding ({ -180.0, 180.0 });

	// Initialize _positions* with invalid vals, to get them non-empty:
	for (Positions* positions: { &_positions, &_positions_accurate_2_times, &_positions_accurate_9_times })
		for (std::size_t i = 0; i < positions->capacity(); ++i)
			positions->push_back (Position());

	parse_properties (config, {
		// Input:
		{ "position.input.longitude", _position_input_longitude, true },
		{ "position.input.latitude", _position_input_latitude, true },
		{ "position.input.altitude.amsl", _position_input_altitude_amsl, true },
		{ "position.input.lateral.stddev", _position_input_lateral_stddev, true },
		{ "position.input.vertical.stddev", _position_input_vertical_stddev, true },
		{ "position.input.source", _position_input_source, true },
		{ "orientation.input.pitch", _orientation_input_pitch, true },
		{ "orientation.input.roll", _orientation_input_roll, true },
		{ "orientation.input.heading.magnetic", _orientation_input_heading_magnetic, true },
		// Output:
		{ "position.longitude", _position_longitude, true },
		{ "position.latitude", _position_latitude, true },
		{ "position.altitude.amsl", _position_altitude_amsl, true },
		{ "position.lateral.stddev", _position_lateral_stddev, true },
		{ "position.vertical.stddev", _position_vertical_stddev, true },
		{ "position.stddev", _position_stddev, true },
		{ "position.source", _position_source, true },
		{ "orientation.pitch", _orientation_pitch, true },
		{ "orientation.roll", _orientation_roll, true },
		{ "orientation.heading.magnetic", _orientation_heading_magnetic, true },
		{ "orientation.heading.true", _orientation_heading_true, true },
		{ "track.vertical", _track_vertical, true },
		{ "track.lateral.magnetic", _track_lateral_magnetic, true },
		{ "track.lateral.true", _track_lateral_true, true },
		{ "track.lateral.rotation", _track_lateral_rotation, true },
		{ "track.ground-speed", _track_ground_speed, true },
		{ "magnetic.declination", _magnetic_declination, true },
		{ "magnetic.inclination", _magnetic_inclination, true },
	});

	_position_computer.set_callback (std::bind (&NavigationComputer::compute_position, this));
	_position_computer.observe ({
		&_position_input_longitude,
		&_position_input_latitude,
		&_position_input_altitude_amsl,
		&_position_input_lateral_stddev,
		&_position_input_vertical_stddev,
		&_position_input_source,
	});

	_magnetic_variation_computer.set_callback (std::bind (&NavigationComputer::compute_magnetic_variation, this));
	_magnetic_variation_computer.observe ({
		&_position_longitude,
		&_position_latitude,
		&_position_altitude_amsl
	});

	_headings_computer.set_callback (std::bind (&NavigationComputer::compute_headings, this));
	_headings_computer.add_depending_smoothers ({
		&_orientation_heading_magnetic_smoother,
		&_orientation_pitch_smoother,
		&_orientation_roll_smoother,
	});
	_headings_computer.observe ({
		&_orientation_input_heading_magnetic,
		&_orientation_input_pitch,
		&_orientation_input_roll,
		&_magnetic_declination,
	});

	_track_computer.set_callback (std::bind (&NavigationComputer::compute_track, this));
	_track_computer.add_depending_smoothers ({
		&_track_vertical_smoother,
		&_track_lateral_true_smoother,
		&_track_lateral_rotation_smoother,
	});
	_track_computer.observe ({
		&_position_computer,
		&_magnetic_declination,
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
NavigationComputer::data_updated()
{
	xf::PropertyObserver* computers[] = {
		// Order is important:
		&_position_computer,
		&_magnetic_variation_computer,
		&_headings_computer,
		&_track_computer,
		&_ground_speed_computer,
	};

	for (xf::PropertyObserver* o: computers)
		o->data_updated (update_time());
}


void
NavigationComputer::compute_position()
{
	Time update_time = _position_computer.update_time();

	_position_longitude.copy_from (_position_input_longitude);
	_position_latitude.copy_from (_position_input_latitude);
	_position_altitude_amsl.copy_from (_position_input_altitude_amsl);
	_position_lateral_stddev.copy_from (_position_input_lateral_stddev);
	_position_vertical_stddev.copy_from (_position_input_vertical_stddev);
	_position_source.copy_from (_position_input_source);

	// Larger of the two:
	if (_position_lateral_stddev.valid() && _position_vertical_stddev.valid())
		_position_stddev = std::max (*_position_lateral_stddev, *_position_vertical_stddev);
	else
		_position_stddev.set_nil();

	Length const failed_accuracy = 100_nmi;

	Position position;
	position.lateral_position = LonLat (*_position_longitude, *_position_latitude);
	position.lateral_position_stddev = _position_lateral_stddev.read (failed_accuracy);
	position.altitude = _position_altitude_amsl.read (0.0_ft);
	position.altitude_stddev = _position_vertical_stddev.read (failed_accuracy);
	position.time = update_time;
	position.valid = _position_longitude.valid() &&
					 _position_latitude.valid() &&
					 _position_altitude_amsl.valid() &&
					 _position_lateral_stddev.valid() &&
					 _position_vertical_stddev.valid();
	_positions.push_back (position);

	// Delayed positioning (after enough distance has been traveled):
	if (_positions.back().valid)
	{
		auto add_accurate_position = [&](Positions& accurate_positions, float accuracy_times, Time max_time_difference) -> void
		{
			Position const& new_position = _positions.back();
			Length worse_accuracy = std::max (new_position.lateral_position_stddev, accurate_positions.back().lateral_position_stddev);

			if (!accurate_positions.back().valid ||
				new_position.lateral_position.haversine_earth (accurate_positions.back().lateral_position) > accuracy_times * worse_accuracy ||
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
	if (_position_longitude.valid() && _position_latitude.valid())
	{
		xf::MagneticVariation mv;
		mv.set_position (LonLat (*_position_longitude, *_position_latitude));
		if (_position_altitude_amsl.valid())
			mv.set_altitude_amsl (*_position_altitude_amsl);
		else
			mv.set_altitude_amsl (0_ft);
		QDate today = QDateTime::fromTime_t (xf::TimeHelper::now().quantity<Second>()).date();
		mv.set_date (today.year(), today.month(), today.day());
		mv.update();
		_magnetic_declination.write (mv.magnetic_declination());
		_magnetic_inclination.write (mv.magnetic_inclination());
	}
	else
	{
		_magnetic_declination.set_nil();
		_magnetic_inclination.set_nil();
	}
}


void
NavigationComputer::compute_headings()
{
	Time update_dt = _headings_computer.update_dt();

	if (_orientation_input_heading_magnetic.valid())
	{
		_orientation_heading_magnetic.write (1_deg * _orientation_heading_magnetic_smoother.process ((*_orientation_input_heading_magnetic).quantity<Degree>(), update_dt));

		if (_magnetic_declination.valid())
			_orientation_heading_true.write (xf::magnetic_to_true (*_orientation_heading_magnetic, *_magnetic_declination));
		else
			_orientation_heading_true.set_nil();
	}
	else
	{
		_orientation_heading_magnetic.set_nil();
		_orientation_heading_true.set_nil();
		_orientation_heading_magnetic_smoother.invalidate();
	}

	// Smoothed pitch:
	if (_orientation_input_pitch.valid())
		_orientation_pitch.write (1_deg * _orientation_pitch_smoother.process ((*_orientation_input_pitch).quantity<Degree>(), update_dt));
	else
	{
		_orientation_pitch.set_nil();
		_orientation_pitch_smoother.invalidate();
	}

	// Smoothed roll:
	if (_orientation_input_roll.valid())
		_orientation_roll.write (1_deg * _orientation_roll_smoother.process ((*_orientation_input_roll).quantity<Degree>(), update_dt));
	else
	{
		_orientation_roll.set_nil();
		_orientation_roll_smoother.invalidate();
	}
}


void
NavigationComputer::compute_track()
{
	Time update_dt = _track_computer.update_dt();

	Position const& pos_last = *(_positions_accurate_2_times.rbegin() + 0);
	Position const& pos_prev = *(_positions_accurate_2_times.rbegin() + 1);
	Position const& pos_prev_prev = *(_positions_accurate_2_times.rbegin() + 2);

	if (pos_last.valid && pos_prev.valid)
	{
		Length distance = pos_last.lateral_position.haversine_earth (pos_prev.lateral_position);
		if (distance > 2.0 * pos_last.lateral_position_stddev)
		{
			Length altitude_diff = pos_last.altitude - pos_prev.altitude;
			_track_vertical.write (1_rad * _track_vertical_smoother.process (std::atan (altitude_diff / distance), update_dt));

			Angle initial_true_heading = pos_last.lateral_position.initial_bearing (pos_prev.lateral_position);
			Angle true_heading = xf::floored_mod (initial_true_heading + 180_deg, 360_deg);
			_track_lateral_true.write (1_deg * _track_lateral_true_smoother.process (true_heading.quantity<Degree>(), update_dt));

			if (_magnetic_declination.valid())
				_track_lateral_magnetic.write (xf::true_to_magnetic (*_track_lateral_true, *_magnetic_declination));
			else
				_track_lateral_magnetic.set_nil();
		}
		else
		{
			_track_vertical.set_nil();
			_track_lateral_true.set_nil();
			_track_lateral_magnetic.set_nil();
			_track_vertical_smoother.invalidate();
			_track_lateral_true_smoother.invalidate();
		}
	}
	else
	{
		_track_lateral_true_smoother.reset ((*_orientation_heading_true).quantity<Degree>());
		_track_vertical.set_nil();
		_track_lateral_true.set_nil();
		_track_lateral_magnetic.set_nil();
	}

	Optional<AngularVelocity> result_rotation_speed;
	if (pos_last.valid && pos_prev.valid && pos_prev_prev.valid)
	{
		Length len_from_prev = pos_prev.lateral_position.haversine_earth (pos_last.lateral_position);

		if (len_from_prev >= *_position_lateral_stddev)
		{
			using std::isinf;
			using std::isnan;

			Time dt = pos_last.time - pos_prev.time;
			Angle alpha = -180.0_deg + LonLat::great_arcs_angle (pos_prev_prev.lateral_position,
																 pos_prev.lateral_position,
																 pos_last.lateral_position);
			// Lateral (parallel to the ground) rotation:
			AngularVelocity rotation_speed = alpha / dt;

			if (!isinf (rotation_speed) && !isnan (rotation_speed))
			{
				rotation_speed = 1_radps * _track_lateral_rotation_smoother.process (rotation_speed.quantity<RadianPerSecond>(), update_dt);
				result_rotation_speed = xf::clamped<AngularVelocity> (rotation_speed, convert (-1_Hz), convert (+1_Hz));
			}
			else
				_track_lateral_rotation_smoother.invalidate();
		}
	}
	else
		_track_lateral_rotation_smoother.invalidate();
	_track_lateral_rotation.write (result_rotation_speed);
}


void
NavigationComputer::compute_ground_speed()
{
	Position const& pos_last = *(_positions_accurate_2_times.rbegin() + 0);
	Position const& pos_prev = *(_positions_accurate_2_times.rbegin() + 1);

	if (pos_last.valid && pos_prev.valid)
	{
		Time update_dt = _ground_speed_computer.update_dt();

		Time dt = pos_last.time - pos_prev.time;
		Length dl = pos_last.lateral_position.haversine_earth (pos_prev.lateral_position);
		_track_ground_speed.write (1_kt * _track_ground_speed_smoother.process ((dl / dt).quantity<Knot>(), update_dt));
	}
	else
	{
		_track_ground_speed.set_nil();
		_track_ground_speed_smoother.invalidate();
	}
}


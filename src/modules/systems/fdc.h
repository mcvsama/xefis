/* vim:ts=4
 *
 * Copyleft 2012…2013  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__MODULES__SYSTEMS__FDC_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__FDC_H__INCLUDED

// Standard:
#include <cstddef>
#include <array>

// Qt:
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/property.h>
#include <xefis/utility/smoother.h>
#include <xefis/utility/lookahead.h>


/**
 * Computations are reliable up to 36,000 ft.
 */
class FlightDataComputer: public Xefis::Module
{
	struct Position
	{
		LonLat		lateral_position;
		Length		altitude;
		Length		accuracy;
		bool		valid = false;
		Time		time;
	};

  public:
	// Ctor
	FlightDataComputer (Xefis::ModuleManager*, QDomElement const& config);

  private:
	void
	data_updated() override;

	void
	compute_position();

	void
	compute_headings();

	void
	compute_track();

	void
	compute_da();

	void
	compute_speeds();

	void
	compute_aoa();

	void
	compute_fpm();

	void
	compute_speed_limits();

	void
	compute_wind();

	void
	compute_performance();

  private:
	Time						_now;
	// [0] - new, [2] - old
	std::array<Position, 3>		_positions;
	std::array<Position, 3>		_ac1_positions;
	std::array<Position, 3>		_ac2_positions;
	Length						_alt_amsl_prev						= 0_ft;
	Time						_alt_amsl_time						= 0_s;
	Speed						_computed_climb_rate				= 0_fpm;
	int							_positions_valid					= 0;
	Xefis::Smoother<double>		_track_vertical_smoother			= 50.0; // TODO make fps independent
	Xefis::Smoother<double>		_track_lateral_true_smoother		= 100.0; // TODO make fps independent
	Xefis::Smoother<double>		_wind_direction_smoother			= 2000.0; // TODO make fps independent
	Xefis::Smoother<double>		_ground_speed_smoother				= 200.0; // TODO make fps independent
	Xefis::Smoother<double>		_climb_rate_smoother				= 400.0; // TODO make fps independent
	Xefis::Smoother<double>		_pressure_alt_smoother				= 100.0; // TODO make fps independent
	Xefis::Lookahead<double>	_pressure_alt_estimator				= Xefis::Lookahead<double> (10_s);
	Xefis::Lookahead<double>	_ias_estimator						= Xefis::Lookahead<double> (10_s);
	Xefis::Smoother<double>		_pressure_alt_lookahead_smoother	= 1000.0; // TODO make fps independent
	Xefis::Smoother<double>		_ias_lookahead_smoother				= 1000.0; // TODO make fps independent

	// Input parameters:
	Xefis::PropertyFloat		_default_airplane_weight_kg; // TODO
	Xefis::PropertyFloat		_actual_airplane_weight_kg; // TODO
	Xefis::PropertyAngle		_low_speed_roll_angle; // TODO
	Xefis::PropertySpeed		_v_a_default; // TODO
	Xefis::PropertySpeed		_v_ne;
	Xefis::PropertySpeed		_v_s;
	Xefis::PropertySpeed		_v_s0;
	Xefis::PropertySpeed		_v_at; // TODO
	Xefis::PropertySpeed		_v_fe; // TODO //settings/flaps/angle
	Xefis::PropertySpeed		_v_le; // TODO //settings/gear/lowered
	Xefis::PropertySpeed		_v_o;
	Xefis::PropertySpeed		_v_be; // TODO
	Xefis::PropertySpeed		_v_br; // TODO
	Xefis::PropertySpeed		_v_bg; // TODO
	Xefis::PropertyBoolean		_use_standard_pressure;
	Xefis::PropertyBoolean		_gear_down; // TODO
	Xefis::PropertyPressure		_static_pressure;
	Xefis::PropertyPressure		_qnh_pressure;
	Xefis::PropertyAngle		_critical_aoa;
	Xefis::PropertySpeed		_ias;
	Xefis::PropertyFloat		_outside_air_temperature_k;
	// Input IMU:
	Xefis::PropertyAngle		_imu_pitch;
	Xefis::PropertyAngle		_imu_roll;
	Xefis::PropertyAngle		_imu_magnetic_heading;
	Xefis::PropertyAngle		_imu_magnetic_heading_accuracy; // TODO
	// Input GPS:
	Xefis::PropertyAngle		_gps_longitude;
	Xefis::PropertyAngle		_gps_latitude;
	Xefis::PropertyLength		_gps_altitude_amsl;
	Xefis::PropertyLength		_gps_accuracy;
	Xefis::PropertyTime			_gps_timestamp;
	// Input INS (Inertial Navigation System):
	Xefis::PropertyAngle		_ins_longitude;
	Xefis::PropertyAngle		_ins_latitude;
	Xefis::PropertyLength		_ins_altitude_amsl;
	Xefis::PropertyLength		_ins_accuracy;
	Xefis::PropertyTime			_ins_timestamp;

	// Output position:
	Xefis::PropertyAngle		_position_longitude;
	Xefis::PropertyAngle		_position_latitude;
	Xefis::PropertyLength		_position_altitude_amsl;
	Xefis::PropertyLength		_position_accuracy;
	Xefis::PropertyString		_position_source;
	// Output track (flight path):
	Xefis::PropertyAngle		_track_vertical;
	Xefis::PropertyAngle		_track_lateral_true;
	Xefis::PropertyAngle		_track_lateral_magnetic;
	Xefis::PropertyFloat		_track_vertical_delta_dpf; // TODO
	Xefis::PropertyFloat		_track_heading_delta_dpf; // TODO
	// Output orientation:
	Xefis::PropertyAngle		_orientation_pitch;
	Xefis::PropertyAngle		_orientation_roll;
	Xefis::PropertyAngle		_orientation_true_heading;
	Xefis::PropertyAngle		_orientation_magnetic_heading;
	// Output altitude:
	Xefis::PropertyLength		_pressure_altitude_amsl;
	Xefis::PropertyLength		_pressure_altitude_amsl_lookahead;
	Xefis::PropertySpeed		_pressure_altitude_climb_rate;
	// Output speeds:
	Xefis::PropertySpeed		_v_a; // TODO
	Xefis::PropertySpeed		_minimum_ias;
	Xefis::PropertySpeed		_minimum_maneuver_ias;
	Xefis::PropertySpeed		_maximum_ias;
	Xefis::PropertySpeed		_maximum_maneuver_ias;
	Xefis::PropertySpeed		_ias_lookahead;
	Xefis::PropertySpeed		_true_airspeed;
	Xefis::PropertySpeed		_ground_speed;
	Xefis::PropertyFloat		_mach;
	Xefis::PropertySpeed		_sound_speed;
	// Output FPM:
	Xefis::PropertyAngle		_fpm_alpha;
	Xefis::PropertyAngle		_fpm_beta;
	// Output AOA:
	Xefis::PropertyAngle		_pitch_limit;
	Xefis::PropertyAngle		_aoa_alpha;
	Xefis::PropertyAngle		_aoa_beta;
	// Output wind:
	Xefis::PropertyAngle		_wind_true_orientation_from;
	Xefis::PropertyAngle		_wind_magnetic_orientation_from;
	Xefis::PropertySpeed		_wind_tas;
	// Output approach:
	Xefis::PropertyAngle		_localizer_vertical_deviation; // TODO
	Xefis::PropertyAngle		_localizer_lateral_deviation; // TODO
	Xefis::PropertyString		_localizer_identifier; // TODO
	Xefis::PropertyString		_localizer_source; // TODO
	Xefis::PropertyLength		_localizer_distance; // TODO
	Xefis::PropertyFrequency	_localizer_frequency; // TODO
	// Output other:
	Xefis::PropertyFloat		_climb_glide_ratio;
	Xefis::PropertyAngle		_magnetic_declination;
	Xefis::PropertyAngle		_magnetic_inclination;
	Xefis::PropertyLength		_density_altitude;
};

#endif

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

#ifndef XEFIS__MODULES__SYSTEMS__FMS_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__FMS_H__INCLUDED

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


/**
 * Computations are reliable up to 36,000 ft.
 */
class FlightManagementSystem: public Xefis::Module
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
	FlightManagementSystem (Xefis::ModuleManager*, QDomElement const& config);

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
	compute_wind();

	void
	compute_performance();

  private:
	Time					_now;
	// [0] - new, [2] - old
	std::array<Position, 3>	_positions;
	std::array<Position, 3>	_ac1_positions;
	std::array<Position, 3>	_ac2_positions;
	Length					_alt_amsl_prev						= 0_ft;
	Time					_alt_amsl_time						= 0_s;
	Speed					_computed_climb_rate				= 0_fpm;
	int						_positions_valid					= 0;
	Xefis::Smoother<double>	_track_vertical_smoother			= 50.0; // TODO make fps independent
	Xefis::Smoother<double>	_track_true_heading_smoother		= 100.0; // TODO make fps independent
	Xefis::Smoother<double>	_wind_direction_smoother			= 2000.0; // TODO make fps independent
	Xefis::Smoother<double>	_ground_speed_smoother				= 200.0; // TODO make fps independent
	Xefis::Smoother<double>	_climb_rate_smoother				= 400.0; // TODO make fps independent
	Xefis::Smoother<double>	_pressure_alt_smoother				= 100.0; // TODO make fps independent

	// Input parameters:
	Xefis::PropertyFloat	_default_airplane_weight_kg;
	Xefis::PropertyFloat	_actual_airplane_weight_kg;
	Xefis::PropertyFloat	_low_speed_roll_angle_deg;
	Xefis::PropertyFloat	_v_a_default_kt;
	Xefis::PropertyFloat	_v_no_default_kt;
	Xefis::PropertyFloat	_v_ne_kt;
	Xefis::PropertyFloat	_v_s_kt;
	Xefis::PropertyFloat	_v_s0_kt;
	Xefis::PropertyFloat	_v_at_kt;
	Xefis::PropertyFloat	_v_fe_kt;
	Xefis::PropertyFloat	_v_le_kt;
	Xefis::PropertyFloat	_v_o_kt;
	Xefis::PropertyFloat	_v_be_kt;
	Xefis::PropertyFloat	_v_bg_kt;
	Xefis::PropertyFloat	_v_br_kt;
	Xefis::PropertyString	_flaps_configuration_properties_path;
	Xefis::PropertyBoolean	_gear_down;
	Xefis::PropertyFloat	_static_pressure_inhg;
	Xefis::PropertyFloat	_qnh_pressure_inhg;
	Xefis::PropertyFloat	_backup_amsl_ft;
	Xefis::PropertyFloat	_ias_kt;
	Xefis::PropertyFloat	_outside_air_temperature_k;
	// Input IMU:
	Xefis::PropertyFloat	_imu_pitch_deg;
	Xefis::PropertyFloat	_imu_roll_deg;
	Xefis::PropertyFloat	_imu_magnetic_heading_deg;
	Xefis::PropertyFloat	_imu_magnetic_heading_accuracy_deg;
	// Input GPS:
	Xefis::PropertyFloat	_gps_longitude_deg;
	Xefis::PropertyFloat	_gps_latitude_deg;
	Xefis::PropertyFloat	_gps_altitude_amsl_ft;
	Xefis::PropertyFloat	_gps_accuracy_nm;
	Xefis::PropertyFloat	_gps_timestamp_s;
	// Input INS (Inertial Navigation System):
	Xefis::PropertyFloat	_ins_longitude_deg;
	Xefis::PropertyFloat	_ins_latitude_deg;
	Xefis::PropertyFloat	_ins_altitude_amsl_ft;
	Xefis::PropertyFloat	_ins_accuracy_nm;
	Xefis::PropertyFloat	_ins_timestamp_s;

	// Output position:
	Xefis::PropertyFloat	_position_longitude_deg;
	Xefis::PropertyFloat	_position_latitude_deg;
	Xefis::PropertyFloat	_position_altitude_amsl_ft;
	Xefis::PropertyFloat	_position_accuracy_nm;
	Xefis::PropertyString	_position_source;
	// Output track (flight path):
	Xefis::PropertyFloat	_track_vertical_deg;
	Xefis::PropertyFloat	_track_true_heading_deg;
	Xefis::PropertyFloat	_track_magnetic_heading_deg;
	Xefis::PropertyFloat	_track_vertical_delta_dpf;
	Xefis::PropertyFloat	_track_heading_delta_dpf;
	// Output orientation:
	Xefis::PropertyFloat	_orientation_pitch_deg;
	Xefis::PropertyFloat	_orientation_roll_deg;
	Xefis::PropertyFloat	_orientation_true_heading_deg;
	Xefis::PropertyFloat	_orientation_magnetic_heading_deg;
	// Output altitude:
	Xefis::PropertyFloat	_pressure_altitude_amsl_ft;
	Xefis::PropertyFloat	_pressure_altitude_amsl_lookahead_ft;
	Xefis::PropertyFloat	_pressure_altitude_amsl_time_s;
	Xefis::PropertyFloat	_pressure_altitude_climb_rate_fpm;
	// Output speeds:
	Xefis::PropertyFloat	_v_r_kt;
	Xefis::PropertyFloat	_v_ref_kt;
	Xefis::PropertyFloat	_v_a_kt;
	Xefis::PropertyFloat	_v_no_kt;
	Xefis::PropertyFloat	_minimum_ias_kt;
	Xefis::PropertyFloat	_minimum_maneuver_ias_kt;
	Xefis::PropertyFloat	_maximum_ias_kt;
	Xefis::PropertyFloat	_maximum_maneuver_ias_kt;
	Xefis::PropertyFloat	_ias_lookahead_kt;
	Xefis::PropertyFloat	_ias_lookahead_time_s;
	Xefis::PropertyFloat	_true_airspeed_kt;
	Xefis::PropertyFloat	_ground_speed_kt;
	Xefis::PropertyFloat	_mach;
	Xefis::PropertyFloat	_sound_speed_kt;
	// Output AOA:
	Xefis::PropertyFloat	_relative_pitch_limit_deg;
	Xefis::PropertyFloat	_aoa_alpha_deg;
	Xefis::PropertyFloat	_aoa_beta_deg;
	// Output wind:
	Xefis::PropertyFloat	_wind_true_orientation_from_deg;
	Xefis::PropertyFloat	_wind_magnetic_orientation_from_deg;
	Xefis::PropertyFloat	_wind_tas_kt;
	// Output approach:
	Xefis::PropertyFloat	_localizer_vertical_deviation_deg;
	Xefis::PropertyFloat	_localizer_lateral_deviation_deg;
	Xefis::PropertyFloat	_localizer_identifier;
	Xefis::PropertyFloat	_localizer_source;
	Xefis::PropertyFloat	_localizer_distance_nm;
	Xefis::PropertyFloat	_localizer_frequency_hz;
	// Output other:
	Xefis::PropertyFloat	_climb_glide_ratio;
	Xefis::PropertyFloat	_magnetic_declination_deg;
	Xefis::PropertyFloat	_magnetic_inclination_deg;
	Xefis::PropertyFloat	_density_altitude_ft;
};

#endif

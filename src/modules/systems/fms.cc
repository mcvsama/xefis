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

// Standard:
#include <cstddef>

// Qt:
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/config/exception.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/numeric.h>
#include <xefis/utility/density_altitude.h>
#include <xefis/utility/sound_speed.h>
#include <xefis/utility/wind_triangle.h>
#include <xefis/utility/magnetic_variation.h>

// Local:
#include "fms.h"


FlightManagementSystem::FlightManagementSystem (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager)
{
	_track_true_heading_smoother.set_winding ({ 0.0, 360.0 });
	_wind_direction_smoother.set_winding ({ 0.0, 360.0 });

	for (QDomElement& e: config)
	{
		if (e == "properties")
		{
			parse_properties (e, {
				// Input:
				{ "settings.default-airplane-weight", _default_airplane_weight_kg, true },
				{ "settings.actual-airplane-weight", _actual_airplane_weight_kg, true },
				{ "settings.low-speed-roll-angle", _low_speed_roll_angle_deg, true },
				{ "settings.speed.v-a-default", _v_a_default_kt, true },
				{ "settings.speed.v-no-default", _v_no_default_kt, true },
				{ "settings.speed.v-ne", _v_ne_kt, true },
				{ "settings.speed.v-s", _v_s_kt, true },
				{ "settings.speed.v-s0", _v_s0_kt, true },
				{ "settings.speed.v-at", _v_at_kt, true },
				{ "settings.speed.v-fe", _v_fe_kt, true },
				{ "settings.speed.v-le", _v_le_kt, true },
				{ "settings.speed.v-o", _v_o_kt, true },
				{ "settings.speed.v-be", _v_be_kt, true },
				{ "settings.speed.v-bg", _v_bg_kt, true },
				{ "settings.speed.v-br", _v_br_kt, true },
				{ "settings.flaps-configuration-properties-path", _flaps_configuration_properties_path, true },
				{ "imu.pitch", _imu_pitch_deg, true },
				{ "imu.roll", _imu_roll_deg, true },
				{ "imu.magnetic-heading", _imu_magnetic_heading_deg, true },
				{ "imu.magnetic-heading-accuracy", _imu_magnetic_heading_accuracy_deg, true },
				{ "gps.longitude", _gps_longitude_deg, true },
				{ "gps.latitude", _gps_latitude_deg, true },
				{ "gps.altitude-amsl", _gps_altitude_amsl_ft, true },
				{ "gps.accuracy", _gps_accuracy_nm, true },
				{ "gps.timestamp", _gps_timestamp_s, true },
				{ "ins.longitude", _ins_longitude_deg, true },
				{ "ins.latitude", _ins_latitude_deg, true },
				{ "ins.altitude-amsl", _ins_altitude_amsl_ft, true },
				{ "ins.accuracy", _ins_accuracy_nm, true },
				{ "ins.timestamp", _ins_timestamp_s, true },
				{ "pressure.static", _static_pressure_inhg, true },
				{ "pressure.qnh", _qnh_pressure_inhg, true },
				{ "backup-amsl", _backup_amsl_ft, true },
				{ "gear-down", _gear_down, true },
				{ "ias", _ias_kt, true },
				{ "outside-air-temperature", _outside_air_temperature_k, true },
				// Output:
				{ "position.longitude", _position_longitude_deg, true },
				{ "position.latitude", _position_latitude_deg, true },
				{ "position.altitude-amsl", _position_altitude_amsl_ft, true },
				{ "position.accuracy", _position_accuracy_nm, true },
				{ "position.source", _position_source, true },
				{ "track.vertical", _track_vertical_deg, true },
				{ "track.true-heading", _track_true_heading_deg, true },
				{ "track.magnetic-heading", _track_magnetic_heading_deg, true },
				{ "track.vertical-delta", _track_vertical_delta_dpf, true },
				{ "track.heading-delta", _track_heading_delta_dpf, true },
				{ "orientation.pitch", _orientation_pitch_deg, true },
				{ "orientation.roll", _orientation_roll_deg, true },
				{ "orientation.true-heading", _orientation_true_heading_deg, true },
				{ "orientation.magnetic-heading", _orientation_magnetic_heading_deg, true },
				{ "pressure-altitude.amsl", _pressure_altitude_amsl_ft, true },
				{ "pressure-altitude.amsl-lookahead", _pressure_altitude_amsl_lookahead_ft, true },
				{ "pressure-altitude.amsl-lookahead-time", _pressure_altitude_amsl_time_s, true },
				{ "pressure-altitude.climb-rate", _pressure_altitude_climb_rate_fpm, true },
				{ "speed.v-r", _v_r_kt, true },
				{ "speed.v-ref", _v_ref_kt, true },
				{ "speed.v-a", _v_a_kt, true },
				{ "speed.v-no", _v_no_kt, true },
				{ "speed.minimum-ias", _minimum_ias_kt, true },
				{ "speed.minimum-maneuver-ias", _minimum_maneuver_ias_kt, true },
				{ "speed.maximum-ias", _maximum_ias_kt, true },
				{ "speed.maximum-maneuver-ias", _maximum_maneuver_ias_kt, true },
				{ "speed.ias-lookahead", _ias_lookahead_kt, true },
				{ "speed.ias-lookahead-time", _ias_lookahead_time_s, true },
				{ "speed.true-airspeed", _true_airspeed_kt, true },
				{ "speed.ground-speed", _ground_speed_kt, true },
				{ "speed.mach", _mach, true },
				{ "speed.sound", _sound_speed_kt, true },
				{ "aoa.relative-pitch-limit", _relative_pitch_limit_deg, true },
				{ "aoa.alpha", _aoa_alpha_deg, true },
				{ "aoa.beta", _aoa_beta_deg, true },
				{ "wind.true-heading", _wind_true_orientation_from_deg, true },
				{ "wind.magnetic-heading", _wind_magnetic_orientation_from_deg, true },
				{ "wind.true-airspeed", _wind_tas_kt, true },
				{ "localizer.vertical-deviation", _localizer_vertical_deviation_deg, true },
				{ "localizer.lateral-deviation", _localizer_lateral_deviation_deg, true },
				{ "localizer.identifier", _localizer_identifier, true },
				{ "localizer.source", _localizer_source, true },
				{ "localizer.distance", _localizer_distance_nm, true },
				{ "localizer.frequency", _localizer_frequency_hz, true },
				{ "performance.climb-glide-ratio", _climb_glide_ratio, true },
				{ "magnetic-declination", _magnetic_declination_deg, true },
				{ "magnetic-inclination", _magnetic_inclination_deg, true },
				{ "density-altitude", _density_altitude_ft, true }
			});
		}
	}
}


void
FlightManagementSystem::data_updated()
{
	_now = Time::now();

	compute_position();
	compute_headings();
	compute_track();
	compute_da();
	compute_speeds();
	compute_aoa();
	compute_wind();
	compute_performance();
}


void
FlightManagementSystem::compute_position()
{
	enum PositionSource { GPS, INS };

	PositionSource source;
	Length accuracy = 100_nm;

	// XXX remove when GPS module is done.
	_gps_accuracy_nm.write (0.001);

	if (_gps_longitude_deg.valid() && _gps_latitude_deg.valid() && _gps_altitude_amsl_ft.valid() && _gps_accuracy_nm.valid())
	{
		if (*_gps_accuracy_nm < accuracy.nm())
		{
			source = GPS;
			accuracy = 1_nm * *_gps_accuracy_nm;
		}
	}

	if (_ins_longitude_deg.valid() && _ins_latitude_deg.valid() && _ins_altitude_amsl_ft.valid() && _ins_accuracy_nm.valid())
	{
		if (*_ins_accuracy_nm < accuracy.nm())
		{
			source = INS;
			accuracy = 1_nm * *_ins_accuracy_nm;
		}
	}

	switch (source)
	{
		case GPS:
			_position_longitude_deg.copy (_gps_longitude_deg);
			_position_latitude_deg.copy (_gps_latitude_deg);
			_position_altitude_amsl_ft.copy (_gps_altitude_amsl_ft);
			_position_accuracy_nm.copy (_gps_accuracy_nm);
			_position_source.write ("GPS");
			break;

		case INS:
			_position_longitude_deg.copy (_ins_longitude_deg);
			_position_latitude_deg.copy (_ins_latitude_deg);
			_position_altitude_amsl_ft.copy (_ins_altitude_amsl_ft);
			_position_accuracy_nm.copy (_ins_accuracy_nm);
			_position_source.write ("INERTIAL");
			break;
	}

	// Positions history:
	_positions[1] = _positions[0];
	_positions[0].lateral_position = LonLat (_position_longitude_deg.read (0.0) * 1_deg,
											 _position_latitude_deg.read (0.0) * 1_deg);
	_positions[0].altitude = _position_altitude_amsl_ft.read (0.0) * 1_ft;
	_positions[0].accuracy = _position_accuracy_nm.read (0.0) * 1_nm;
	_positions[0].valid = _position_longitude_deg.valid() && _position_latitude_deg.valid() &&
						  _position_altitude_amsl_ft.valid() && _position_accuracy_nm.valid();
	_positions[0].time = _now;

	// Delayed positioning:
	if (_positions[0].valid)
	{
		Length accuracy1 = std::max (_positions[0].accuracy, _ac1_positions[0].accuracy);
		if (!_ac1_positions[0].valid ||
			_positions[0].lateral_position.haversine_earth (_ac1_positions[0].lateral_position) > 2.0 * accuracy1 ||
			_positions[0].time - _ac1_positions[0].time > 1_s)
		{
			_ac1_positions[2] = _ac1_positions[1];
			_ac1_positions[1] = _ac1_positions[0];
			_ac1_positions[0] = _positions[0];
		}

		Length accuracy2 = std::max (_positions[0].accuracy, _ac2_positions[0].accuracy);
		if (!_ac2_positions[0].valid ||
			_positions[0].lateral_position.haversine_earth (_ac2_positions[0].lateral_position) > 20.0 * accuracy2 ||
			_positions[0].time - _ac2_positions[0].time > 10_s)
		{
			_ac2_positions[2] = _ac2_positions[1];
			_ac2_positions[1] = _ac2_positions[0];
			_ac2_positions[0] = _positions[0];
		}
	}
	else
	{
		_ac1_positions[0].valid = false;
		_ac2_positions[0].valid = false;
	}

	if (_static_pressure_inhg.valid() && _qnh_pressure_inhg.valid())
	{
		double a = 6.8755856e-6;
		double b = 5.2558797;
		double p = *_static_pressure_inhg;
		double p0 = 29.92126;
		// Good for heights below tropopause (36 kft):
		double h = (1.0 - std::pow (p / p0, 1 / b)) / a;
		double alt_set = *_qnh_pressure_inhg;
		double p_alt_corr = 145442.2 * (1.0 - std::pow (alt_set / p0, 0.190261));
		double p_alt = h + p_alt_corr;

		_pressure_altitude_amsl_ft.write (_pressure_alt_smoother.process (p_alt));
	}
	else
		_pressure_altitude_amsl_ft.copy (_position_altitude_amsl_ft);
}


void
FlightManagementSystem::compute_headings()
{
	if (_position_longitude_deg.valid() && _position_latitude_deg.valid())
	{
		Xefis::MagneticVariation mv;
		mv.set_position (LonLat (1_deg * *_position_longitude_deg, 1_deg * *_position_latitude_deg));
		if (_position_altitude_amsl_ft.valid())
			mv.set_altitude_amsl (1_ft * *_position_altitude_amsl_ft);
		else
			mv.set_altitude_amsl (0_ft);
		mv.set_date (2013, 1, 1);
		mv.update();
		_magnetic_declination_deg.write (mv.magnetic_declination().deg());
		_magnetic_inclination_deg.write (mv.magnetic_inclination().deg());
	}
	else
	{
		_magnetic_declination_deg.set_nil();
		_magnetic_inclination_deg.set_nil();
	}

	if (_imu_magnetic_heading_deg.valid())
	{
		_orientation_magnetic_heading_deg.copy (_imu_magnetic_heading_deg);

		if (_magnetic_declination_deg.valid())
			_orientation_true_heading_deg.write (magnetic_to_true (*_imu_magnetic_heading_deg, *_magnetic_declination_deg));
		else
			_orientation_true_heading_deg.set_nil();
	}
	else
	{
		_orientation_magnetic_heading_deg.set_nil();
		_orientation_true_heading_deg.set_nil();
	}
}


void
FlightManagementSystem::compute_track()
{
	if (_ac1_positions[0].valid && _ac1_positions[1].valid)
	{
		Length distance = _ac1_positions[0].lateral_position.haversine_earth (_ac1_positions[1].lateral_position);
		if (distance > 2.0 * _ac1_positions[0].accuracy)
		{
			Length altitude_diff = _ac1_positions[0].altitude - _ac1_positions[1].altitude;
			_track_vertical_deg.write (_track_vertical_smoother.process ((std::asin (altitude_diff.m() / distance.m()) * 1_rad).deg()));

			Angle initial_true_heading = _ac1_positions[0].lateral_position.initial_bearing (_ac1_positions[1].lateral_position);
			Angle true_heading = 1_deg * floored_mod (initial_true_heading.deg() + 180.0, 360.0);
			_track_true_heading_deg.write (_track_true_heading_smoother.process (true_heading.deg()));

			if (_magnetic_declination_deg.valid())
				_track_magnetic_heading_deg.write (true_to_magnetic (*_track_true_heading_deg, *_magnetic_declination_deg));
		}
		else
		{
			_track_vertical_deg.set_nil();
			_track_true_heading_deg.set_nil();
			_track_magnetic_heading_deg.set_nil();
		}
	}
	else
	{
		_track_true_heading_smoother.reset (*_orientation_magnetic_heading_deg);
		_track_vertical_deg.set_nil();
		_track_true_heading_deg.set_nil();
	}

	if (_ac1_positions[0].valid && _ac1_positions[1].valid && _ac1_positions[2].valid)
	{
		// TODO deltas for trend vector
	}
	else
	{
		_track_vertical_delta_dpf.set_nil();
		_track_heading_delta_dpf.set_nil();
	}
}


void
FlightManagementSystem::compute_da()
{
	if (_outside_air_temperature_k.valid() &&
		_pressure_altitude_amsl_ft.valid())
	{
		Xefis::DensityAltitude da;
		da.set_pressure_altitude (1_ft * *_pressure_altitude_amsl_ft);
		da.set_outside_air_temperature (*_outside_air_temperature_k);
		da.update();
		_density_altitude_ft.write (da.density_altitude().ft());
	}
	else
		_density_altitude_ft.set_nil();
}


void
FlightManagementSystem::compute_speeds()
{
	if (_outside_air_temperature_k.valid())
	{
		Xefis::SoundSpeed ss;
		ss.set_outside_air_temperature (*_outside_air_temperature_k);
		ss.update();
		_sound_speed_kt.write (ss.sound_speed());
	}

	if (_ias_kt.valid() && _pressure_altitude_amsl_ft.valid())
	{
		double ias_kt = *_ias_kt;
		double cas_kt = ias_kt;

		if (_density_altitude_ft.valid())
		{
			double da = *_density_altitude_ft;

			// This does not take into account air compressibility factor, so it's valid
			// for low speeds (mach < 0.3) and altitude below tropopause (36 kft):
			_true_airspeed_kt.write (cas_kt / std::pow (1 - 6.8755856 * 1e-6 * da, 2.127940));
		}
		else
			// Very simple equation for TAS, fix it to use air temperature:
			_true_airspeed_kt.write (cas_kt + 0.02 * cas_kt * *_pressure_altitude_amsl_ft / 1000.0);
	}
	else
		_true_airspeed_kt.set_nil();

	if (_ac2_positions[0].valid && _ac2_positions[1].valid)
	{
		Time dt = _ac2_positions[0].time - _ac2_positions[1].time;
		Length dl = _ac2_positions[0].lateral_position.haversine_earth (_ac2_positions[1].lateral_position);
		_ground_speed_kt.write (_ground_speed_smoother.process ((dl / dt).kt()));
	}
	else
		_ground_speed_kt.set_nil();

	// The approximate speed of sound in dry (0% humidity) air:
	if (_true_airspeed_kt.valid() && _sound_speed_kt.valid())
		_mach.write (*_true_airspeed_kt / *_sound_speed_kt);
	else
		_mach.set_nil();

	// Climb rate:
	if (_pressure_altitude_amsl_ft.valid())
	{
		_alt_amsl_time += update_dt();
		if (_alt_amsl_time > 0.05_s)
		{
			Length alt_diff = 1_ft * *_pressure_altitude_amsl_ft - _alt_amsl_prev;
			_computed_climb_rate = alt_diff / _alt_amsl_time;
			_alt_amsl_time = 0_s;
			_alt_amsl_prev = 1_ft * *_pressure_altitude_amsl_ft;
		}

		_pressure_altitude_climb_rate_fpm.write (_climb_rate_smoother.process (_computed_climb_rate.fpm()));
	}
	else
		_pressure_altitude_climb_rate_fpm.set_nil();
}


void
FlightManagementSystem::compute_aoa()
{
	if (_imu_pitch_deg.valid() && _imu_roll_deg.valid() && _imu_magnetic_heading_deg.valid() &&
		_track_vertical_deg.valid() && _track_magnetic_heading_deg.valid())
	{
		Angle vdiff = 1_deg * floored_mod (*_imu_pitch_deg - *_track_vertical_deg, -180.0, +180.0);
		Angle hdiff = 1_deg * floored_mod (*_imu_magnetic_heading_deg - *_track_magnetic_heading_deg, -180.0, +180.0);
		Angle roll = 1_deg * *_imu_roll_deg;

		Angle alpha = vdiff * std::cos (roll) + hdiff * std::sin (roll);
		Angle beta = -vdiff * std::sin (roll) + hdiff * std::cos (roll);

		_aoa_alpha_deg.write (floored_mod (alpha.deg(), -180.0, +180.0));
		_aoa_beta_deg.write (floored_mod (beta.deg(), -180.0, +180.0));;
	}
	else
	{
		_aoa_alpha_deg.set_nil();
		_aoa_beta_deg.set_nil();
	}
}


void
FlightManagementSystem::compute_wind()
{
	if (_true_airspeed_kt.valid() &&
		_ground_speed_kt.valid() &&
		_track_true_heading_deg.valid() &&
		_orientation_true_heading_deg.valid())
	{
		Xefis::WindTriangle wt;
		wt.set_aircraft_tas (*_true_airspeed_kt);
		wt.set_aircraft_track (1_deg * *_track_true_heading_deg);
		wt.set_aircraft_ground_speed (*_ground_speed_kt);
		wt.set_aircraft_heading (1_deg * *_orientation_true_heading_deg);
		wt.update();
		_wind_true_orientation_from_deg.write (floored_mod (_wind_direction_smoother.process (wt.wind_direction().deg()), 360.0));
		_wind_magnetic_orientation_from_deg.write (true_to_magnetic (*_wind_true_orientation_from_deg, *_magnetic_declination_deg));
		_wind_tas_kt.write (wt.wind_speed());
	}
	else
	{
		_wind_true_orientation_from_deg.set_nil();
		_wind_magnetic_orientation_from_deg.set_nil();
		_wind_tas_kt.set_nil();
	}
}


void
FlightManagementSystem::compute_performance()
{
	if (_true_airspeed_kt.valid() && _pressure_altitude_climb_rate_fpm.valid())
	{
		Speed forward_speed = (1_kt * *_true_airspeed_kt) * std::cos (1_deg * *_imu_pitch_deg);
		int ratio = (forward_speed > 1_kt)
			? limit<int> (forward_speed / (1_fpm * *_pressure_altitude_climb_rate_fpm), -99, +99)
			: 0;
		_climb_glide_ratio.write (ratio);
	}
}


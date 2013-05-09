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
				{ "settings.low-speed-roll-angle", _low_speed_roll_angle, true },
				{ "settings.speed.v-a-default", _v_a_default, true },
				{ "settings.speed.v-no-default", _v_no_default, true },
				{ "settings.speed.v-ne", _v_ne, true },
				{ "settings.speed.v-s", _v_s, true },
				{ "settings.speed.v-s0", _v_s0, true },
				{ "settings.speed.v-at", _v_at, true },
				{ "settings.speed.v-fe", _v_fe, true },
				{ "settings.speed.v-le", _v_le, true },
				{ "settings.speed.v-o", _v_o, true },
				{ "settings.speed.v-be", _v_be, true },
				{ "settings.speed.v-bg", _v_bg, true },
				{ "settings.speed.v-br", _v_br, true },
				{ "settings.flaps-configuration-properties-path", _flaps_configuration_properties_path, true },
				{ "settings.use-standard-pressure", _use_standard_pressure, true },
				{ "settings.pressure.qnh", _qnh_pressure, true },
				{ "settings.critical-aoa", _critical_aoa, true },
				{ "imu.pitch", _imu_pitch, true },
				{ "imu.roll", _imu_roll, true },
				{ "imu.magnetic-heading", _imu_magnetic_heading, true },
				{ "imu.magnetic-heading-accuracy", _imu_magnetic_heading_accuracy, true },
				{ "gps.longitude", _gps_longitude, true },
				{ "gps.latitude", _gps_latitude, true },
				{ "gps.altitude-amsl", _gps_altitude_amsl, true },
				{ "gps.accuracy", _gps_accuracy, true },
				{ "gps.timestamp", _gps_timestamp, true },
				{ "ins.longitude", _ins_longitude, true },
				{ "ins.latitude", _ins_latitude, true },
				{ "ins.altitude-amsl", _ins_altitude_amsl, true },
				{ "ins.accuracy", _ins_accuracy, true },
				{ "ins.timestamp", _ins_timestamp, true },
				{ "pressure.static", _static_pressure, true },
				{ "backup-amsl", _backup_amsl, true },
				{ "gear-down", _gear_down, true },
				{ "ias", _ias, true },
				{ "outside-air-temperature", _outside_air_temperature_k, true },
				// Output:
				{ "position.longitude", _position_longitude, true },
				{ "position.latitude", _position_latitude, true },
				{ "position.altitude-amsl", _position_altitude_amsl, true },
				{ "position.accuracy", _position_accuracy, true },
				{ "position.source", _position_source, true },
				{ "track.vertical", _track_vertical, true },
				{ "track.true-heading", _track_true_heading, true },
				{ "track.magnetic-heading", _track_magnetic_heading, true },
				{ "track.vertical-delta", _track_vertical_delta_dpf, true },
				{ "track.heading-delta", _track_heading_delta_dpf, true },
				{ "orientation.pitch", _orientation_pitch, true },
				{ "orientation.roll", _orientation_roll, true },
				{ "orientation.true-heading", _orientation_true_heading, true },
				{ "orientation.magnetic-heading", _orientation_magnetic_heading, true },
				{ "pressure-altitude.amsl", _pressure_altitude_amsl, true },
				{ "pressure-altitude.amsl-lookahead", _pressure_altitude_amsl_lookahead, true },
				{ "pressure-altitude.amsl-lookahead-time", _pressure_altitude_amsl_time, true },
				{ "pressure-altitude.climb-rate", _pressure_altitude_climb_rate, true },
				{ "speed.v-r", _v_r, true },
				{ "speed.v-ref", _v_ref, true },
				{ "speed.v-a", _v_a, true },
				{ "speed.v-no", _v_no, true },
				{ "speed.minimum-ias", _minimum_ias, true },
				{ "speed.minimum-maneuver-ias", _minimum_maneuver_ias, true },
				{ "speed.maximum-ias", _maximum_ias, true },
				{ "speed.maximum-maneuver-ias", _maximum_maneuver_ias, true },
				{ "speed.ias-lookahead", _ias_lookahead, true },
				{ "speed.ias-lookahead-time", _ias_lookahead_time, true },
				{ "speed.true-airspeed", _true_airspeed, true },
				{ "speed.ground-speed", _ground_speed, true },
				{ "speed.mach", _mach, true },
				{ "speed.sound", _sound_speed, true },
				{ "aoa.pitch-limit", _pitch_limit, true },
				{ "aoa.alpha", _aoa_alpha, true },
				{ "aoa.beta", _aoa_beta, true },
				{ "wind.true-heading", _wind_true_orientation_from, true },
				{ "wind.magnetic-heading", _wind_magnetic_orientation_from, true },
				{ "wind.true-airspeed", _wind_tas, true },
				{ "localizer.vertical-deviation", _localizer_vertical_deviation, true },
				{ "localizer.lateral-deviation", _localizer_lateral_deviation, true },
				{ "localizer.identifier", _localizer_identifier, true },
				{ "localizer.source", _localizer_source, true },
				{ "localizer.distance", _localizer_distance, true },
				{ "localizer.frequency", _localizer_frequency, true },
				{ "performance.climb-glide-ratio", _climb_glide_ratio, true },
				{ "magnetic-declination", _magnetic_declination, true },
				{ "magnetic-inclination", _magnetic_inclination, true },
				{ "density-altitude", _density_altitude, true }
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
	compute_speed_limits();
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
	_gps_accuracy.write (0.001_nm);

	if (_gps_longitude.valid() && _gps_latitude.valid() && _gps_altitude_amsl.valid() && _gps_accuracy.valid())
	{
		if (*_gps_accuracy < accuracy)
		{
			source = GPS;
			accuracy = *_gps_accuracy;
		}
	}

	if (_ins_longitude.valid() && _ins_latitude.valid() && _ins_altitude_amsl.valid() && _ins_accuracy.valid())
	{
		if (*_ins_accuracy < accuracy)
		{
			source = INS;
			accuracy = *_ins_accuracy;
		}
	}

	switch (source)
	{
		case GPS:
			_position_longitude.copy (_gps_longitude);
			_position_latitude.copy (_gps_latitude);
			_position_altitude_amsl.copy (_gps_altitude_amsl);
			_position_accuracy.copy (_gps_accuracy);
			_position_source.write ("GPS");
			break;

		case INS:
			_position_longitude.copy (_ins_longitude);
			_position_latitude.copy (_ins_latitude);
			_position_altitude_amsl.copy (_ins_altitude_amsl);
			_position_accuracy.copy (_ins_accuracy);
			_position_source.write ("INERTIAL");
			break;
	}

	// Positions history:
	_positions[1] = _positions[0];
	_positions[0].lateral_position = LonLat (*_position_longitude, *_position_latitude);
	_positions[0].altitude = _position_altitude_amsl.read (0.0_ft);
	_positions[0].accuracy = _position_accuracy.read (0.0_nm);
	_positions[0].valid = _position_longitude.valid() && _position_latitude.valid() &&
						  _position_altitude_amsl.valid() && _position_accuracy.valid();
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

	if (_static_pressure.valid() &&
		((_use_standard_pressure.valid() && *_use_standard_pressure) || _qnh_pressure.valid()))
	{
		Pressure pressure_setting = _use_standard_pressure.valid() && *_use_standard_pressure
			? 29.92_inHg
			: *_qnh_pressure;
		// Good for heights below tropopause (36 kft):
		double a = 6.8755856e-6;
		double b = 5.2558797;
		double p = (*_static_pressure).inHg();
		double p0 = pressure_setting.inHg();
		double h = -(std::pow (p / p0, 1.0 / b) - 1.0) / a;
		_pressure_altitude_amsl.write (1_ft * _pressure_alt_smoother.process (h));
	}
	else
		_pressure_altitude_amsl.copy (_position_altitude_amsl);
}


void
FlightManagementSystem::compute_headings()
{
	if (_position_longitude.valid() && _position_latitude.valid())
	{
		Xefis::MagneticVariation mv;
		mv.set_position (LonLat (*_position_longitude, *_position_latitude));
		if (_position_altitude_amsl.valid())
			mv.set_altitude_amsl (*_position_altitude_amsl);
		else
			mv.set_altitude_amsl (0_ft);
		mv.set_date (2013, 1, 1);
		mv.update();
		_magnetic_declination.write (mv.magnetic_declination());
		_magnetic_inclination.write (mv.magnetic_inclination());
	}
	else
	{
		_magnetic_declination.set_nil();
		_magnetic_inclination.set_nil();
	}

	if (_imu_magnetic_heading.valid())
	{
		_orientation_magnetic_heading.copy (_imu_magnetic_heading);

		if (_magnetic_declination.valid())
			_orientation_true_heading.write (magnetic_to_true (*_imu_magnetic_heading, *_magnetic_declination));
		else
			_orientation_true_heading.set_nil();
	}
	else
	{
		_orientation_magnetic_heading.set_nil();
		_orientation_true_heading.set_nil();
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
			_track_vertical.write (1_rad * _track_vertical_smoother.process (std::atan (altitude_diff / distance)));

			Angle initial_true_heading = _ac1_positions[0].lateral_position.initial_bearing (_ac1_positions[1].lateral_position);
			Angle true_heading = floored_mod (initial_true_heading + 180_deg, 360_deg);
			_track_true_heading.write (1_deg * _track_true_heading_smoother.process (true_heading.deg()));

			if (_magnetic_declination.valid())
				_track_magnetic_heading.write (true_to_magnetic (*_track_true_heading, *_magnetic_declination));
		}
		else
		{
			_track_vertical.set_nil();
			_track_true_heading.set_nil();
			_track_magnetic_heading.set_nil();
		}
	}
	else
	{
		_track_true_heading_smoother.reset ((*_orientation_magnetic_heading).deg());
		_track_vertical.set_nil();
		_track_true_heading.set_nil();
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
	if (_outside_air_temperature_k.valid() && _pressure_altitude_amsl.valid())
	{
		Xefis::DensityAltitude da;
		da.set_pressure_altitude (*_pressure_altitude_amsl);
		da.set_outside_air_temperature (*_outside_air_temperature_k);
		da.update();
		_density_altitude.write (da.density_altitude());
	}
	else
		_density_altitude.set_nil();
}


void
FlightManagementSystem::compute_speeds()
{
	if (_outside_air_temperature_k.valid())
	{
		Xefis::SoundSpeed ss;
		ss.set_outside_air_temperature (*_outside_air_temperature_k);
		ss.update();
		_sound_speed.write (ss.sound_speed());
	}

	if (_ias.valid() && _pressure_altitude_amsl.valid())
	{
		Speed cas = *_ias;

		if (_density_altitude.valid())
		{
			// This does not take into account air compressibility factor, so it's valid
			// for low speeds (mach < 0.3) and altitude below tropopause (36 kft):
			_true_airspeed.write (cas / std::pow (1 - 6.8755856 * 1e-6 * (*_density_altitude).ft(), 2.127940));
		}
		else
			// Very simple equation for TAS, fix it to use air temperature someday:
			_true_airspeed.write (cas + 0.02 * cas * (*_pressure_altitude_amsl / 1000_ft));
	}
	else
		_true_airspeed.set_nil();

	if (_ac2_positions[0].valid && _ac2_positions[1].valid)
	{
		Time dt = _ac2_positions[0].time - _ac2_positions[1].time;
		Length dl = _ac2_positions[0].lateral_position.haversine_earth (_ac2_positions[1].lateral_position);
		_ground_speed.write (1_kt * _ground_speed_smoother.process ((dl / dt).kt()));
	}
	else
		_ground_speed.set_nil();

	// The approximate speed of sound in dry (0% humidity) air:
	if (_true_airspeed.valid() && _sound_speed.valid())
		_mach.write (*_true_airspeed / *_sound_speed);
	else
		_mach.set_nil();

	// Climb rate:
	if (_pressure_altitude_amsl.valid())
	{
		_alt_amsl_time += update_dt();
		if (_alt_amsl_time > 0.05_s)
		{
			Length alt_diff = *_pressure_altitude_amsl - _alt_amsl_prev;
			_computed_climb_rate = alt_diff / _alt_amsl_time;
			_alt_amsl_time = 0_s;
			_alt_amsl_prev = *_pressure_altitude_amsl;
		}

		_pressure_altitude_climb_rate.write (1_fpm * _climb_rate_smoother.process (_computed_climb_rate.fpm()));
	}
	else
		_pressure_altitude_climb_rate.set_nil();
}


void
FlightManagementSystem::compute_aoa()
{
	if (_imu_pitch.valid() && _imu_roll.valid() && _imu_magnetic_heading.valid() &&
		_track_vertical.valid() && _track_magnetic_heading.valid())
	{
		Angle vdiff = floored_mod (*_imu_pitch - *_track_vertical, -180_deg, +180_deg);
		Angle hdiff = floored_mod (*_imu_magnetic_heading - *_track_magnetic_heading, -180_deg, +180_deg);
		Angle roll = *_imu_roll;

		Angle alpha = vdiff * std::cos (roll) + hdiff * std::sin (roll);
		Angle beta = -vdiff * std::sin (roll) + hdiff * std::cos (roll);

		_aoa_alpha.write (floored_mod (alpha, -180_deg, +180_deg));
		_aoa_beta.write (floored_mod (beta, -180_deg, +180_deg));
	}
	else
	{
		_aoa_alpha.set_nil();
		_aoa_beta.set_nil();
	}

	if (_aoa_alpha.valid() && _critical_aoa.valid())
		_pitch_limit.write (-*_aoa_alpha + *_critical_aoa);
	else
		_pitch_limit.set_nil();
}


void
FlightManagementSystem::compute_speed_limits()
{
	if (_v_s.valid())
		_minimum_ias.write (*_v_s);
	else
		_minimum_ias.set_nil();

	if (_v_at.valid())
		_minimum_maneuver_ias.write (*_v_at);
	else
		_minimum_maneuver_ias.set_nil();

	// TODO compute max speeds
}


void
FlightManagementSystem::compute_wind()
{
	if (_true_airspeed.valid() &&
		_ground_speed.valid() &&
		_track_true_heading.valid() &&
		_orientation_true_heading.valid())
	{
		Xefis::WindTriangle wt;
		wt.set_aircraft_tas (*_true_airspeed);
		wt.set_aircraft_track (*_track_true_heading);
		wt.set_aircraft_ground_speed (*_ground_speed);
		wt.set_aircraft_heading (*_orientation_true_heading);
		wt.update();
		_wind_true_orientation_from.write (floored_mod (1_deg * _wind_direction_smoother.process (wt.wind_direction().deg()), 360_deg));
		_wind_magnetic_orientation_from.write (true_to_magnetic (*_wind_true_orientation_from, *_magnetic_declination));
		_wind_tas.write (wt.wind_speed());
	}
	else
	{
		_wind_true_orientation_from.set_nil();
		_wind_magnetic_orientation_from.set_nil();
		_wind_tas.set_nil();
	}
}


void
FlightManagementSystem::compute_performance()
{
	if (_true_airspeed.valid() && _pressure_altitude_climb_rate.valid())
	{
		Speed forward_speed = *_true_airspeed * std::cos (*_imu_pitch);
		int ratio = (forward_speed > 1_kt)
			? limit<int> (forward_speed / *_pressure_altitude_climb_rate, -99, +99)
			: 0;
		_climb_glide_ratio.write (ratio);
	}
}


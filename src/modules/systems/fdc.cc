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

// Boost:
#include <boost/optional.hpp>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/config/exception.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/numeric.h>
#include <xefis/airnav/density_altitude.h>
#include <xefis/airnav/sound_speed.h>
#include <xefis/airnav/wind_triangle.h>
#include <xefis/airnav/magnetic_variation.h>

// Local:
#include "fdc.h"


XEFIS_REGISTER_MODULE_CLASS ("systems/fdc", FlightDataComputer);


using Xefis::floored_mod;
using Xefis::limit;


FlightDataComputer::FlightDataComputer (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	_track_lateral_true_smoother.set_winding ({ 0.0, 360.0 });
	_wind_direction_smoother.set_winding ({ 0.0, 360.0 });

	for (QDomElement& e: config)
	{
		if (e == "settings")
		{
			parse_settings (e, {
				{ "airspeed.valid-minimum", _airspeed_valid_minimum, true },
				{ "airspeed.valid-maximum", _airspeed_valid_maximum, true },
			});
		}
		else if (e == "properties")
		{
			parse_properties (e, {
				// Input:
				{ "settings.default-airplane-weight", _default_airplane_weight_g, true },
				{ "settings.actual-airplane-weight", _actual_airplane_weight_g, true },
				{ "settings.low-speed-roll-angle", _low_speed_roll_angle, true },
				{ "settings.speed.v-a-default", _v_a_default, true },
				{ "settings.speed.v-ne", _v_ne, true },
				{ "settings.speed.v-s", _v_s, true },
				{ "settings.speed.v-s0", _v_s0, true },
				{ "settings.speed.v-at", _v_at, true },
				{ "settings.speed.v-fe", _v_fe, true },
				{ "settings.speed.v-le", _v_le, true },
				{ "settings.speed.v-o", _v_o, true },
				{ "settings.speed.v-be", _v_be, true },
				{ "settings.speed.v-br", _v_br, true },
				{ "settings.speed.v-bg", _v_bg, true },
				{ "settings.use-standard-pressure", _use_standard_pressure, true },
				{ "settings.pressure.qnh", _qnh_pressure, true },
				{ "settings.critical-aoa", _critical_aoa, true },
				{ "settings.target-pressure-altitude-amsl", _target_pressure_altitude_amsl, false },
				{ "imu.pitch", _imu_pitch, true },
				{ "imu.roll", _imu_roll, true },
				{ "imu.heading.magnetic", _imu_magnetic_heading, true },
				{ "imu.heading.magnetic.accuracy", _imu_magnetic_heading_accuracy, true },
				{ "gps.longitude", _gps_longitude, true },
				{ "gps.latitude", _gps_latitude, true },
				{ "gps.altitude-amsl", _gps_altitude_amsl, true },
				{ "gps.accuracy.lateral", _gps_lateral_accuracy, true },
				{ "gps.accuracy.vertical", _gps_vertical_accuracy, true },
				{ "gps.timestamp", _gps_timestamp, true },
				{ "ins.longitude", _ins_longitude, true },
				{ "ins.latitude", _ins_latitude, true },
				{ "ins.altitude-amsl", _ins_altitude_amsl, true },
				{ "ins.accuracy.lateral", _ins_lateral_accuracy, true },
				{ "ins.accuracy.vertical", _ins_vertical_accuracy, true },
				{ "ins.timestamp", _ins_timestamp, true },
				{ "pressure.static", _static_pressure, true },
				{ "gear-down", _gear_down, true },
				{ "airspeed", _ias_input, true },
				{ "outside-air-temperature", _outside_air_temperature_k, true },
				// Output:
				{ "position.longitude", _position_longitude, true },
				{ "position.latitude", _position_latitude, true },
				{ "position.altitude-amsl", _position_altitude_amsl, true },
				{ "position.accuracy.lateral", _position_lateral_accuracy, true },
				{ "position.accuracy.vertical", _position_vertical_accuracy, true },
				{ "position.source", _position_source, true },
				{ "track.vertical", _track_vertical, true },
				{ "track.lateral.true", _track_lateral_true, true },
				{ "track.lateral.magnetic", _track_lateral_magnetic, true },
				{ "track.delta.lateral", _track_lateral_delta_dpm, true },
				{ "orientation.pitch", _orientation_pitch, true },
				{ "orientation.roll", _orientation_roll, true },
				{ "orientation.true-heading", _orientation_true_heading, true },
				{ "orientation.magnetic-heading", _orientation_magnetic_heading, true },
				{ "pressure-altitude.amsl", _pressure_altitude_amsl, true },
				{ "pressure-altitude.amsl-lookahead", _pressure_altitude_amsl_lookahead, true },
				{ "pressure-altitude-std.amsl", _pressure_altitude_std_amsl, true },
				{ "pressure-altitude.climb-rate", _pressure_altitude_climb_rate, true },
				{ "pressure-altitude.total-energy-variometer", _total_energy_variometer, false },
				{ "speed.v-a", _v_a, true },
				{ "speed.ias", _ias, true },
				{ "speed.minimum-ias", _minimum_ias, true },
				{ "speed.minimum-maneuver-ias", _minimum_maneuver_ias, true },
				{ "speed.maximum-ias", _maximum_ias, true },
				{ "speed.maximum-maneuver-ias", _maximum_maneuver_ias, true },
				{ "speed.ias-lookahead", _ias_lookahead, true },
				{ "speed.true-airspeed", _true_airspeed, true },
				{ "speed.ground-speed", _ground_speed, true },
				{ "speed.mach", _mach, true },
				{ "speed.sound", _sound_speed, true },
				{ "aoa.pitch-limit", _pitch_limit, true },
				{ "aoa.alpha", _aoa_alpha, true },
				{ "aoa.beta", _aoa_beta, true },
				{ "fpm.alpha", _fpm_alpha, true },
				{ "fpm.beta", _fpm_beta, true },
				{ "wind.heading.true", _wind_true_orientation_from, true },
				{ "wind.heading.magnetic", _wind_magnetic_orientation_from, true },
				{ "wind.true-airspeed", _wind_tas, true },
				{ "localizer.deviation.vertical", _localizer_vertical_deviation, true },
				{ "localizer.deviation.lateral", _localizer_lateral_deviation, true },
				{ "localizer.identifier", _localizer_identifier, true },
				{ "localizer.source", _localizer_source, true },
				{ "localizer.distance", _localizer_distance, true },
				{ "localizer.frequency", _localizer_frequency, true },
				{ "performance.climb-glide-ratio", _climb_glide_ratio, true },
				{ "magnetic.declination", _magnetic_declination, true },
				{ "magnetic.inclination", _magnetic_inclination, true },
				{ "density-altitude", _density_altitude, true },
				{ "target-altitude-reach-distance", _target_altitude_reach_distance, false },
			});
		}
	}

	_pressure_alt_estimator.set_minimum_integration_time (0.2_s);
	_ias_estimator.set_minimum_integration_time (0.2_s);

	_position_computer.set_callback (std::bind (&FlightDataComputer::compute_position, this));
	_position_computer.observe ({ &_gps_lateral_accuracy, &_gps_vertical_accuracy, &_gps_longitude, &_gps_latitude, &_gps_altitude_amsl,
								  &_ins_lateral_accuracy, &_ins_vertical_accuracy, &_ins_longitude, &_ins_latitude, &_ins_altitude_amsl,
								  &_static_pressure, &_use_standard_pressure, &_use_standard_pressure, &_qnh_pressure });

	_magnetic_variation_computer.set_callback (std::bind (&FlightDataComputer::compute_magnetic_variation, this));
	_magnetic_variation_computer.observe ({ &_position_longitude, &_position_latitude, &_position_altitude_amsl });

	_headings_computer.set_callback (std::bind (&FlightDataComputer::compute_headings, this));
	_headings_computer.observe ({ &_imu_magnetic_heading, &_magnetic_declination });

	_track_computer.set_callback (std::bind (&FlightDataComputer::compute_track, this));
	_track_computer.observe ({ &_position_computer, &_magnetic_declination });

	_ias_computer.set_callback (std::bind (&FlightDataComputer::compute_ias, this));
	_ias_computer.observe ({ &_ias_input });

	_da_computer.set_callback (std::bind (&FlightDataComputer::compute_da, this));
	_da_computer.observe ({ &_outside_air_temperature_k, &_pressure_altitude_amsl });

	_sound_speed_computer.set_callback (std::bind (&FlightDataComputer::compute_sound_speed, this));
	_sound_speed_computer.observe ({ &_outside_air_temperature_k });

	_true_airspeed_computer.set_callback (std::bind (&FlightDataComputer::compute_true_airspeed, this));
	_true_airspeed_computer.observe ({ &_ias, &_pressure_altitude_amsl, &_density_altitude });

	_ground_speed_computer.set_callback (std::bind (&FlightDataComputer::compute_ground_speed, this));
	_ground_speed_computer.observe ({ &_position_computer });

	_mach_computer.set_callback (std::bind (&FlightDataComputer::compute_mach, this));
	_mach_computer.observe ({ &_true_airspeed, &_sound_speed });

	_climb_rate_computer.set_callback (std::bind (&FlightDataComputer::compute_climb_rate, this));
	_climb_rate_computer.observe ({ &_pressure_altitude_std_amsl });

	_ias_lookahead_computer.set_callback (std::bind (&FlightDataComputer::compute_ias_lookahead, this));
	_ias_lookahead_computer.observe ({ &_ias });

	_fpm_computer.set_callback (std::bind (&FlightDataComputer::compute_fpm, this));
	_fpm_computer.observe ({ &_imu_pitch, &_imu_roll, &_imu_magnetic_heading, &_track_vertical, &_track_lateral_magnetic });

	_aoa_computer.set_callback (std::bind (&FlightDataComputer::compute_aoa, this));
	_aoa_computer.observe ({ &_fpm_alpha, &_fpm_beta, &_aoa_alpha, &_critical_aoa });

	_speed_limits_computer.set_callback (std::bind (&FlightDataComputer::compute_speed_limits, this));
	_speed_limits_computer.observe ({ &_v_ne, &_v_fe, &_v_le, &_v_o, &_v_s0, &_v_s });

	_wind_computer.set_callback (std::bind (&FlightDataComputer::compute_wind, this));
	_wind_computer.observe ({ &_true_airspeed, &_ground_speed, &_track_lateral_true,
							  &_orientation_true_heading, &_magnetic_declination });

	_cgratio_computer.set_callback (std::bind (&FlightDataComputer::compute_cgratio, this));
	_cgratio_computer.observe ({ &_true_airspeed, &_pressure_altitude_climb_rate });

	_tev_computer.set_callback (std::bind (&FlightDataComputer::compute_tev, this));
	_tev_computer.observe ({ &_actual_airplane_weight_g, &_pressure_altitude_std_amsl, &_ias });

	_alt_reach_distance_computer.set_callback (std::bind (&FlightDataComputer::compute_alt_reach_distance, this));
	_alt_reach_distance_computer.observe ({ &_target_pressure_altitude_amsl, &_ground_speed,
											&_pressure_altitude_climb_rate, &_pressure_altitude_amsl });
}


void
FlightDataComputer::data_updated()
{
	Xefis::PropertyObserver* computers[] = {
		// Order is important:
		&_position_computer,
		&_magnetic_variation_computer,
		&_headings_computer,
		&_track_computer,
		&_ias_computer,
		&_da_computer,
		&_sound_speed_computer,
		&_true_airspeed_computer,
		&_ground_speed_computer,
		&_mach_computer,
		&_climb_rate_computer,
		&_ias_lookahead_computer,
		&_fpm_computer,
		&_aoa_computer,
		&_speed_limits_computer,
		&_wind_computer,
		&_cgratio_computer,
		&_tev_computer,
		&_alt_reach_distance_computer
	};

	for (Xefis::PropertyObserver* o: computers)
		o->data_updated (update_time());
}


void
FlightDataComputer::compute_position()
{
	Time update_time = _position_computer.update_time();
	Time update_dt = _position_computer.update_dt();

	enum PositionSource { GPS, INS };

	PositionSource source;
	Length constexpr failed_accuracy = 100_nm;
	Length accuracy = failed_accuracy;

	if (_gps_longitude.valid() && _gps_latitude.valid() && _gps_altitude_amsl.valid() && _gps_lateral_accuracy.valid())
	{
		if (*_gps_lateral_accuracy < accuracy)
		{
			source = GPS;
			accuracy = *_gps_lateral_accuracy;
		}
	}

	if (_ins_longitude.valid() && _ins_latitude.valid() && _ins_altitude_amsl.valid() && _ins_lateral_accuracy.valid())
	{
		if (*_ins_lateral_accuracy < accuracy)
		{
			source = INS;
			accuracy = *_ins_lateral_accuracy;
		}
	}

	switch (source)
	{
		case GPS:
			_position_longitude.copy (_gps_longitude);
			_position_latitude.copy (_gps_latitude);
			_position_altitude_amsl.copy (_gps_altitude_amsl);
			_position_lateral_accuracy.copy (_gps_lateral_accuracy);
			_position_vertical_accuracy.copy (_gps_vertical_accuracy);
			_position_source.write ("GPS");
			break;

		case INS:
			_position_longitude.copy (_ins_longitude);
			_position_latitude.copy (_ins_latitude);
			_position_altitude_amsl.copy (_ins_altitude_amsl);
			_position_lateral_accuracy.copy (_ins_lateral_accuracy);
			_position_vertical_accuracy.copy (_ins_vertical_accuracy);
			_position_source.write ("INERTIAL");
			break;
	}

	// Positions history:
	_positions[1] = _positions[0];
	_positions[0].lateral_position = LonLat (*_position_longitude, *_position_latitude);
	_positions[0].altitude = _position_altitude_amsl.read (0.0_ft);
	_positions[0].lateral_accuracy = _position_lateral_accuracy.read (failed_accuracy);
	_positions[0].vertical_accuracy = _position_vertical_accuracy.read (failed_accuracy);
	_positions[0].valid = _position_longitude.valid() && _position_latitude.valid() &&
						  _position_altitude_amsl.valid() && _position_lateral_accuracy.valid() && _position_vertical_accuracy.valid();
	_positions[0].time = update_time;

	// Delayed positioning:
	if (_positions[0].valid)
	{
		Length accuracy1 = std::max (_positions[0].lateral_accuracy, _ac1_positions[0].lateral_accuracy);
		if (!_ac1_positions[0].valid ||
			_positions[0].lateral_position.haversine_earth (_ac1_positions[0].lateral_position) > 2.0 * accuracy1 ||
			_positions[0].time - _ac1_positions[0].time > 1_s)
		{
			_ac1_positions[2] = _ac1_positions[1];
			_ac1_positions[1] = _ac1_positions[0];
			_ac1_positions[0] = _positions[0];
		}

		Length accuracy2 = std::max (_positions[0].lateral_accuracy, _ac2_positions[0].lateral_accuracy);
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
		auto compute_pressure_altitude = [&](Pressure pressure_setting) -> Length {
			// Good for heights below tropopause (36 kft):
			double a = 6.8755856e-6;
			double b = 5.2558797;
			double p = (*_static_pressure).inHg();
			double p0 = pressure_setting.inHg();
			return 1_ft * -(std::pow (p / p0, 1.0 / b) - 1.0) / a;
		};

		bool hide_alt_lookahead = false;
		Pressure pressure_setting;
		if (_use_standard_pressure.valid() && *_use_standard_pressure)
		{
			pressure_setting = 29.92_inHg;
			if (!_prev_use_standard_pressure)
				hide_alt_lookahead = true;
			_prev_use_standard_pressure = true;
		}
		else
		{
			pressure_setting = *_qnh_pressure;
			if (_prev_use_standard_pressure)
				hide_alt_lookahead = true;
			_prev_use_standard_pressure = false;
		}

		if (hide_alt_lookahead)
			_hide_alt_lookahead_until = update_time + _alt_lookahead_output_smoother.smoothing_time() + _alt_lookahead_input_smoother.smoothing_time();

		Length height = compute_pressure_altitude (pressure_setting);
		Length qnh_height = compute_pressure_altitude (*_qnh_pressure);
		Length std_height = compute_pressure_altitude (29.92_inHg);

		_pressure_altitude_amsl.write (1_ft * _pressure_alt_smoother.process (height.ft(), update_dt));
		_pressure_altitude_qnh_amsl.write (1_ft * _pressure_alt_qnh_smoother.process (qnh_height.ft(), update_dt));
		_pressure_altitude_std_amsl.write (1_ft * _pressure_alt_std_smoother.process (std_height.ft(), update_dt));
	}
	else
	{
		_pressure_altitude_amsl.set_nil();
		_pressure_altitude_qnh_amsl.set_nil();
		_pressure_altitude_std_amsl.set_nil();
		_pressure_alt_smoother.invalidate();
		_pressure_alt_qnh_smoother.invalidate();
		_pressure_alt_std_smoother.invalidate();
	}

	if (_pressure_altitude_amsl.valid() && update_time > _hide_alt_lookahead_until)
	{
		double est = _pressure_alt_estimator.process (_alt_lookahead_input_smoother.process ((*_pressure_altitude_amsl).ft(), update_dt), update_dt);
		est = _alt_lookahead_output_smoother.process (est, update_dt);
		_pressure_altitude_amsl_lookahead.write (1_ft * est);
	}
	else
	{
		_pressure_altitude_amsl_lookahead.set_nil();
		_pressure_alt_estimator.invalidate();
		_alt_lookahead_input_smoother.invalidate();
		_alt_lookahead_output_smoother.invalidate();
	}
}


void
FlightDataComputer::compute_magnetic_variation()
{
	if (_position_longitude.valid() && _position_latitude.valid())
	{
		Xefis::MagneticVariation mv;
		mv.set_position (LonLat (*_position_longitude, *_position_latitude));
		if (_position_altitude_amsl.valid())
			mv.set_altitude_amsl (*_position_altitude_amsl);
		else
			mv.set_altitude_amsl (0_ft);
		mv.set_date (2013, 1, 1); // TODO get date from system or default to 2013-01-01
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
FlightDataComputer::compute_headings()
{
	if (_imu_magnetic_heading.valid())
	{
		_orientation_magnetic_heading.copy (_imu_magnetic_heading);

		if (_magnetic_declination.valid())
			_orientation_true_heading.write (Xefis::magnetic_to_true (*_imu_magnetic_heading, *_magnetic_declination));
		else
			_orientation_true_heading.set_nil();
	}
	else
	{
		_orientation_magnetic_heading.set_nil();
		_orientation_true_heading.set_nil();
	}

	_orientation_pitch.copy (_imu_pitch);
	_orientation_roll.copy (_imu_roll);
}


void
FlightDataComputer::compute_track()
{
	Time update_dt = _track_computer.update_dt();

	if (_ac1_positions[0].valid && _ac1_positions[1].valid)
	{
		Length distance = _ac1_positions[0].lateral_position.haversine_earth (_ac1_positions[1].lateral_position);
		if (distance > 2.0 * _ac1_positions[0].lateral_accuracy)
		{
			Length altitude_diff = _ac1_positions[0].altitude - _ac1_positions[1].altitude;
			_track_vertical.write (1_rad * _track_vertical_smoother.process (std::atan (altitude_diff / distance), update_dt));

			Angle initial_true_heading = _ac1_positions[0].lateral_position.initial_bearing (_ac1_positions[1].lateral_position);
			Angle true_heading = floored_mod (initial_true_heading + 180_deg, 360_deg);
			_track_lateral_true.write (1_deg * _track_lateral_true_smoother.process (true_heading.deg(), update_dt));

			if (_magnetic_declination.valid())
				_track_lateral_magnetic.write (Xefis::true_to_magnetic (*_track_lateral_true, *_magnetic_declination));
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
		_track_lateral_true_smoother.reset ((*_orientation_true_heading).deg());
		_track_vertical.set_nil();
		_track_lateral_true.set_nil();
		_track_lateral_magnetic.set_nil();
	}

	boost::optional<Angle> result_delta;
	if (_ac1_positions[0].valid && _ac1_positions[1].valid && _ac1_positions[2].valid)
	{
		Length len10 = _ac1_positions[1].lateral_position.haversine_earth (_ac1_positions[0].lateral_position);

		if (len10 >= *_position_lateral_accuracy)
		{
			Angle alpha = -180.0_deg + LonLat::great_arcs_angle (_ac1_positions[2].lateral_position,
																 _ac1_positions[1].lateral_position,
																 _ac1_positions[0].lateral_position);
			Angle beta_per_mile = alpha / len10.nm();

			if (!std::isinf (beta_per_mile.internal()) && !std::isnan (beta_per_mile.internal()))
			{
				beta_per_mile = 1_deg * _track_heading_delta_smoother.process (beta_per_mile.deg(), update_dt);
				result_delta = limit (beta_per_mile, -180.0_deg, +180.0_deg);
			}
			else
				_track_heading_delta_smoother.invalidate();
		}
	}
	else
		_track_heading_delta_smoother.invalidate();
	_track_lateral_delta_dpm.write (result_delta);
}


void
FlightDataComputer::compute_ias()
{
	Time update_dt = _ias_computer.update_dt();

	if (_ias_input.valid() && _airspeed_valid_maximum >= *_ias_input)
	{
		_airspeed_reached_minimum = _airspeed_valid_minimum <= *_ias_input;
		_ias.write (1_kt * _ias_smoother.process ((*_ias_input).kt(), update_dt));
	}
	else
	{
		_ias.set_nil();
		_ias_smoother.invalidate();
	}
}


void
FlightDataComputer::compute_da()
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
FlightDataComputer::compute_sound_speed()
{
	if (_outside_air_temperature_k.valid())
	{
		Xefis::SoundSpeed ss;
		ss.set_outside_air_temperature (*_outside_air_temperature_k);
		ss.update();
		_sound_speed.write (ss.sound_speed());
	}
	else
		_sound_speed.set_nil();
}


void
FlightDataComputer::compute_true_airspeed()
{
	if (_ias.valid() && _airspeed_reached_minimum && _pressure_altitude_amsl.valid())
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
}


void
FlightDataComputer::compute_ground_speed()
{
	if (_ac2_positions[0].valid && _ac2_positions[1].valid)
	{
		Time update_dt = _ground_speed_computer.update_dt();

		Time dt = _ac2_positions[0].time - _ac2_positions[1].time;
		Length dl = _ac2_positions[0].lateral_position.haversine_earth (_ac2_positions[1].lateral_position);
		_ground_speed.write (1_kt * _ground_speed_smoother.process ((dl / dt).kt(), update_dt));
	}
	else
	{
		_ground_speed.set_nil();
		_ground_speed_smoother.invalidate();
	}
}


void
FlightDataComputer::compute_mach()
{
	// The approximate speed of sound in dry (0% humidity) air:
	if (_true_airspeed.valid() && _sound_speed.valid())
		_mach.write (*_true_airspeed / *_sound_speed);
	else
		_mach.set_nil();
}


void
FlightDataComputer::compute_climb_rate()
{
	// Climb rate:
	if (_pressure_altitude_std_amsl.valid())
	{
		Time update_dt = _climb_rate_computer.update_dt();

		// If previous climb-rate was invalid, use current STD pressure
		// as source for 'previous altitude' value:
		if (_pressure_altitude_climb_rate.is_nil())
			_alt_amsl_prev = *_pressure_altitude_std_amsl;

		_alt_amsl_time += update_dt;
		if (_alt_amsl_time > 0.05_s)
		{
			Length alt_diff = *_pressure_altitude_std_amsl - _alt_amsl_prev;
			_computed_climb_rate = alt_diff / _alt_amsl_time;
			_alt_amsl_time = 0_s;
			_alt_amsl_prev = *_pressure_altitude_std_amsl;
			_pressure_altitude_climb_rate.write (1_fpm * _climb_rate_smoother.process (_computed_climb_rate.fpm(), update_dt));
		}
	}
	else
	{
		_pressure_altitude_climb_rate.set_nil();
		_climb_rate_smoother.invalidate();
	}
}


void
FlightDataComputer::compute_ias_lookahead()
{
	if (_ias.valid() && _airspeed_reached_minimum)
	{
		Time update_dt = _ias_lookahead_computer.update_dt();

		double est = _ias_estimator.process (_ias_lookahead_input_smoother.process ((*_ias).kt(), update_dt), update_dt);
		est = _ias_lookahead_output_smoother.process (est, update_dt);
		_ias_lookahead.write (1_kt * est);
	}
	else
	{
		_ias_lookahead.set_nil();
		_ias_estimator.invalidate();
		_ias_lookahead_input_smoother.invalidate();
		_ias_lookahead_output_smoother.invalidate();
	}
}


void
FlightDataComputer::compute_fpm()
{
	if (_imu_pitch.valid() && _imu_roll.valid() && _imu_magnetic_heading.valid() &&
		_track_vertical.valid() && _track_lateral_magnetic.valid())
	{
		Angle vdiff = floored_mod (*_imu_pitch - *_track_vertical, -180_deg, +180_deg);
		Angle hdiff = floored_mod (*_imu_magnetic_heading - *_track_lateral_magnetic, -180_deg, +180_deg);
		Angle roll = *_imu_roll;

		Angle alpha = vdiff * std::cos (roll) + hdiff * std::sin (roll);
		Angle beta = -vdiff * std::sin (roll) + hdiff * std::cos (roll);

		_fpm_alpha.write (floored_mod (alpha, -180_deg, +180_deg));
		_fpm_beta.write (floored_mod (beta, -180_deg, +180_deg));
	}
	else
	{
		_fpm_alpha.set_nil();
		_fpm_beta.set_nil();
	}
}


void
FlightDataComputer::compute_aoa()
{
	// This is not valid since AOA is relative to the air,
	// and FPM to the ground. But we don't have any better
	// AOA indicator now.
	// Therefore: TODO

	_aoa_alpha.copy (_fpm_alpha);
	_aoa_beta.copy (_fpm_beta);

	if (_aoa_alpha.valid() && _critical_aoa.valid())
		_pitch_limit.write (-*_aoa_alpha + *_critical_aoa);
	else
		_pitch_limit.set_nil();
}


void
FlightDataComputer::compute_speed_limits()
{
	// TODO zamiast flaps-extended i prędkości z tym związanych zrób tabelaryczne prędkości
	// w zależności od kąta klap.
	bool flaps_extended = false;//TODO input properties
	bool gear_lowered = _gear_down.read (false);

	// Maximum IAS:

	Speed const magic = 99999_kt;
	_maximum_ias.write (magic);

	if (_v_ne.valid())
		_maximum_ias.write (*_v_ne);
	if (_v_fe.valid() && flaps_extended)
		_maximum_ias.write (std::min (*_maximum_ias, *_v_fe));
	if (_v_le.valid() && gear_lowered)
		_maximum_ias.write (std::min (*_maximum_ias, *_v_le));

	if (_maximum_ias.valid() && *_maximum_ias == magic)
		_maximum_ias.set_nil();

	if (_v_o.valid() && !flaps_extended && !gear_lowered)
		_maximum_maneuver_ias.write (*_v_o);
	else
		_maximum_maneuver_ias.set_nil();

	// Minimum IAS:

	if (_v_s0.valid() && flaps_extended && gear_lowered)
		_minimum_ias.write (*_v_s0);
	else if (_v_s.valid())
		_minimum_ias.write (*_v_s);
	else
		_minimum_ias.set_nil();

	if (_minimum_ias.valid())
		_minimum_maneuver_ias.write (1.15 * *_minimum_ias);

	// TODO BUGS for V-be, V-bg, V-br if "speed bugs" switch is on.
	// TODO BUG for _v_at if mode is APP
	// TODO BUG for _v_a_default if mode is not APP (cruise mode)
}


void
FlightDataComputer::compute_wind()
{
	if (_true_airspeed.valid() &&
		_ground_speed.valid() &&
		_track_lateral_true.valid() &&
		_orientation_true_heading.valid() &&
		_magnetic_declination.valid())
	{
		Time update_dt = _wind_computer.update_dt();

		Xefis::WindTriangle wt;
		wt.set_aircraft_tas (*_true_airspeed);
		wt.set_aircraft_track (*_track_lateral_true);
		wt.set_aircraft_ground_speed (*_ground_speed);
		wt.set_aircraft_heading (*_orientation_true_heading);
		wt.update();
		_wind_true_orientation_from.write (floored_mod (1_deg * _wind_direction_smoother.process (wt.wind_direction().deg(), update_dt), 360_deg));
		_wind_magnetic_orientation_from.write (Xefis::true_to_magnetic (*_wind_true_orientation_from, *_magnetic_declination));
		_wind_tas.write (wt.wind_speed());
	}
	else
	{
		_wind_true_orientation_from.set_nil();
		_wind_magnetic_orientation_from.set_nil();
		_wind_tas.set_nil();
		_wind_direction_smoother.invalidate();
	}
}


void
FlightDataComputer::compute_cgratio()
{
	if (_true_airspeed.valid() && _pressure_altitude_climb_rate.valid())
	{
		Speed forward_speed = *_true_airspeed * std::cos (*_imu_pitch);
		int ratio = (forward_speed > 1_kt)
			? limit<int> (forward_speed / *_pressure_altitude_climb_rate, -99, +99)
			: 0;
		_climb_glide_ratio.write (ratio);
	}
	else
		_climb_glide_ratio.set_nil();
}


void
FlightDataComputer::compute_tev()
{
	if (_total_energy_variometer.configured())
	{
		Time update_dt = _tev_computer.update_dt();

		if (_actual_airplane_weight_g.valid() &&
			_pressure_altitude_std_amsl.valid() &&
			_ias.valid() &&
			_airspeed_reached_minimum)
		{
			double const m = *_actual_airplane_weight_g;
			double const g = 9.81;

			if ((_total_energy_time += update_dt) > 0.1_s)
			{
				double const v = (*_ias).mps();
				double const Ep = m * g * (*_pressure_altitude_std_amsl).m();
				double const Ek = m * v * v * 0.5;
				_prev_total_energy = _total_energy;
				_total_energy = Ep + Ek;

				// If total energy was nil (invalid), reset _prev_total_energy
				// value to the current _total_energy:
				if (_total_energy_variometer.is_nil())
					_prev_total_energy = _total_energy;

				double const energy_diff = _total_energy - _prev_total_energy;
				_tev = (1_m * energy_diff / (m * g)) / _total_energy_time;
				_total_energy_time = 0_s;
				_total_energy_variometer.write (1_fpm * _variometer_smoother.process (_tev.fpm(), update_dt));
			}
		}
		else
		{
			_total_energy_variometer.set_nil();
			_variometer_smoother.invalidate();
		}
	}
}


void
FlightDataComputer::compute_alt_reach_distance()
{
	if (_target_altitude_reach_distance.configured())
	{
		Time update_dt = _alt_reach_distance_computer.update_dt();

		if (_target_pressure_altitude_amsl.valid() &&
			_ground_speed.valid() &&
			_pressure_altitude_climb_rate.valid() &&
			_pressure_altitude_amsl.valid())
		{
			Length const alt_diff = *_target_pressure_altitude_amsl - *_pressure_altitude_amsl;
			Length const distance = *_ground_speed * (alt_diff / *_pressure_altitude_climb_rate);
			_target_altitude_reach_distance.write (1_m * _alt_reach_distance_smoother.process (distance.m(), update_dt));
		}
		else
		{
			_target_altitude_reach_distance.set_nil();
			_alt_reach_distance_smoother.invalidate();
		}
	}
}


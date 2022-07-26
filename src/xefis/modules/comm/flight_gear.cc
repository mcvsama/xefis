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
#include "flight_gear.h"

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/numeric.h>
#include <neutrino/qt/qdom.h>

// Standard:
#include <cstddef>
#include <cstring>


typedef float		FGFloat;
typedef double		FGDouble;
typedef uint8_t		FGBool;
typedef uint32_t	FGInt;


BEGIN_PACKED_STRUCT
struct FGInputData
{
	FGDouble	rotation_x_degps;					// rox
	FGDouble	rotation_y_degps;					// roy
	FGDouble	rotation_z_degps;					// roz
	FGDouble	acceleration_x_fps2;				// acx
	FGDouble	acceleration_y_fps2;				// acy
	FGDouble	acceleration_z_fps2;				// acz
	FGDouble	aoa_alpha_maximum_rad;				// ama
	FGDouble	aoa_alpha_minimum_rad;				// ami
	FGDouble	aoa_alpha_rad;						// aoa
	FGDouble	cmd_alt_setting_ft;					// apa
	FGDouble	cmd_cbr_setting_fpm;				// apc
	FGDouble	cmd_speed_setting_kt;				// ats
	FGDouble	cmd_heading_setting_deg;			// aph
	FGDouble	flight_director_pitch_deg;			// fdp
	FGDouble	flight_director_roll_deg;			// fdr
	FGDouble	ias_kt;								// ias
	FGDouble	tas_kt;								// tas
	FGDouble	gs_kt;								// gs
	FGDouble	mach;								// ma
	FGDouble	ias_lookahead_kt;					// iasl
	FGDouble	maximum_ias_kt;						// iasma
	FGDouble	minimum_ias_kt;						// iasmi
	FGBool		standard_pressure;					// std
	FGDouble	altitude_ft;						// al
	FGDouble	radar_altimeter_altitude_agl_ft;	// alr
	FGDouble	pressure_inHg;						// als
	FGDouble	cbr_fpm;							// cbr
	FGDouble	gps_latitude_deg;					// lt
	FGDouble	gps_longitude_deg;					// ln
	FGDouble	gps_amsl_ft;						// alg
	FGDouble	ahrs_pitch_deg;						// p
	FGDouble	ahrs_roll_deg;						// r
	FGDouble	ahrs_magnetic_heading_deg;			// h
	FGDouble	ahrs_true_heading_deg;				// th
	FGDouble	fpm_alpha_deg;						// fpa
	FGDouble	fpm_beta_deg;						// fpb
	FGDouble	magnetic_track_deg;					// tr
	FGBool		navigation_needles_visible;			// nav
	FGBool		vertical_deviation_ok;				// ngso
	FGDouble	vertical_deviation_deg;				// ngs
	FGBool		lateral_deviation_ok;				// nhdo
	FGDouble	lateral_deviation_deg;				// nhd
	FGBool		navigation_dme_ok;					// dok
	FGDouble	dme_distance_nmi;					// dme
	FGDouble	slip_skid_g;						// ss
	FGDouble	total_air_temperature_degc;			// tat
	FGDouble	engine_throttle_pct;				// thr
	FGDouble	engine_1_thrust_lb;					// thrust1
	FGDouble	engine_1_rpm_rpm;					// rpm1
	FGDouble	engine_1_pitch_deg;					// pitch1
	FGDouble	engine_1_epr;						// epr1
	FGDouble	engine_1_n1_pct;					// n1-1
	FGDouble	engine_1_n2_pct;					// n2-1
	FGDouble	engine_1_egt_degf;					// egt1
	FGDouble	engine_2_thrust_lb;					// thrust2
	FGDouble	engine_2_rpm_rpm;					// rpm2
	FGDouble	engine_2_pitch_deg;					// pitch2
	FGDouble	engine_2_epr;						// epr2
	FGDouble	engine_2_n1_pct;					// n1-2
	FGDouble	engine_2_n2_pct;					// n2-2
	FGDouble	engine_2_egt_degf;					// egt2
	FGDouble	wind_from_magnetic_heading_deg;		// wfh
	FGDouble	wind_tas_kt;						// ws
	FGBool		gear_setting_down;					// gd
	FGDouble	gear_nose_position;					// gdn
	FGDouble	gear_left_position;					// gdl
	FGDouble	gear_right_position;				// gdr
}
END_PACKED_STRUCT


BEGIN_PACKED_STRUCT
struct FGOutputData
{
	FGFloat		ailerons;						// a
	FGFloat		elevator;						// e
	FGFloat		rudder;							// r
	FGFloat		throttle_1;						// t1
	FGFloat		throttle_2;						// t2
	FGFloat		flaps;							// f
}
END_PACKED_STRUCT


FlightGear::FlightGear (std::string_view const& instance):
	FlightGearIO (instance)
{
	_serviceable_flags = {
		&_io.ahrs_serviceable,
		&_io.ias_serviceable,
		&_io.radar_altimeter_serviceable,
		&_io.pressure_serviceable,
		&_io.gps_serviceable,
	};

	_output_sockets = {
		&_io.rotation_x,
		&_io.rotation_y,
		&_io.rotation_z,
		&_io.acceleration_x,
		&_io.acceleration_y,
		&_io.acceleration_z,
		&_io.aoa_alpha_maximum,
		&_io.aoa_alpha_minimum,
		&_io.aoa_alpha,
		&_io.ias,
		&_io.ias_lookahead,
		&_io.minimum_ias,
		&_io.maximum_ias,
		&_io.gs,
		&_io.tas,
		&_io.mach,
		&_io.ahrs_pitch,
		&_io.ahrs_roll,
		&_io.ahrs_magnetic_heading,
		&_io.ahrs_true_heading,
		&_io.slip_skid,
		&_io.fpm_alpha,
		&_io.fpm_beta,
		&_io.magnetic_track,
		&_io.standard_pressure,
		&_io.altitude,
		&_io.radar_altimeter_altitude_agl,
		&_io.cbr,
		&_io.pressure,
		&_io.cmd_alt_setting,
		&_io.cmd_speed_setting,
		&_io.cmd_heading_setting,
		&_io.cmd_cbr_setting,
		&_io.flight_director_pitch,
		&_io.flight_director_roll,
		&_io.navigation_needles_visible,
		&_io.lateral_deviation,
		&_io.vertical_deviation,
		&_io.dme_distance,
		&_io.total_air_temperature,
		&_io.engine_throttle_pct,
		&_io.engine_1_thrust,
		&_io.engine_1_rpm,
		&_io.engine_1_pitch,
		&_io.engine_1_epr,
		&_io.engine_1_n1_pct,
		&_io.engine_1_n2_pct,
		&_io.engine_1_egt,
		&_io.engine_2_thrust,
		&_io.engine_2_rpm,
		&_io.engine_2_pitch,
		&_io.engine_2_epr,
		&_io.engine_2_n1_pct,
		&_io.engine_2_n2_pct,
		&_io.engine_2_egt,
		&_io.gps_latitude,
		&_io.gps_longitude,
		&_io.gps_amsl,
		&_io.gps_lateral_stddev,
		&_io.gps_vertical_stddev,
		&_io.wind_from_magnetic_heading,
		&_io.wind_tas,
		&_io.gear_setting_down,
		&_io.gear_nose_up,
		&_io.gear_nose_down,
		&_io.gear_left_up,
		&_io.gear_left_down,
		&_io.gear_right_up,
		&_io.gear_right_down,
	};

	_timeout_timer = std::make_unique<QTimer>();
	_timeout_timer->setSingleShot (true);
	_timeout_timer->setInterval (200);
	QObject::connect (_timeout_timer.get(), SIGNAL (timeout()), this, SLOT (invalidate_all()));

	invalidate_all();
}


void
FlightGear::initialize()
{
	_input_address = QHostAddress (QString::fromStdString (*_io.input_host));
	_output_address = QHostAddress (QString::fromStdString (*_io.output_host));

	_input = std::make_unique<QUdpSocket>();
	_input->bind (_input_address, *_io.input_port, QUdpSocket::ShareAddress);
	QObject::connect (_input.get(), SIGNAL (readyRead()), this, SLOT (got_packet()));

	_output = std::make_unique<QUdpSocket>();
}


void
FlightGear::got_packet()
{
	read_input();
	write_output();
}


void
FlightGear::invalidate_all()
{
	for (auto* socket: _output_sockets)
		*socket = xf::nil;

	for (auto* flag: _serviceable_flags)
		*flag = false;
}


void
FlightGear::read_input()
{
	while (_input->hasPendingDatagrams())
	{
		int datagram_size = _input->pendingDatagramSize();
		if (_input_datagram.size() < datagram_size)
			_input_datagram.resize (datagram_size);

		_input->readDatagram (_input_datagram.data(), datagram_size, nullptr, nullptr);

		if (!_io.input_enabled)
			continue;

		FGInputData fg_data;
		std::memcpy (&fg_data, _input_datagram.data(), sizeof (fg_data));

#define ASSIGN(unit, x) \
		_io.x = 1_##unit * fg_data.x##_##unit;

#define ASSIGN_UNITLESS(x) \
		_io.x = static_cast<decltype (fg_data.x)> (fg_data.x);

		ASSIGN (ft,   cmd_alt_setting);
		ASSIGN (fpm,  cmd_cbr_setting);
		ASSIGN (kt,   cmd_speed_setting);
		ASSIGN (deg,  cmd_heading_setting);
		ASSIGN (deg,  flight_director_pitch);
		ASSIGN (deg,  flight_director_roll);
		ASSIGN (rad,  aoa_alpha_maximum);
		ASSIGN (rad,  aoa_alpha_minimum);
		ASSIGN (rad,  aoa_alpha);
		ASSIGN (kt,   ias);
		ASSIGN (kt,   tas);
		ASSIGN (kt,   gs);
		ASSIGN_UNITLESS (mach);
		ASSIGN (kt,   ias_lookahead);
		ASSIGN (kt,   maximum_ias);
		ASSIGN (kt,   minimum_ias);
		ASSIGN_UNITLESS (standard_pressure);
		ASSIGN (ft,   altitude);
		ASSIGN (ft,   radar_altimeter_altitude_agl);
		ASSIGN (inHg, pressure);
		ASSIGN (fpm,  cbr);
		ASSIGN (deg,  gps_latitude);
		ASSIGN (deg,  gps_longitude);
		ASSIGN (ft,   gps_amsl);
		ASSIGN (deg,  ahrs_pitch);
		ASSIGN (deg,  ahrs_roll);
		ASSIGN (deg,  ahrs_magnetic_heading);
		ASSIGN (deg,  ahrs_true_heading);
		ASSIGN (deg,  fpm_alpha);
		ASSIGN (deg,  fpm_beta);
		ASSIGN (deg,  magnetic_track);
		ASSIGN_UNITLESS (navigation_needles_visible);
		ASSIGN (nmi,  dme_distance);
		ASSIGN (g,    slip_skid);
		ASSIGN_UNITLESS (engine_throttle_pct);
		ASSIGN (rpm,  engine_1_rpm);
		ASSIGN (deg,  engine_1_pitch);
		ASSIGN_UNITLESS (engine_1_epr);
		ASSIGN_UNITLESS (engine_1_n1_pct);
		ASSIGN_UNITLESS (engine_1_n2_pct);
		ASSIGN (rpm,  engine_2_rpm);
		ASSIGN (deg,  engine_2_pitch);
		ASSIGN_UNITLESS (engine_2_epr);
		ASSIGN_UNITLESS (engine_2_n1_pct);
		ASSIGN_UNITLESS (engine_2_n2_pct);
		ASSIGN (deg,  wind_from_magnetic_heading);
		ASSIGN (kt,   wind_tas);
		ASSIGN_UNITLESS (gear_setting_down);

#undef ASSIGN_UNITLESS
#undef ASSIGN

		_io.rotation_x = 1_deg * fg_data.rotation_x_degps / 1_s;
		_io.rotation_y = 1_deg * fg_data.rotation_y_degps / 1_s;
		_io.rotation_z = 1_deg * fg_data.rotation_z_degps / 1_s;

		_io.acceleration_x = 1_ft * fg_data.acceleration_x_fps2 / 1_s / 1_s;
		_io.acceleration_y = 1_ft * fg_data.acceleration_y_fps2 / 1_s / 1_s;
		_io.acceleration_z = -1_ft * fg_data.acceleration_z_fps2 / 1_s / 1_s;

		_io.vertical_deviation = 2_deg * fg_data.vertical_deviation_deg;
		_io.lateral_deviation = 2_deg * fg_data.lateral_deviation_deg;

		if (!fg_data.vertical_deviation_ok)
			_io.vertical_deviation = xf::nil;

		if (!fg_data.lateral_deviation_ok)
			_io.lateral_deviation = xf::nil;

		if (!fg_data.navigation_dme_ok)
			_io.dme_distance = xf::nil;

		_io.gear_nose_down = fg_data.gear_nose_position > 0.999;
		_io.gear_left_down = fg_data.gear_left_position > 0.999;
		_io.gear_right_down = fg_data.gear_right_position > 0.999;

		_io.gear_nose_up = fg_data.gear_nose_position < 0.001;
		_io.gear_left_up = fg_data.gear_left_position < 0.001;
		_io.gear_right_up = fg_data.gear_right_position < 0.001;

		// TAT
		_io.total_air_temperature = si::Quantity<si::Celsius> (fg_data.total_air_temperature_degc);

		// Convert EGT from °F to Kelvins:
		_io.engine_1_egt = si::Quantity<si::Fahrenheit> (fg_data.engine_1_egt_degf);
		_io.engine_2_egt = si::Quantity<si::Fahrenheit> (fg_data.engine_2_egt_degf);

		// Engine thrust:
		_io.engine_1_thrust = 1_lb * fg_data.engine_1_thrust_lb * 1_g;
		_io.engine_2_thrust = 1_lb * fg_data.engine_2_thrust_lb * 1_g;
	}

	if (_io.maximum_ias && *_io.maximum_ias < 1_kt)
		_io.maximum_ias = xf::nil;

	if (_io.minimum_ias && *_io.minimum_ias < 1_kt)
		_io.minimum_ias = xf::nil;

	if (_io.radar_altimeter_altitude_agl && *_io.radar_altimeter_altitude_agl > 2500_ft)
		_io.radar_altimeter_altitude_agl = xf::nil;

	for (auto* flag: _serviceable_flags)
		*flag = true;

	_io.gps_lateral_stddev = 1_m;
	_io.gps_vertical_stddev = 1_m;
	_io.gps_source = "GPS";

	_timeout_timer->start();
}


void
FlightGear::write_output()
{
	if (!_io.output_enabled)
		return;

	FGOutputData fg_data;

#define ASSIGN(x, def) \
		fg_data.x = _io.x.value_or (def);

	ASSIGN (ailerons, 0.0);
	ASSIGN (elevator, 0.0);
	ASSIGN (rudder, 0.0);
	ASSIGN (throttle_1, 0.0);
	ASSIGN (throttle_2, 0.0);
	ASSIGN (flaps, 0.0);

#undef ASSIGN

	_output->writeDatagram (reinterpret_cast<const char*> (&fg_data), sizeof (fg_data), _output_address, *_io.output_port);
}


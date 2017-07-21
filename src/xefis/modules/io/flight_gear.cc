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

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/services.h>
#include <xefis/core/xefis.h>
#include <xefis/utility/numeric.h>
#include <xefis/utility/painter.h>
#include <xefis/utility/qdom.h>

// Local:
#include "flight_gear.h"


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


FlightGearIO::FlightGearIO (std::string const& instance):
	Module (instance)
{
	_serviceable_flags = {
		&output_ahrs_serviceable,
		&output_ias_serviceable,
		&output_radar_altimeter_serviceable,
		&output_pressure_serviceable,
		&output_gps_serviceable,
	};

	_output_properties = {
		&output_rotation_x,
		&output_rotation_y,
		&output_rotation_z,
		&output_acceleration_x,
		&output_acceleration_y,
		&output_acceleration_z,
		&output_aoa_alpha_maximum,
		&output_aoa_alpha_minimum,
		&output_aoa_alpha,
		&output_ias,
		&output_ias_lookahead,
		&output_minimum_ias,
		&output_maximum_ias,
		&output_gs,
		&output_tas,
		&output_mach,
		&output_ahrs_pitch,
		&output_ahrs_roll,
		&output_ahrs_magnetic_heading,
		&output_ahrs_true_heading,
		&output_slip_skid,
		&output_fpm_alpha,
		&output_fpm_beta,
		&output_magnetic_track,
		&output_standard_pressure,
		&output_altitude,
		&output_radar_altimeter_altitude_agl,
		&output_cbr,
		&output_pressure,
		&output_cmd_alt_setting,
		&output_cmd_speed_setting,
		&output_cmd_heading_setting,
		&output_cmd_cbr_setting,
		&output_flight_director_pitch,
		&output_flight_director_roll,
		&output_navigation_needles_visible,
		&output_lateral_deviation,
		&output_vertical_deviation,
		&output_dme_distance,
		&output_total_air_temperature,
		&output_engine_throttle_pct,
		&output_engine_1_thrust,
		&output_engine_1_rpm,
		&output_engine_1_pitch,
		&output_engine_1_epr,
		&output_engine_1_n1_pct,
		&output_engine_1_n2_pct,
		&output_engine_1_egt,
		&output_engine_2_thrust,
		&output_engine_2_rpm,
		&output_engine_2_pitch,
		&output_engine_2_epr,
		&output_engine_2_n1_pct,
		&output_engine_2_n2_pct,
		&output_engine_2_egt,
		&output_gps_latitude,
		&output_gps_longitude,
		&output_gps_amsl,
		&output_gps_lateral_stddev,
		&output_gps_vertical_stddev,
		&output_wind_from_magnetic_heading,
		&output_wind_tas,
		&output_gear_setting_down,
		&output_gear_nose_up,
		&output_gear_nose_down,
		&output_gear_left_up,
		&output_gear_left_down,
		&output_gear_right_up,
		&output_gear_right_down,
	};

	_timeout_timer = std::make_unique<QTimer>();
	_timeout_timer->setSingleShot (true);
	_timeout_timer->setInterval (200);
	QObject::connect (_timeout_timer.get(), SIGNAL (timeout()), this, SLOT (invalidate_all()));

	invalidate_all();
}


void
FlightGearIO::initialize()
{
	_input_address = QHostAddress (QString::fromStdString (*setting_input_host));
	_output_address = QHostAddress (QString::fromStdString (*setting_output_host));

	_input = std::make_unique<QUdpSocket>();
	_input->bind (_input_address, *setting_input_port, QUdpSocket::ShareAddress);
	QObject::connect (_input.get(), SIGNAL (readyRead()), this, SLOT (got_packet()));

	_output = std::make_unique<QUdpSocket>();
}


void
FlightGearIO::got_packet()
{
	read_input();
	write_output();
}


void
FlightGearIO::invalidate_all()
{
	for (auto property: _output_properties)
		property->set_nil();

	for (auto flag: _serviceable_flags)
		*flag = false;
}


void
FlightGearIO::read_input()
{
	while (_input->hasPendingDatagrams())
	{
		int datagram_size = _input->pendingDatagramSize();
		if (_input_datagram.size() < datagram_size)
			_input_datagram.resize (datagram_size);

		_input->readDatagram (_input_datagram.data(), datagram_size, nullptr, nullptr);

		if (!setting_input_enabled)
			continue;

		FGInputData* fg_data = reinterpret_cast<FGInputData*> (_input_datagram.data());

#define ASSIGN(unit, x) \
		output_##x = 1_##unit * fg_data->x##_##unit;

#define ASSIGN_UNITLESS(x) \
		output_##x = static_cast<decltype (fg_data->x)> (fg_data->x);

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

		output_rotation_x = 1_deg * fg_data->rotation_x_degps / 1_s;
		output_rotation_y = 1_deg * fg_data->rotation_y_degps / 1_s;
		output_rotation_z = 1_deg * fg_data->rotation_z_degps / 1_s;

		output_acceleration_x = 1_ft * fg_data->acceleration_x_fps2 / 1_s / 1_s;
		output_acceleration_y = 1_ft * fg_data->acceleration_y_fps2 / 1_s / 1_s;
		output_acceleration_z = -1_ft * fg_data->acceleration_z_fps2 / 1_s / 1_s;

		output_vertical_deviation = 2_deg * fg_data->vertical_deviation_deg;
		output_lateral_deviation = 2_deg * fg_data->lateral_deviation_deg;

		if (!fg_data->vertical_deviation_ok)
			output_vertical_deviation.set_nil();

		if (!fg_data->lateral_deviation_ok)
			output_lateral_deviation.set_nil();

		if (!fg_data->navigation_dme_ok)
			output_dme_distance.set_nil();

		output_gear_nose_down = fg_data->gear_nose_position > 0.999;
		output_gear_left_down = fg_data->gear_left_position > 0.999;
		output_gear_right_down = fg_data->gear_right_position > 0.999;

		output_gear_nose_up = fg_data->gear_nose_position < 0.001;
		output_gear_left_up = fg_data->gear_left_position < 0.001;
		output_gear_right_up = fg_data->gear_right_position < 0.001;

		// TAT
		output_total_air_temperature = Quantity<Celsius> (fg_data->total_air_temperature_degc);

		// Convert EGT from °F to Kelvins:
		output_engine_1_egt = Quantity<Fahrenheit> (fg_data->engine_1_egt_degf);
		output_engine_2_egt = Quantity<Fahrenheit> (fg_data->engine_2_egt_degf);

		// Engine thrust:
		output_engine_1_thrust = 1_lb * fg_data->engine_1_thrust_lb * 1_g;
		output_engine_2_thrust = 1_lb * fg_data->engine_2_thrust_lb * 1_g;
	}

	if (output_maximum_ias && *output_maximum_ias < 1_kt)
		output_maximum_ias.set_nil();

	if (output_minimum_ias && *output_minimum_ias < 1_kt)
		output_minimum_ias.set_nil();

	if (output_radar_altimeter_altitude_agl && *output_radar_altimeter_altitude_agl > 2500_ft)
		output_radar_altimeter_altitude_agl.set_nil();

	for (auto flag: _serviceable_flags)
		*flag = true;

	output_gps_lateral_stddev = 1_m;
	output_gps_vertical_stddev = 1_m;
	output_gps_source = "GPS";

	_timeout_timer->start();
}


void
FlightGearIO::write_output()
{
	if (!setting_output_enabled)
		return;

	FGOutputData fg_data;

#define ASSIGN(x, def) \
		fg_data.x = input_##x.value_or (def);

	ASSIGN (ailerons, 0.0);
	ASSIGN (elevator, 0.0);
	ASSIGN (rudder, 0.0);
	ASSIGN (throttle_1, 0.0);
	ASSIGN (throttle_2, 0.0);
	ASSIGN (flaps, 0.0);

#undef ASSIGN

	_output->writeDatagram (reinterpret_cast<const char*> (&fg_data), sizeof (fg_data), _output_address, *setting_output_port);
}


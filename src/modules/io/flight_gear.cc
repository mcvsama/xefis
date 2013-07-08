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

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/application.h>
#include <xefis/core/services.h>
#include <xefis/utility/numeric.h>
#include <xefis/utility/painter.h>
#include <xefis/utility/qdom.h>

// Local:
#include "flight_gear.h"


XEFIS_REGISTER_MODULE_CLASS ("io/flightgear", FlightGearIO);


typedef float		FGFloat;
typedef double		FGDouble;
typedef uint8_t		FGBool;
typedef uint32_t	FGInt;


BEGIN_PACKED_STRUCT
struct FGInputData
{
	FGDouble	cmd_alt_setting_ft;				// apa
	FGDouble	cmd_cbr_setting_fpm;			// apc
	FGDouble	cmd_speed_setting_kt;			// ats
	FGDouble	cmd_heading_setting_deg;		// aph
	FGDouble	flight_director_pitch_deg;		// fdp
	FGDouble	flight_director_roll_deg;		// fdr
	FGDouble	ias_kt;							// ias
	FGDouble	tas_kt;							// tas
	FGDouble	gs_kt;							// gs
	FGDouble	mach;							// ma
	FGDouble	ias_lookahead_kt;				// iasl
	FGDouble	maximum_ias_kt;					// iasma
	FGDouble	minimum_ias_kt;					// iasmi
	FGBool		standard_pressure;				// std
	FGDouble	altitude_ft;					// al
	FGDouble	altitude_agl_ft;				// alr
	FGDouble	pressure_inHg;					// als
	FGDouble	cbr_fpm;						// cbr
	FGDouble	position_lat_deg;				// lt
	FGDouble	position_lng_deg;				// ln
	FGDouble	position_amsl_ft;				// alg
	FGDouble	pitch_deg;						// p
	FGDouble	roll_deg;						// r
	FGDouble	magnetic_heading_deg;			// h
	FGDouble	true_heading_deg;				// th
	FGDouble	fpm_alpha_deg;					// fpa
	FGDouble	fpm_beta_deg;					// fpb
	FGDouble	magnetic_track_deg;				// tr
	FGBool		navigation_needles_visible;		// nav
	FGBool		vertical_deviation_ok;			// ngso
	FGDouble	vertical_deviation_deg;			// ngs
	FGBool		lateral_deviation_ok;			// nhdo
	FGDouble	lateral_deviation_deg;			// nhd
	FGBool		navigation_dme_ok;				// dok
	FGDouble	dme_distance_nm;				// dme
	FGDouble	slip_skid_g;					// ss
	FGDouble	outside_air_temperature_k;		// tmp
	FGDouble	engine_throttle_pct;			// thr
	FGDouble	engine_epr;						// epr
	FGDouble	engine_n1_pct;					// n1
	FGDouble	engine_n2_pct;					// n2
	FGDouble	engine_egt_degc;				// egt
	FGDouble	wind_from_magnetic_heading_deg;	// wfh
	FGDouble	wind_tas_kt;					// ws
}
END_PACKED_STRUCT


BEGIN_PACKED_STRUCT
struct FGOutputData
{
	FGFloat	ailerons;						// a
	FGFloat	elevator;						// e
	FGFloat	rudder;							// r
}
END_PACKED_STRUCT


FlightGearIO::FlightGearIO (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Input (module_manager)
{
	for (QDomElement& e: config)
	{
		if (e == "input")
		{
			_input_enabled = e.attribute ("disabled") != "true";

			for (QDomElement& e2: e)
			{
				if (e2 == "host")
					_input_host = e2.text();
				else if (e2 == "port")
					_input_port = e2.text().toInt();
				else if (e2 == "properties")
				{
					parse_properties (e2, {
						{ "ias", _ias, false },
						{ "ias-lookahead", _ias_lookahead, false },
						{ "ias-minimum", _minimum_ias, false },
						{ "ias-maximum", _maximum_ias, false },
						{ "gs", _gs, false },
						{ "tas", _tas, false },
						{ "mach", _mach, false },
						{ "orientation-pitch", _pitch, false },
						{ "orientation-roll", _roll, false },
						{ "orientation-magnetic-heading", _magnetic_heading, false },
						{ "orientation-true-heading", _true_heading, false },
						{ "slip-skid", _slip_skid_g, false },
						{ "flight-path-marker-alpha", _fpm_alpha, false },
						{ "flight-path-marker-beta", _fpm_beta, false },
						{ "magnetic-track", _magnetic_track, false },
						{ "standard-pressure", _standard_pressure, false },
						{ "altitude", _altitude, false },
						{ "altitude-agl", _altitude_agl, false },
						{ "cbr", _cbr, false },
						{ "pressure", _pressure, false },
						{ "cmd-setting-altitude", _cmd_alt_setting, false },
						{ "cmd-setting-ias", _cmd_speed_setting, false },
						{ "cmd-setting-heading", _cmd_heading_setting, false },
						{ "cmd-setting-cbr", _cmd_cbr_setting, false },
						{ "flight-director-pitch", _flight_director_pitch, false },
						{ "flight-director-roll", _flight_director_roll, false },
						{ "navigation-needles-visible", _navigation_needles_visible, false },
						{ "lateral-deviation", _lateral_deviation, false },
						{ "vertical-deviation", _vertical_deviation, false },
						{ "dme-distance", _dme_distance, false },
						{ "outside-air-temperature", _outside_air_temperature_k, false },
						{ "engine-throttle-pct", _engine_throttle_pct, false },
						{ "engine-epr", _engine_epr, false },
						{ "engine-n1", _engine_n1_pct, false },
						{ "engine-n2", _engine_n2_pct, false },
						{ "engine-egt", _engine_egt_degc, false },
						{ "position-latitude", _position_lat, false },
						{ "position-longitude", _position_lng, false },
						{ "position-amsl", _position_amsl, false },
						{ "wind-from-mag-heading", _wind_from_magnetic_heading, false },
						{ "wind-tas", _wind_tas, false },
					});
				}
			}
		}
		else if (e == "output")
		{
			_output_enabled = e.attribute ("disabled") != "true";

			for (QDomElement& e2: e)
			{
				if (e2 == "host")
					_output_host = e2.text();
				else if (e2 == "port")
					_output_port = e2.text().toInt();
				else if (e2 == "properties")
				{
					parse_properties (e2, {
						{ "ailerons", _ailerons, false },
						{ "elevator", _elevator, false },
						{ "rudder", _rudder, false },
					});
				}
			}
		}
	}

	_timeout_timer = new QTimer();
	_timeout_timer->setSingleShot (true);
	_timeout_timer->setInterval (200);
	QObject::connect (_timeout_timer, SIGNAL (timeout()), this, SLOT (invalidate_all()));

	_input = new QUdpSocket();
	_input->bind (QHostAddress (_input_host), _input_port, QUdpSocket::ShareAddress);
	QObject::connect (_input, SIGNAL (readyRead()), this, SLOT (got_packet()));

	_output = new QUdpSocket();

	invalidate_all();
}


FlightGearIO::~FlightGearIO()
{
	delete _input;
	delete _output;
	delete _timeout_timer;
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
	Xefis::GenericProperty* properties[] = {
		&_ias,
		&_ias_lookahead,
		&_minimum_ias,
		&_maximum_ias,
		&_gs,
		&_tas,
		&_mach,
		&_pitch,
		&_roll,
		&_magnetic_heading,
		&_true_heading,
		&_slip_skid_g,
		&_fpm_alpha,
		&_fpm_beta,
		&_magnetic_track,
		&_standard_pressure,
		&_altitude,
		&_altitude_agl,
		&_cbr,
		&_pressure,
		&_cmd_alt_setting,
		&_cmd_speed_setting,
		&_cmd_heading_setting,
		&_cmd_cbr_setting,
		&_flight_director_pitch,
		&_flight_director_roll,
		&_navigation_needles_visible,
		&_lateral_deviation,
		&_vertical_deviation,
		&_dme_distance,
		&_outside_air_temperature_k,
		&_engine_throttle_pct,
		&_engine_epr,
		&_engine_n1_pct,
		&_engine_n2_pct,
		&_engine_egt_degc,
		&_position_lat,
		&_position_lng,
		&_position_amsl,
		&_wind_from_magnetic_heading,
		&_wind_tas
	};

	for (auto property: properties)
	{
		if (property->valid())
			property->set_nil();
	}

	signal_data_updated();
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

		if (!_input_enabled)
			continue;

		FGInputData* fg_data = reinterpret_cast<FGInputData*> (_input_datagram.data());

#define ASSIGN(unit, x) \
		if (!_##x.is_singular()) \
			_##x.write (1_##unit * fg_data->x##_##unit);

#define ASSIGN_UNITLESS(x) \
		if (!_##x.is_singular()) \
			_##x.write (fg_data->x);

		ASSIGN (ft,   cmd_alt_setting);
		ASSIGN (fpm,  cmd_cbr_setting);
		ASSIGN (kt,   cmd_speed_setting);
		ASSIGN (deg,  cmd_heading_setting);
		ASSIGN (deg,  flight_director_pitch);
		ASSIGN (deg,  flight_director_roll);
		ASSIGN (kt,   ias);
		ASSIGN (kt,   tas);
		ASSIGN (kt,   gs);
		ASSIGN_UNITLESS (mach);
		ASSIGN (kt,   ias_lookahead);
		ASSIGN (kt,   maximum_ias);
		ASSIGN (kt,   minimum_ias);
		ASSIGN_UNITLESS (standard_pressure);
		ASSIGN (ft,   altitude);
		ASSIGN (ft,   altitude_agl);
		ASSIGN (inHg, pressure);
		ASSIGN (fpm,  cbr);
		ASSIGN (deg,  position_lat);
		ASSIGN (deg,  position_lng);
		ASSIGN (ft,   position_amsl);
		ASSIGN (deg,  pitch);
		ASSIGN (deg,  roll);
		ASSIGN (deg,  magnetic_heading);
		ASSIGN (deg,  true_heading);
		ASSIGN (deg,  fpm_alpha);
		ASSIGN (deg,  fpm_beta);
		ASSIGN (deg,  magnetic_track);
		ASSIGN_UNITLESS (navigation_needles_visible);
		ASSIGN (nm,   dme_distance);
		ASSIGN_UNITLESS (slip_skid_g);
		ASSIGN_UNITLESS (outside_air_temperature_k);
		ASSIGN_UNITLESS (engine_throttle_pct);
		ASSIGN_UNITLESS (engine_epr);
		ASSIGN_UNITLESS (engine_n1_pct);
		ASSIGN_UNITLESS (engine_n2_pct);
		ASSIGN_UNITLESS (engine_egt_degc);
		ASSIGN (deg,  wind_from_magnetic_heading);
		ASSIGN (kt,   wind_tas);

#undef ASSIGN_UNITLESS
#undef ASSIGN

		if (!_vertical_deviation.is_singular())
			_vertical_deviation.write (2_deg * fg_data->vertical_deviation_deg);
		if (!_lateral_deviation.is_singular())
			_lateral_deviation.write (2_deg * fg_data->lateral_deviation_deg);

		if (!fg_data->vertical_deviation_ok && !_vertical_deviation.is_singular())
			_vertical_deviation.set_nil();
		if (!fg_data->lateral_deviation_ok && !_lateral_deviation.is_singular())
			_lateral_deviation.set_nil();
		if (!fg_data->navigation_dme_ok && !_dme_distance.is_singular())
			_dme_distance.set_nil();

		if (_outside_air_temperature_k.valid())
			_outside_air_temperature_k.write (*_outside_air_temperature_k + 273.15);
	}

	if (_maximum_ias.valid() && *_maximum_ias < 1_kt)
		_maximum_ias.set_nil();
	if (_minimum_ias.valid() && *_minimum_ias < 1_kt)
		_minimum_ias.set_nil();
	if (_altitude_agl.valid() && *_altitude_agl > 2500_ft)
		_altitude_agl.set_nil();
	// Convert EGT from °F to °C:
	if (_engine_egt_degc.valid())
		_engine_egt_degc.write (5.f / 9.f * (*_engine_egt_degc - 32.f));

	signal_data_updated();

	_timeout_timer->start();
}


void
FlightGearIO::write_output()
{
	if (!_output_enabled)
		return;

	FGOutputData fg_data;

#define ASSIGN(x) \
		if (!_##x.is_singular()) \
			fg_data.x = *_##x;

	ASSIGN (ailerons);
	ASSIGN (elevator);
	ASSIGN (rudder);

#undef ASSIGN

	_output->writeDatagram (reinterpret_cast<const char*> (&fg_data), sizeof (fg_data), QHostAddress (_output_host), _output_port);
}


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
#include <xefis/application/services.h>
#include <xefis/utility/numeric.h>
#include <xefis/utility/text_painter.h>
#include <xefis/utility/qdom.h>

// Local:
#include "flight_gear.h"


FlightGearIO::FlightGearIO (QDomElement const& config)
{
	for (QDomElement& e: config)
	{
		if (e == "input")
		{
			for (QDomElement& e2: e)
			{
				if (e2 == "properties")
				{
					parse_properties (e2, {
						{ "ias", _ias_kt, false },
						{ "ias-lookahead", _ias_lookahead_kt, false },
						{ "ias-minimum", _minimum_ias_kt, false },
						{ "ias-maximum", _maximum_ias_kt, false },
						{ "gs", _gs_kt, false },
						{ "tas", _tas_kt, false },
						{ "mach", _mach, false },
						{ "orientation-pitch", _pitch_deg, false },
						{ "orientation-roll", _roll_deg, false },
						{ "orientation-heading", _heading_deg, false },
						{ "slip-skid", _slip_skid_g, false },
						{ "flight-path-marker-alpha", _fpm_alpha_deg, false },
						{ "flight-path-marker-beta", _fpm_beta_deg, false },
						{ "track", _track_deg, false },
						{ "altitude", _altitude_ft, false },
						{ "altitude-agl", _altitude_agl_ft, false },
						{ "landing-altitude", _landing_altitude_ft, false },
						{ "cbr", _cbr_fpm, false },
						{ "pressure", _pressure_inhg, false },
						{ "autopilot-setting-altitude", _autopilot_alt_setting_ft, false },
						{ "autopilot-setting-ias", _autopilot_speed_setting_kt, false },
						{ "autopilot-setting-heading", _autopilot_heading_setting_deg, false },
						{ "autopilot-setting-cbr", _autopilot_cbr_setting_fpm, false },
						{ "flight-director-pitch", _flight_director_pitch_deg, false },
						{ "flight-director-roll", _flight_director_roll_deg, false },
						{ "navigation-needles-visible", _navigation_needles_visible, false },
						{ "navigation-glide-slope-needle", _navigation_gs_needle, false },
						{ "navigation-heading-needle", _navigation_hd_needle, false },
						{ "dme-distance", _dme_distance_nm, false },
						{ "engine-throttle-pct", _engine_throttle_pct, false },
						{ "engine-epr", _engine_epr, false },
						{ "engine-n1", _engine_n1_pct, false },
						{ "engine-n2", _engine_n2_pct, false },
						{ "engine-egt", _engine_egt_degc, false },
						{ "position-latitude", _position_lat_deg, false },
						{ "position-longitude", _position_lng_deg, false },
						{ "position-sea-level-radius", _position_sea_level_radius_ft, false }
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
	_input->bind (QHostAddress::Any, 9000, QUdpSocket::ShareAddress);
	QObject::connect (_input, SIGNAL (readyRead()), this, SLOT (read_input()));

	_float_vars = {
		{ "ias",	&_ias_kt },
		{ "iasl",	&_ias_lookahead_kt },
		{ "gs",		&_gs_kt },
		{ "tas",	&_tas_kt },
		{ "ma",		&_mach },
		{ "p",		&_pitch_deg },
		{ "r",		&_roll_deg },
		{ "h",		&_heading_deg },
		{ "ss",		&_slip_skid_g },
		{ "fpa",	&_fpm_alpha_deg },
		{ "fpb",	&_fpm_beta_deg },
		{ "tr",		&_track_deg },
		{ "al",		&_altitude_ft },
		{ "cbr",	&_cbr_fpm },
		{ "als",	&_pressure_inhg },
		{ "apa",	&_autopilot_alt_setting_ft },
		{ "ats",	&_autopilot_speed_setting_kt },
		{ "aph",	&_autopilot_heading_setting_deg },
		{ "apc",	&_autopilot_cbr_setting_fpm },
		{ "fdp",	&_flight_director_pitch_deg },
		{ "fdr",	&_flight_director_roll_deg },
		{ "ngs",	&_navigation_gs_needle },
		{ "nhd",	&_navigation_hd_needle },
		{ "thr",	&_engine_throttle_pct },
		{ "epr",	&_engine_epr },
		{ "n1",		&_engine_n1_pct },
		{ "n2",		&_engine_n2_pct },
		{ "slr",	&_position_sea_level_radius_ft }
	};

	invalidate_all();
}


FlightGearIO::~FlightGearIO()
{
	delete _input;
	delete _timeout_timer;
}


void
FlightGearIO::read_input()
{
	invalidate_all();

	while (_input->hasPendingDatagrams())
	{
		QByteArray datagram;
		datagram.resize (_input->pendingDatagramSize());
		QHostAddress sender_host;
		uint16_t sender_port;

		bool navigation_gs_needle_ok = false;
		bool navigation_hd_needle_ok = false;
		bool navigation_dme_ok = false;
		float navigation_gs_needle = 0.f;
		float navigation_hd_needle = 0.f;
		float navigation_dme = 0.f;

		_input->readDatagram (datagram.data(), datagram.size(), &sender_host, &sender_port);

		for (int i = 0; i < datagram.size(); ++i)
			if (datagram[i] == ',' || datagram[i] == '=')
				datagram[i] = 0; // NUL character.

		const char* data = datagram.constData();
		const char* c_var = nullptr;
		std::size_t c_var_len = 0;
		const char* c_value = nullptr;
		std::size_t c_value_len = 0;

		auto is_var = [&](const char* test) -> bool
		{
			return strcmp (c_var, test) == 0;
		};

		for (int i = 0; i < datagram.size(); i += c_var_len + 1 + c_value_len + 1)
		{
			// data is NUL-terminated string.
			c_var = data + i;
			c_var_len = strlen (c_var);
			c_value = c_var + c_var_len + 1;
			c_value_len = strlen (c_value);
			// Value is not present, stop parsing:
			if (c_value >= data + datagram.size())
				break;

			QString value (c_value);

			if (is_var ("iasmi"))
			{
				if (!_minimum_ias_kt.is_singular() && value.toDouble() > 1.f)
					_minimum_ias_kt.write (value.toDouble());
			}
			else if (is_var ("iasma"))
			{
				if (!_maximum_ias_kt.is_singular() && value.toDouble() > 1.f)
					_maximum_ias_kt.write (value.toDouble());
			}
			else if (is_var ("alr"))
			{
				if (!_altitude_agl_ft.is_singular() && value.toDouble() < 2500.f)
					_altitude_agl_ft.write (value.toDouble());
			}
			else if (is_var ("egt"))
			{
				if (!_engine_egt_degc.is_singular())
					_engine_egt_degc.write (5.f / 9.f * (value.toDouble() - 32.f));
			}
			else if (is_var ("nav"))
			{
				if (!_navigation_needles_visible.is_singular())
					_navigation_needles_visible.write (!!value.toInt());
			}
			else if (is_var ("ngso"))
				navigation_gs_needle_ok = !!value.toInt();
			else if (is_var ("ngs"))
				navigation_gs_needle = value.toDouble();
			else if (is_var ("nhdo"))
				navigation_hd_needle_ok = !!value.toInt();
			else if (is_var ("nhd"))
				navigation_hd_needle = value.toDouble();
			else if (is_var ("dok"))
				navigation_dme_ok = !!value.toInt();
			else if (is_var ("dme"))
				navigation_dme = value.toDouble();
			else if (is_var ("lt"))
			{
				if (!_position_lat_deg.is_singular())
				{
					_prev_position_lat_valid = true;
					_prev_position_lat_deg = *_position_lat_deg;
					_position_lat_deg.write (value.toDouble());
				}
			}
			else if (is_var ("ln"))
			{
				if (!_position_lng_deg.is_singular())
				{
					_prev_position_lng_valid = true;
					_prev_position_lng_deg = *_position_lng_deg;
					_position_lng_deg.write (value.toDouble());
				}
			}
			else if (!handle_float_variable (c_var, value))
				std::cerr << "Unknown variable from FlightGear protocol: " << c_var << std::endl;
		}

		if (navigation_gs_needle_ok)
		{
			if (!_navigation_gs_needle.is_singular())
				_navigation_gs_needle.write (navigation_gs_needle);
		}
		if (navigation_hd_needle_ok)
		{
			if (!_navigation_hd_needle.is_singular())
				_navigation_hd_needle.write (navigation_hd_needle);
		}
		if (navigation_dme_ok)
		{
			if (!_dme_distance_nm.is_singular())
				_dme_distance_nm.write (navigation_dme);
		}
	}

	_timeout_timer->start();
}


bool
FlightGearIO::handle_float_variable (QString const& variable, QString const& value)
{
	auto it = _float_vars.find (variable);
	if (it == _float_vars.end())
		return false;

	if (!it->second->is_singular())
		it->second->write (value.toDouble());
	return true;
}


void
FlightGearIO::invalidate_all()
{
	Xefis::BaseProperty* properties[] = {
		&_ias_kt,
		&_ias_lookahead_kt,
		&_minimum_ias_kt,
		&_maximum_ias_kt,
		&_gs_kt,
		&_tas_kt,
		&_mach,
		&_pitch_deg,
		&_roll_deg,
		&_heading_deg,
		&_slip_skid_g,
		&_fpm_alpha_deg,
		&_fpm_beta_deg,
		&_track_deg,
		&_altitude_ft,
		&_altitude_agl_ft,
		&_landing_altitude_ft,
		&_cbr_fpm,
		&_pressure_inhg,
		&_autopilot_alt_setting_ft,
		&_autopilot_speed_setting_kt,
		&_autopilot_heading_setting_deg,
		&_autopilot_cbr_setting_fpm,
		&_flight_director_pitch_deg,
		&_flight_director_roll_deg,
		&_navigation_needles_visible,
		&_navigation_gs_needle,
		&_navigation_hd_needle,
		&_dme_distance_nm,
		&_engine_throttle_pct,
		&_engine_epr,
		&_engine_n1_pct,
		&_engine_n2_pct,
		&_engine_egt_degc,
		&_position_lat_deg,
		&_position_lng_deg,
		&_position_sea_level_radius_ft
	};

	for (auto property: properties)
	{
		if (property->valid())
			property->set_nil();
	}
}


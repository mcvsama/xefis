/* vim:ts=4
 *
 * Copyleft 2008…2012  Michał Gawron
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
						{ "engine-egt", _engine_egt_degc, false }
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
	_input->bind (QHostAddress::LocalHost, 9000, QUdpSocket::ShareAddress);
	QObject::connect (_input, SIGNAL (readyRead()), this, SLOT (read_input()));

	_float_vars = {
		{ "ias",			&_ias_kt },
		{ "ias-tend",		&_ias_lookahead_kt },
		{ "gs",				&_gs_kt },
		{ "tas",			&_tas_kt },
		{ "mach",			&_mach },
		{ "pitch",			&_pitch_deg },
		{ "roll",			&_roll_deg },
		{ "heading",		&_heading_deg },
		{ "ss",				&_slip_skid_g },
		{ "alpha",			&_fpm_alpha_deg },
		{ "beta",			&_fpm_beta_deg },
		{ "track",			&_track_deg },
		{ "altitude",		&_altitude_ft },
		{ "cbr",			&_cbr_fpm },
		{ "altimeter-inhg",	&_pressure_inhg },
		{ "ap-alt-sel",		&_autopilot_alt_setting_ft },
		{ "at-speed-sel",	&_autopilot_speed_setting_kt },
		{ "ap-hdg-sel",		&_autopilot_heading_setting_deg },
		{ "ap-cbr-sel",		&_autopilot_cbr_setting_fpm },
		{ "fd-pitch",		&_flight_director_pitch_deg },
		{ "fd-roll",		&_flight_director_roll_deg },
		{ "nav-gs",			&_navigation_gs_needle },
		{ "nav-hd",			&_navigation_hd_needle },
		{ "thr",			&_engine_throttle_pct },
		{ "epr",			&_engine_epr },
		{ "n1",				&_engine_n1_pct },
		{ "n2",				&_engine_n2_pct }
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

		QString line (datagram);
		for (QString pair: QString (datagram).split (',', QString::SkipEmptyParts))
		{
			QStringList split_pair = pair.split ('=');
			if (split_pair.size() != 2)
				continue;
			QString var = split_pair[0];
			QString value = split_pair[1];

			if (var == "ias-min")
			{
				if (!_minimum_ias_kt.is_singular() && value.toFloat() > 1.f)
					_minimum_ias_kt.write (value.toFloat());
			}
			else if (var == "ias-max")
			{
				if (!_maximum_ias_kt.is_singular() && value.toFloat() > 1.f)
					_maximum_ias_kt.write (value.toFloat());
			}
			else if (var == "alt-agl")
			{
				if (!_altitude_agl_ft.is_singular() && value.toFloat() < 2500.f)
					_altitude_agl_ft.write (value.toFloat());
			}
			else if (var == "egt")
			{
				if (!_engine_egt_degc.is_singular())
					_engine_egt_degc.write (5.f / 9.f * (value.toFloat() - 32.f));
			}
			else if (var == "nav")
			{
				if (!_navigation_needles_visible.is_singular())
					_navigation_needles_visible.write (!!value.toInt());
			}
			else if (var == "nav-gs-ok")
				navigation_gs_needle_ok = !!value.toInt();
			else if (var == "nav-gs")
				navigation_gs_needle = value.toFloat();
			else if (var == "nav-hd-ok")
				navigation_hd_needle_ok = !!value.toInt();
			else if (var == "nav-hd")
				navigation_hd_needle = value.toFloat();
			else if (var == "dme-ok")
				navigation_dme_ok = !!value.toInt();
			else if (var == "dme")
				navigation_dme = value.toFloat();
			else if (!handle_float_variable (var, value))
				std::cerr << "Unknown variable from FlightGear protocol: " << var.toStdString() << std::endl;
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
		it->second->write (value.toFloat());
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
		&_engine_egt_degc
	};

	for (auto property: properties)
	{
		if (property->valid())
			property->set_nil();
	}
}


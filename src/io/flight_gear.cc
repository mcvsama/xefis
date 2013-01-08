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
#include <xefis/application/application.h>
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
	while (_input->hasPendingDatagrams())
	{
		int datagram_size = _input->pendingDatagramSize();
		if (_datagram.size() < datagram_size)
			_datagram.resize (datagram_size);

		bool navigation_gs_needle_ok = false;
		bool navigation_hd_needle_ok = false;
		bool navigation_dme_ok = false;
		float navigation_gs_needle = 0.f;
		float navigation_hd_needle = 0.f;
		float navigation_dme = 0.f;

		_input->readDatagram (_datagram.data(), datagram_size, nullptr, nullptr);

		for (int i = 0; i < datagram_size; ++i)
			if (_datagram[i] == ',' || _datagram[i] == '=')
				_datagram[i] = 0; // NUL character.

		const char* data = _datagram.constData();
		const char* c_value = data;

		auto next_value = [&]() -> QString
		{
			if (c_value < data + datagram_size)
			{
				const char* x = c_value;
				c_value += strlen (c_value) + 1;
				return QString (x);
			}
			else
				return nullptr;
		};

		auto handle_float_var = [](QString const& value, Xefis::PropertyFloat& property) -> void
		{
			if (!property.is_singular())
				property.write (value.toDouble());
		};

		QString value;

		/* apa */	handle_float_var (next_value(), _autopilot_alt_setting_ft);
		/* apc */	handle_float_var (next_value(), _autopilot_cbr_setting_fpm);
		/* ats */	handle_float_var (next_value(), _autopilot_speed_setting_kt);
		/* aph */	handle_float_var (next_value(), _autopilot_heading_setting_deg);
		/* fdp */	handle_float_var (next_value(), _flight_director_pitch_deg);
		/* fdr */	handle_float_var (next_value(), _flight_director_roll_deg);
		/* ias */	handle_float_var (next_value(), _ias_kt);
		/* tas */	handle_float_var (next_value(), _tas_kt);
		/* gs */	handle_float_var (next_value(), _gs_kt);
		/* ma */	handle_float_var (next_value(), _mach);
		/* iasl */	handle_float_var (next_value(), _ias_lookahead_kt);
		/* iasma */	value = next_value();
					if (!_maximum_ias_kt.is_singular() && value.toDouble() > 1.f)
						_maximum_ias_kt.write (value.toDouble());
		/* iasmi */	value = next_value();
					if (!_minimum_ias_kt.is_singular() && value.toDouble() > 1.f)
						_minimum_ias_kt.write (value.toDouble());
		/* al */	handle_float_var (next_value(), _altitude_ft);
		/* alr */	value = next_value();
					if (!_altitude_agl_ft.is_singular() && value.toDouble() < 2500.f)
						_altitude_agl_ft.write (value.toDouble());
		/* als */	handle_float_var (next_value(), _pressure_inhg);
		/* cbr */	handle_float_var (next_value(), _cbr_fpm);
		/* lt */	value = next_value();
					if (!_position_lat_deg.is_singular())
					{
						_prev_position_lat_valid = true;
						_prev_position_lat_deg = *_position_lat_deg;
						_position_lat_deg.write (value.toDouble());
					}
		/* ln */	value = next_value();
					if (!_position_lng_deg.is_singular())
					{
						_prev_position_lng_valid = true;
						_prev_position_lng_deg = *_position_lng_deg;
						_position_lng_deg.write (value.toDouble());
					}
		/* slr */	handle_float_var (next_value(), _position_sea_level_radius_ft);
		/* p */		handle_float_var (next_value(), _pitch_deg);
		/* r */		handle_float_var (next_value(), _roll_deg);
		/* h */		handle_float_var (next_value(), _heading_deg);
		/* fpa */	handle_float_var (next_value(), _fpm_alpha_deg);
		/* fpb */	handle_float_var (next_value(), _fpm_beta_deg);
		/* tr */	handle_float_var (next_value(), _track_deg);
		/* nav */	value = next_value();
					if (!_navigation_needles_visible.is_singular())
						_navigation_needles_visible.write (!!value.toInt());
		/* ngso */	navigation_gs_needle_ok = !!next_value().toInt();
		/* ngs */	navigation_gs_needle = next_value().toDouble();
		/* nhdo */	navigation_hd_needle_ok = !!next_value().toInt();
		/* nhd */	navigation_hd_needle = next_value().toDouble();
		/* dok */	navigation_dme_ok = !!next_value().toInt();
		/* dme */	navigation_dme = next_value().toDouble();
		/* ss */	handle_float_var (next_value(), _slip_skid_g);
		/* thr */	handle_float_var (next_value(), _engine_throttle_pct);
		/* epr */	handle_float_var (next_value(), _engine_epr);
		/* n1 */	handle_float_var (next_value(), _engine_n1_pct);
		/* n2 */	handle_float_var (next_value(), _engine_n2_pct);
		/* egt */	value = next_value();
					if (!_engine_egt_degc.is_singular())
						_engine_egt_degc.write (5.f / 9.f * (value.toDouble() - 32.f));

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

	data_updated();

	_timeout_timer->start();
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


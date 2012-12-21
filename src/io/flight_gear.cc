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

// Local:
#include "flight_gear.h"


FlightGearInput::FlightGearInput (QDomElement const&)
{
	_timeout_timer = new QTimer();
	_timeout_timer->setSingleShot (true);
	_timeout_timer->setInterval (200);
	QObject::connect (_timeout_timer, SIGNAL (timeout()), this, SLOT (invalidate_all()));

	_input = new QUdpSocket();
	_input->bind (QHostAddress::LocalHost, 9000, QUdpSocket::ShareAddress);
	QObject::connect (_input, SIGNAL (readyRead()), this, SLOT (read_input()));

	set_path ("/instrumentation");
	invalidate_all();
}


FlightGearInput::~FlightGearInput()
{
	delete _input;
	delete _timeout_timer;
}


void
FlightGearInput::set_path (QString const& path)
{
	_property_path = path.toStdString();

	_ias_kt = Xefis::PropertyFloat (_property_path + "/speed/ias.kt");
	_ias_tendency_kt = Xefis::PropertyFloat (_property_path + "/speed/ias-lookahead.kt");
	_minimum_ias_kt = Xefis::PropertyFloat (_property_path + "/speed/ias-minimum.kt");
	_maximum_ias_kt = Xefis::PropertyFloat (_property_path + "/speed/ias-maximum.kt");
	_gs_kt = Xefis::PropertyFloat (_property_path + "/speed/gs.kt");
	_tas_kt = Xefis::PropertyFloat (_property_path + "/speed/tas.kt");
	_mach = Xefis::PropertyFloat (_property_path + "/speed/mach");
	_pitch_deg = Xefis::PropertyFloat (_property_path + "/orientation/pitch.deg");
	_roll_deg = Xefis::PropertyFloat (_property_path + "/orientation/roll.deg");
	_heading_deg = Xefis::PropertyFloat (_property_path + "/orientation/heading.deg");
	_slip_skid = Xefis::PropertyFloat (_property_path + "/slip-skid/slip-skid");
	_fpm_alpha_deg = Xefis::PropertyFloat (_property_path + "/flight-path-marker/alpha.deg");
	_fpm_beta_deg = Xefis::PropertyFloat (_property_path + "/flight-path-marker/beta.deg");
	_track_deg = Xefis::PropertyFloat (_property_path + "/flight-path-marker/track.deg");
	_altitude_ft = Xefis::PropertyFloat (_property_path + "/altitude/amsl.ft");
	_altitude_agl_ft = Xefis::PropertyFloat (_property_path + "/altitude/agl.ft");
	_landing_altitude_ft = Xefis::PropertyFloat (_property_path + "/altitude/landing-altitude.ft");
	_pressure_inhg = Xefis::PropertyFloat (_property_path + "/static/pressure.inhg");
	_cbr_fpm = Xefis::PropertyFloat (_property_path + "/cbr/fpm");
	_autopilot_alt_setting_ft = Xefis::PropertyFloat (_property_path + "/autopilot/setting/altitude.ft");
	_autopilot_speed_setting_kt = Xefis::PropertyFloat (_property_path + "/autopilot/setting/speed.kt");
	_autopilot_heading_setting_deg = Xefis::PropertyFloat (_property_path + "/autopilot/setting/heading.deg");
	_autopilot_cbr_setting_fpm = Xefis::PropertyFloat (_property_path + "/autopilot/setting/climb-rate.fpm");
	_flight_director_pitch_deg = Xefis::PropertyFloat (_property_path + "/autopilot/flight-director/pitch.deg");
	_flight_director_roll_deg = Xefis::PropertyFloat (_property_path + "/autopilot/flight-director/roll.deg");
	_navigation_needles_enabled = Xefis::PropertyBoolean (_property_path + "/navigation/enabled");
	_navigation_gs_needle = Xefis::PropertyFloat (_property_path + "/navigation/glide-slope");
	_navigation_hd_needle = Xefis::PropertyFloat (_property_path + "/navigation/heading");
	_dme_distance_nm = Xefis::PropertyFloat (_property_path + "/navigation/dme-distance.nm");
	_engine_throttle_pct = Xefis::PropertyFloat ("/engine/throttle.pct");
	_engine_epr = Xefis::PropertyFloat ("/engine/epr");
	_engine_n1_pct = Xefis::PropertyFloat ("/engine/n1.pct");
	_engine_n2_pct = Xefis::PropertyFloat ("/engine/n2.pct");
	_engine_egt_degc = Xefis::PropertyFloat ("/engine/egt.degc");

	invalidate_all();
}


void
FlightGearInput::read_input()
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

			if (var == "ias")
				_ias_kt.write (value.toFloat());
			else if (var == "ias-tend")
				_ias_tendency_kt.write (value.toFloat());
			else if (var == "ias-min")
			{
				if (value.toFloat() > 1.f)
					_minimum_ias_kt.write (value.toFloat());
			}
			else if (var == "ias-max")
			{
				if (value.toFloat() > 1.f)
					_maximum_ias_kt.write (value.toFloat());
			}
			else if (var == "gs")
				_gs_kt.write (value.toFloat());
			else if (var == "tas")
				_tas_kt.write (value.toFloat());
			else if (var == "mach")
				_mach.write (value.toFloat());
			else if (var == "pitch")
				_pitch_deg.write (value.toFloat());
			else if (var == "roll")
				_roll_deg.write (value.toFloat());
			else if (var == "heading")
				_heading_deg.write (value.toFloat());
			else if (var == "ss")
				_slip_skid.write (value.toFloat());
			else if (var == "alpha")
				_fpm_alpha_deg.write (value.toFloat());
			else if (var == "beta")
				_fpm_beta_deg.write (value.toFloat());
			else if (var == "track")
				_track_deg.write (value.toFloat());
			else if (var == "altitude")
				_altitude_ft.write (value.toFloat());
			else if (var == "alt-agl")
			{
				if (value.toFloat() < 2500.f)
					_altitude_agl_ft.write (value.toFloat());
			}
			else if (var == "altimeter-inhg")
				_pressure_inhg.write (value.toFloat());
			else if (var == "cbr")
				_cbr_fpm.write (value.toFloat());
			else if (var == "ap-alt-sel")
				_autopilot_alt_setting_ft.write (value.toFloat());
			else if (var == "at-speed-sel")
				_autopilot_speed_setting_kt.write (value.toFloat());
			else if (var == "ap-hdg-sel")
				_autopilot_heading_setting_deg.write (value.toFloat());
			else if (var == "ap-cbr-sel")
				_autopilot_cbr_setting_fpm.write (value.toFloat());
			else if (var == "fd-pitch")
				_flight_director_pitch_deg.write (value.toFloat());
			else if (var == "fd-roll")
				_flight_director_roll_deg.write (value.toFloat());
			else if (var == "nav")
				_navigation_needles_enabled.write (!!value.toInt());
			else if (var == "nav-gs")
				navigation_gs_needle = value.toFloat();
			else if (var == "nav-gs-ok")
				navigation_gs_needle_ok = !!value.toInt();
			else if (var == "nav-hd")
				navigation_hd_needle = value.toFloat();
			else if (var == "nav-hd-ok")
				navigation_hd_needle_ok = !!value.toInt();
			else if (var == "dme-ok")
				navigation_dme_ok = !!value.toInt();
			else if (var == "dme")
				navigation_dme = value.toFloat();
			else if (var == "thr")
				_engine_throttle_pct.write (value.toFloat());
			else if (var == "epr")
				_engine_epr.write (value.toFloat());
			else if (var == "n1")
				_engine_n1_pct.write (value.toFloat());
			else if (var == "n2")
				_engine_n2_pct.write (value.toFloat());
			else if (var == "egt")
				_engine_egt_degc.write (5.f / 9.f * (value.toFloat() - 32.f));
		}

		if (navigation_gs_needle_ok)
			_navigation_gs_needle.write (navigation_gs_needle);
		if (navigation_hd_needle_ok)
			_navigation_hd_needle.write (navigation_hd_needle);
		if (navigation_dme_ok)
			_dme_distance_nm.write (navigation_dme);
	}

	_timeout_timer->start();
}


void
FlightGearInput::invalidate_all()
{
	_ias_kt.set_nil();
	_ias_tendency_kt.set_nil();
	_minimum_ias_kt.set_nil();
	_maximum_ias_kt.set_nil();
	_gs_kt.set_nil();
	_tas_kt.set_nil();
	_mach.set_nil();
	_pitch_deg.set_nil();
	_roll_deg.set_nil();
	_heading_deg.set_nil();
	_slip_skid.set_nil();
	_fpm_alpha_deg.set_nil();
	_fpm_beta_deg.set_nil();
	_track_deg.set_nil();
	_altitude_ft.set_nil();
	_altitude_agl_ft.set_nil();
	_landing_altitude_ft.set_nil();
	_pressure_inhg.set_nil();
	_cbr_fpm.set_nil();
	_autopilot_alt_setting_ft.set_nil();
	_autopilot_speed_setting_kt.set_nil();
	_autopilot_heading_setting_deg.set_nil();
	_autopilot_cbr_setting_fpm.set_nil();
	_flight_director_pitch_deg.set_nil();
	_flight_director_roll_deg.set_nil();
	_navigation_needles_enabled.write (false);
	_navigation_gs_needle.set_nil();
	_navigation_hd_needle.set_nil();
	_dme_distance_nm.set_nil();
	_engine_throttle_pct.set_nil();
	_engine_epr.set_nil();
	_engine_n1_pct.set_nil();
	_engine_n2_pct.set_nil();
	_engine_egt_degc.set_nil();
}


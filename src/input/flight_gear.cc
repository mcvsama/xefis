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

// Qt:
#include <QtCore/QTimer>
#include <QtGui/QLayout>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/application/services.h>
#include <xefis/utility/numeric.h>
#include <xefis/utility/text_painter.h>

// Local:
#include "flight_gear.h"


FlightGearInput::FlightGearInput()
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

	_speed_kt = Xefis::Property<float> (_property_path + "/speed/kt");
	_speed_valid = Xefis::Property<bool> (_property_path + "/speed/valid");
	_speed_tendency_ktps = Xefis::Property<float> (_property_path + "/speed/lookahead/ktps");
	_speed_tendency_valid = Xefis::Property<bool> (_property_path + "/speed/lookahead/valid");
	_mach = Xefis::Property<float> (_property_path + "/mach/value");
	_mach_valid = Xefis::Property<bool> (_property_path + "/mach/valid");
	_pitch_deg = Xefis::Property<float> (_property_path + "/pitch/deg");
	_pitch_valid = Xefis::Property<bool> (_property_path + "/pitch/valid");
	_roll_deg = Xefis::Property<float> (_property_path + "/roll/deg");
	_roll_valid = Xefis::Property<bool> (_property_path + "/roll/valid");
	_heading_deg = Xefis::Property<float> (_property_path + "/heading/deg");
	_heading_valid = Xefis::Property<bool> (_property_path + "/heading/valid");
	_fpm_alpha_deg = Xefis::Property<float> (_property_path + "/flight-path-marker/alpha/deg");
	_fpm_alpha_valid = Xefis::Property<bool> (_property_path + "/flight-path-marker/alpha/valid");
	_fpm_beta_deg = Xefis::Property<float> (_property_path + "/flight-path-marker/beta/deg");
	_fpm_beta_valid = Xefis::Property<bool> (_property_path + "/flight-path-marker/beta/valid");
	_altitude_ft = Xefis::Property<float> (_property_path + "/altitude/ft");
	_altitude_valid = Xefis::Property<bool> (_property_path + "/altitude/valid");
	_altitude_agl_ft = Xefis::Property<float> (_property_path + "/altitude/agl/ft");
	_altitude_agl_valid = Xefis::Property<bool> (_property_path + "/altitude/agl/valid");
	_pressure_inhg = Xefis::Property<float> (_property_path + "/pressure/inhg");
	_pressure_valid = Xefis::Property<bool> (_property_path + "/pressure/valid");
	_cbr_fpm = Xefis::Property<float> (_property_path + "/cbr/fpm");
	_cbr_valid = Xefis::Property<bool> (_property_path + "/cbr/valid");
	_autopilot_alt_setting_ft = Xefis::Property<float> (_property_path + "/autopilot/setting/altitude/ft");
	_autopilot_alt_setting_valid = Xefis::Property<bool> (_property_path + "/autopilot/setting/altitude/valid");
	_autopilot_speed_setting_kt = Xefis::Property<float> (_property_path + "/autopilot/setting/speed/kt");
	_autopilot_speed_setting_valid = Xefis::Property<bool> (_property_path + "/autopilot/setting/speed/valid");

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
			{
				_speed_kt.write (value.toFloat());
				_speed_valid.write (true);
			}
			if (var == "ias-tend")
			{
				_speed_tendency_ktps.write (value.toFloat());
				_speed_tendency_valid.write (true);
			}
			if (var == "mach")
			{
				_mach.write (value.toFloat());
				_mach_valid.write (true);
			}
			else if (var == "pitch")
			{
				_pitch_deg.write (value.toFloat());
				_pitch_valid.write (true);
			}
			else if (var == "roll")
			{
				_roll_deg.write (value.toFloat());
				_roll_valid.write (true);
			}
			else if (var == "heading")
			{
				_heading_deg.write (value.toFloat());
				_heading_valid.write (true);
			}
			else if (var == "alpha")
			{
				_fpm_alpha_deg.write (value.toFloat());
				_fpm_alpha_valid.write (true);
			}
			else if (var == "beta")
			{
				_fpm_beta_deg.write (value.toFloat());
				_fpm_beta_valid.write (true);
			}
			else if (var == "altitude")
			{
				_altitude_ft.write (value.toFloat());
				_altitude_valid.write (true);
			}
			else if (var == "alt-agl")
			{
				_altitude_agl_ft.write (value.toFloat());
				_altitude_agl_valid.write (value.toFloat() < 2500.f);
			}
			else if (var == "altimeter-inhg")
			{
				_pressure_inhg.write (value.toFloat());
				_pressure_valid.write (true);
			}
			else if (var == "cbr")
			{
				_cbr_fpm.write (value.toFloat());
				_cbr_valid.write (true);
			}
			else if (var == "ap-alt-sel")
			{
				_autopilot_alt_setting_ft.write (value.toFloat());
				_autopilot_alt_setting_valid.write (true);
			}
			else if (var == "at-speed-sel")
			{
				_autopilot_speed_setting_kt.write (value.toFloat());
				_autopilot_speed_setting_valid.write (true);
			}
		}
	}

	_timeout_timer->start();
}


void
FlightGearInput::invalidate_all()
{
	_speed_valid.write (false);
	_speed_tendency_valid.write (false);
	_mach_valid.write (false);
	_pitch_valid.write (false);
	_roll_valid.write (false);
	_heading_valid.write (false);
	_fpm_alpha_valid.write (false);
	_fpm_beta_valid.write (false);
	_altitude_valid.write (false);
	_altitude_agl_valid.write (false);
	_pressure_valid.write (false);
	_cbr_valid.write (false);
	_autopilot_alt_setting_valid.write (false);
	_autopilot_speed_setting_valid.write (false);
}


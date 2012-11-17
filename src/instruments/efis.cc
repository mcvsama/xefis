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
#include "efis.h"


EFIS::EFIS (QWidget* parent):
	QWidget (parent)
{
	_efis_widget = new EFISWidget (this);

	_input_alert_timer = new QTimer (this);
	_input_alert_timer->setSingleShot (true);
	QObject::connect (_input_alert_timer, SIGNAL (timeout()), this, SLOT (input_timeout()));

	_input_alert_hide_timer = new QTimer (this);
	_input_alert_hide_timer->setSingleShot (true);
	QObject::connect (_input_alert_hide_timer, SIGNAL (timeout()), this, SLOT (input_ok()));

	_input = new QUdpSocket (this);
	_input->bind (QHostAddress::LocalHost, 9000, QUdpSocket::ShareAddress);
	QObject::connect (_input, SIGNAL (readyRead()), this, SLOT (read_input()));

	// Default alert timeout:
	set_input_alert_timeout (0.15f);

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->addWidget (_efis_widget);
}


void
EFIS::set_input_alert_timeout (Seconds timeout)
{
	_input_alert_timeout = timeout;

	if (_input_alert_timeout > 0.f)
		_input_alert_timer->start (_input_alert_timeout * 1000.f);
	else
	{
		_show_input_alert = false;
		_input_alert_timer->stop();
		_input_alert_hide_timer->stop();
		_efis_widget->set_input_alert_visibility (_show_input_alert);
		_efis_widget->update();
	}
}


void
EFIS::read_input()
{
	hide_all();

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
			bool no_control = false;

			QStringList split_pair = pair.split ('=');
			if (split_pair.size() != 2)
				continue;
			QString var = split_pair[0];
			QString value = split_pair[1];

			if (var == "ias")
			{
				_efis_widget->set_speed (value.toFloat());
				_efis_widget->set_speed_visibility (true);
			}
			if (var == "mach")
			{
				_efis_widget->set_mach (value.toFloat());
				_efis_widget->set_mach_visibility (true);
			}
			else if (var == "pitch")
			{
				_efis_widget->set_pitch (value.toFloat());
				_efis_widget->set_pitch_visibility (true);
			}
			else if (var == "roll")
			{
				_efis_widget->set_roll (value.toFloat());
				_efis_widget->set_roll_visibility (true);
			}
			else if (var == "heading")
			{
				_efis_widget->set_heading (value.toFloat());
				_efis_widget->set_heading_visibility (true);
			}
			else if (var == "alpha")
			{
				_efis_widget->set_flight_path_alpha (value.toFloat());
				_efis_widget->set_flight_path_marker_visibility (true);
			}
			else if (var == "beta")
			{
				_efis_widget->set_flight_path_beta (value.toFloat());
				_efis_widget->set_flight_path_marker_visibility (true);
			}
			else if (var == "altitude")
			{
				_efis_widget->set_altitude (value.toFloat());
				_efis_widget->set_altitude_visibility (true);
			}
			else if (var == "altimeter-inhg")
			{
				_efis_widget->set_pressure (value.toFloat());
				_efis_widget->set_pressure_visibility (true);
			}
			else if (var == "cbr")
			{
				_efis_widget->set_climb_rate (value.toFloat());
				_efis_widget->set_climb_rate_visibility (true);
			}
			else if (var == "ap-alt-sel")
				_efis_widget->add_altitude_bug (EFISWidget::AP, value.toFloat());
			else if (var == "at-speed-sel")
				_efis_widget->add_speed_bug (EFISWidget::AT, value.toFloat());
			else
				no_control = true;
//			else if (var == "latitude")
//				set_latitude (value.toFloat());
//			else if (var == "longitude")
//				set_longitude (value.toFloat());
//			else if (var == "time")
//				set_time (value.toInt());

			if (no_control)
			{
				if (_input_alert_timeout > 0.f)
					_input_alert_timer->start (_input_alert_timeout * 1000.f);
			}
			else if (_show_input_alert && !_input_alert_hide_timer->isActive())
				_input_alert_hide_timer->start (350);
		}
	}
}


void
EFIS::input_timeout()
{
	_input_alert_hide_timer->stop();
	_show_input_alert = true;
	_efis_widget->set_input_alert_visibility (_show_input_alert);
	_efis_widget->update();
}


void
EFIS::input_ok()
{
	_show_input_alert = false;
	_efis_widget->set_input_alert_visibility (_show_input_alert);
	_efis_widget->update();
}


void
EFIS::hide_all()
{
	_efis_widget->remove_altitude_bug (EFISWidget::AP);
	_efis_widget->remove_speed_bug (EFISWidget::AT);
	_efis_widget->set_pressure_visibility (false);
	_efis_widget->set_mach_visibility (false);
	_efis_widget->set_flight_path_marker_visibility (false);
	_efis_widget->set_speed_visibility (false);
	_efis_widget->set_altitude_visibility (false);
	_efis_widget->set_climb_rate_visibility (false);
	_efis_widget->set_pitch_visibility (false);
	_efis_widget->set_roll_visibility (false);
	_efis_widget->set_heading_visibility (false);
}


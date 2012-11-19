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

	set_input_alert_timeout (0.15f);
	set_path ("/instrumentation/efis");

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->addWidget (_efis_widget);

	QTimer* t = new QTimer (this);
	t->setInterval (33);
	QObject::connect (t, SIGNAL (timeout()), this, SLOT (read()));
	t->start();
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
EFIS::set_path (QString const& path)
{
	_property_path = path.toStdString();

	_speed_kt = Xefis::Property<float> (_property_path + "/speed/kt");
	_speed_valid = Xefis::Property<bool> (_property_path + "/speed/valid");
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
	_pressure_inhg = Xefis::Property<float> (_property_path + "/pressure/inhg");
	_pressure_valid = Xefis::Property<bool> (_property_path + "/pressure/valid");
	_cbr_fpm = Xefis::Property<float> (_property_path + "/cbr/fpm");
	_cbr_valid = Xefis::Property<bool> (_property_path + "/cbr/valid");
	_autopilot_alt_setting_ft = Xefis::Property<float> (_property_path + "/autopilot/setting/altitude/ft");
	_autopilot_alt_setting_valid = Xefis::Property<bool> (_property_path + "/autopilot/setting/altitude/valid");
	_autopilot_speed_setting_kt = Xefis::Property<float> (_property_path + "/autopilot/setting/speed/kt");
	_autopilot_speed_setting_valid = Xefis::Property<bool> (_property_path + "/autopilot/setting/speed/valid");
}


void
EFIS::read()
{
	_efis_widget->set_speed (*_speed_kt);;
	_efis_widget->set_speed_visibility (*_speed_valid);

	_efis_widget->set_mach (*_mach);
	_efis_widget->set_mach_visibility (*_mach_valid);

	_efis_widget->set_pitch (*_pitch_deg);
	_efis_widget->set_pitch_visibility (*_pitch_valid);

	_efis_widget->set_roll (*_roll_deg);
	_efis_widget->set_roll_visibility (*_roll_valid);

	_efis_widget->set_heading (*_heading_deg);
	_efis_widget->set_heading_visibility (*_heading_valid);

	_efis_widget->set_flight_path_alpha (*_fpm_alpha_deg);
	_efis_widget->set_flight_path_marker_visibility (*_fpm_alpha_valid);

	_efis_widget->set_flight_path_beta (*_fpm_beta_deg);
	_efis_widget->set_flight_path_marker_visibility (*_fpm_beta_valid);

	_efis_widget->set_altitude (*_altitude_ft);
	_efis_widget->set_altitude_visibility (*_altitude_valid);

	_efis_widget->set_pressure (*_pressure_inhg);
	_efis_widget->set_pressure_visibility (*_pressure_valid);

	_efis_widget->set_climb_rate (*_cbr_fpm);
	_efis_widget->set_climb_rate_visibility (*_cbr_valid);

	if (*_autopilot_alt_setting_valid)
		_efis_widget->add_altitude_bug (EFISWidget::AP, *_autopilot_alt_setting_ft);
	else
		_efis_widget->remove_altitude_bug (EFISWidget::AP);

	if (*_autopilot_speed_setting_valid)
		_efis_widget->add_speed_bug (EFISWidget::AT, *_autopilot_speed_setting_kt);
	else
		_efis_widget->remove_speed_bug (EFISWidget::AP);
}


void
EFIS::read_input()
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
			bool no_control = false;

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
			else
				no_control = true;

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
	invalidate_all();

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
EFIS::invalidate_all()
{
	_speed_valid.write (false);
	_mach_valid.write (false);
	_pitch_valid.write (false);
	_roll_valid.write (false);
	_heading_valid.write (false);
	_fpm_alpha_valid.write (false);
	_fpm_beta_valid.write (false);
	_altitude_valid.write (false);
	_pressure_valid.write (false);
	_cbr_valid.write (false);
	_autopilot_alt_setting_valid.write (false);
	_autopilot_speed_setting_valid.write (false);
}


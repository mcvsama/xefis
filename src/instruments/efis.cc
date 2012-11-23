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
	Instrument (parent)
{
	_efis_widget = new EFISWidget (this);

	set_path ("/instrumentation");

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->addWidget (_efis_widget);

	QTimer* t = new QTimer (this);
	t->setInterval (33);
	QObject::connect (t, SIGNAL (timeout()), this, SLOT (read()));
	t->start();
}


void
EFIS::set_path (QString const& path)
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
	_autopilot_ldgalt_setting_ft = Xefis::Property<float> (_property_path + "/autopilot/setting/landing-altitude/ft");
	_autopilot_ldgalt_setting_valid = Xefis::Property<bool> (_property_path + "/autopilot/setting/landing-altitude/valid");
}


void
EFIS::read()
{
	_efis_widget->set_speed (*_speed_kt);;
	_efis_widget->set_speed_visibility (*_speed_valid);

	_efis_widget->set_speed_tendency (*_speed_tendency_ktps);;
	_efis_widget->set_speed_tendency_visibility(*_speed_tendency_valid);

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

	_efis_widget->set_altitude_agl (*_altitude_agl_ft);
	_efis_widget->set_altitude_agl_visibility (*_altitude_agl_valid);

	if (*_autopilot_ldgalt_setting_valid)
		_efis_widget->add_altitude_bug (EFISWidget::LDGALT, *_autopilot_ldgalt_setting_ft);
	else
		_efis_widget->remove_altitude_bug (EFISWidget::LDGALT);

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
		_efis_widget->remove_speed_bug (EFISWidget::AT);
}


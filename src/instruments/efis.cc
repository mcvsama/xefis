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
	_efis_nav_widget = new EFISNavWidget (this);

	set_path ("/instrumentation");

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (0);
	layout->addWidget (_efis_widget, 80);
	layout->addWidget (_efis_nav_widget, 50);

	QTimer* t = new QTimer (this);
	t->setInterval (33);
	QObject::connect (t, SIGNAL (timeout()), this, SLOT (read()));
	t->start();
}


void
EFIS::set_path (QString const& path)
{
	_property_path = path.toStdString();

	_ias_kt = Xefis::Property<float> (_property_path + "/ias/kt");
	_ias_tendency_kt = Xefis::Property<float> (_property_path + "/ias/lookahead/kt");
	_minimum_ias_kt = Xefis::Property<float> (_property_path + "/ias/minimum/kt");
	_maximum_ias_kt = Xefis::Property<float> (_property_path + "/ias/maximum/kt");
	_gs_kt = Xefis::Property<float> (_property_path + "/gs/kt");
	_tas_kt = Xefis::Property<float> (_property_path + "/tas/kt");
	_mach = Xefis::Property<float> (_property_path + "/mach/value");
	_pitch_deg = Xefis::Property<float> (_property_path + "/pitch/deg");
	_roll_deg = Xefis::Property<float> (_property_path + "/roll/deg");
	_heading_deg = Xefis::Property<float> (_property_path + "/heading/deg");
	_fpm_alpha_deg = Xefis::Property<float> (_property_path + "/flight-path-marker/alpha/deg");
	_fpm_beta_deg = Xefis::Property<float> (_property_path + "/flight-path-marker/beta/deg");
	_altitude_ft = Xefis::Property<float> (_property_path + "/altitude/ft");
	_altitude_agl_ft = Xefis::Property<float> (_property_path + "/altitude/agl/ft");
	_landing_altitude_ft = Xefis::Property<float> (_property_path + "/altitude/landing-altitude/ft");
	_pressure_inhg = Xefis::Property<float> (_property_path + "/pressure/inhg");
	_cbr_fpm = Xefis::Property<float> (_property_path + "/cbr/fpm");
	_autopilot_alt_setting_ft = Xefis::Property<float> (_property_path + "/autopilot/setting/altitude/ft");
	_autopilot_speed_setting_kt = Xefis::Property<float> (_property_path + "/autopilot/setting/speed/kt");
	_autopilot_cbr_setting_fpm = Xefis::Property<float> (_property_path + "/autopilot/setting/climb-rate/fpm");
	_flight_director_pitch_deg = Xefis::Property<float> (_property_path + "/autopilot/flight-director/pitch/deg");
	_flight_director_roll_deg = Xefis::Property<float> (_property_path + "/autopilot/flight-director/roll/deg");
	_navigation_needles_enabled = Xefis::Property<bool> (_property_path + "/navigation/enabled");
	_navigation_gs_needle = Xefis::Property<float> (_property_path + "/navigation/glide-slope/value");
	_navigation_hd_needle = Xefis::Property<float> (_property_path + "/navigation/heading/value");
	_dme_distance_nm = Xefis::Property<float> (_property_path + "/navigation/dme/nm");
}


void
EFIS::read()
{
	_efis_widget->set_speed (*_ias_kt);
	_efis_widget->set_speed_visibility (!_ias_kt.is_nil());

	_efis_widget->set_minimum_speed (*_minimum_ias_kt);
	_efis_widget->set_minimum_speed_visibility (!_minimum_ias_kt.is_nil());

	_efis_widget->set_maximum_speed (*_maximum_ias_kt);
	_efis_widget->set_maximum_speed_visibility (!_maximum_ias_kt.is_nil());

	_efis_widget->set_speed_tendency (*_ias_tendency_kt);
	_efis_widget->set_speed_tendency_visibility(!_ias_tendency_kt.is_nil());

	_efis_widget->set_mach (*_mach);
	_efis_widget->set_mach_visibility (!_mach.is_nil());

	_efis_widget->set_pitch (*_pitch_deg);
	_efis_widget->set_pitch_visibility (!_pitch_deg.is_nil());

	_efis_widget->set_roll (*_roll_deg);
	_efis_widget->set_roll_visibility (!_roll_deg.is_nil());

	_efis_widget->set_heading (*_heading_deg);
	_efis_widget->set_heading_visibility (!_heading_deg.is_nil());

	_efis_widget->set_flight_path_alpha (*_fpm_alpha_deg);
	_efis_widget->set_flight_path_marker_visibility (!_fpm_alpha_deg.is_nil());

	_efis_widget->set_flight_path_beta (*_fpm_beta_deg);
	_efis_widget->set_flight_path_marker_visibility (!_fpm_beta_deg.is_nil());

	_efis_widget->set_altitude (*_altitude_ft);
	_efis_widget->set_altitude_visibility (!_altitude_ft.is_nil());

	_efis_widget->set_altitude_agl (*_altitude_agl_ft);
	_efis_widget->set_altitude_agl_visibility (!_altitude_agl_ft.is_nil());

	_efis_widget->set_landing_altitude (*_landing_altitude_ft);
	_efis_widget->set_landing_altitude_visibility (!_landing_altitude_ft.is_nil());

	_efis_widget->set_pressure (*_pressure_inhg);
	_efis_widget->set_pressure_visibility (!_pressure_inhg.is_nil());

	_efis_widget->set_climb_rate (*_cbr_fpm);
	_efis_widget->set_climb_rate_visibility (!_cbr_fpm.is_nil());

	_efis_widget->set_ap_altitude (*_autopilot_alt_setting_ft);
	_efis_widget->set_ap_altitude_visibility (!_autopilot_alt_setting_ft.is_nil());

	_efis_widget->set_at_speed (*_autopilot_speed_setting_kt);
	_efis_widget->set_at_speed_visibility (!_autopilot_speed_setting_kt.is_nil());

	_efis_widget->set_at_speed (*_autopilot_speed_setting_kt);
	_efis_widget->set_at_speed_visibility (!_autopilot_speed_setting_kt.is_nil());

	_efis_widget->set_ap_climb_rate (*_autopilot_cbr_setting_fpm);
	_efis_widget->set_ap_climb_rate_visibility (!_autopilot_cbr_setting_fpm.is_nil());

	_efis_widget->set_flight_director_pitch (*_flight_director_pitch_deg);
	_efis_widget->set_flight_director_pitch_visibility (!_flight_director_pitch_deg.is_nil());

	_efis_widget->set_flight_director_roll (*_flight_director_roll_deg);
	_efis_widget->set_flight_director_roll_visibility (!_flight_director_roll_deg.is_nil());

	_efis_widget->set_navigation_needles_visibility (*_navigation_needles_enabled);
	_efis_widget->set_navigation_hint (*_navigation_needles_enabled ? "ILS" : "");

	_efis_widget->set_dme_distance (*_dme_distance_nm);
	_efis_widget->set_dme_distance_visibility (!_dme_distance_nm.is_nil());

	_efis_widget->set_navigation_glideslope_needle (*_navigation_gs_needle);
	_efis_widget->set_navigation_glideslope_needle_visibility (!_navigation_gs_needle.is_nil());

	_efis_widget->set_navigation_heading_needle (*_navigation_hd_needle);
	_efis_widget->set_navigation_heading_needle_visibility (!_navigation_hd_needle.is_nil());

	_efis_widget->set_navigation_runway_visibility (*_navigation_needles_enabled && !_navigation_hd_needle.is_nil() &&
													!_altitude_agl_ft.is_nil() && *_altitude_agl_ft <= 500.f);

	_efis_nav_widget->set_heading (*_heading_deg);
	_efis_nav_widget->set_heading_visibility (!_heading_deg.is_nil());

	_efis_nav_widget->set_flight_path_beta (*_fpm_beta_deg);
	_efis_nav_widget->set_flight_path_marker_visibility (!_fpm_beta_deg.is_nil());

	_efis_nav_widget->set_ground_speed (*_gs_kt);
	_efis_nav_widget->set_ground_speed_visibility (!_gs_kt.is_nil());

	_efis_nav_widget->set_true_air_speed (*_tas_kt);
	_efis_nav_widget->set_true_air_speed_visibility (!_tas_kt.is_nil());
}


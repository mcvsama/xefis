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


EFIS::EFIS (QDomElement const&, QWidget* parent):
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
EFIS::read()
{
	bool fpm_ok = _fpm_alpha_deg.valid() && _fpm_beta_deg.valid();
	Degrees fpm_alpha = *_fpm_alpha_deg;
	Degrees fpm_beta = *_fpm_beta_deg;
	if (_track_deg.valid() && _roll_deg.valid() && _heading_deg.valid())
	{
		fpm_alpha -= std::sin (*_roll_deg / 180.f * M_PI) * (*_track_deg - *_heading_deg);
		fpm_beta -= std::cos (*_roll_deg / 180.f * M_PI) * (*_track_deg - *_heading_deg);
	}

	_efis_widget->set_speed (*_ias_kt);
	_efis_widget->set_speed_visibility (_ias_kt.valid());

	_efis_widget->set_minimum_speed (*_minimum_ias_kt);
	_efis_widget->set_minimum_speed_visibility (_minimum_ias_kt.valid());

	_efis_widget->set_maximum_speed (*_maximum_ias_kt);
	_efis_widget->set_maximum_speed_visibility (_maximum_ias_kt.valid());

	_efis_widget->set_speed_tendency (*_ias_tendency_kt);
	_efis_widget->set_speed_tendency_visibility(_ias_tendency_kt.valid());

	_efis_widget->set_mach (*_mach);
	_efis_widget->set_mach_visibility (_mach.valid());

	_efis_widget->set_pitch (*_pitch_deg);
	_efis_widget->set_pitch_visibility (_pitch_deg.valid());

	_efis_widget->set_roll (*_roll_deg);
	_efis_widget->set_roll_visibility (_roll_deg.valid());

	_efis_widget->set_heading (*_heading_deg);
	_efis_widget->set_heading_visibility (_heading_deg.valid());

	_efis_widget->set_slip_skid (*_slip_skid);
	_efis_widget->set_slip_skid_visibility (_slip_skid.valid());

	_efis_widget->set_flight_path_alpha (fpm_alpha);
	_efis_widget->set_flight_path_marker_visibility (fpm_ok);

	_efis_widget->set_flight_path_beta (fpm_beta);
	_efis_widget->set_flight_path_marker_visibility (fpm_ok);

	_efis_widget->set_altitude (*_altitude_ft);
	_efis_widget->set_altitude_visibility (_altitude_ft.valid());

	_efis_widget->set_altitude_agl (*_altitude_agl_ft);
	_efis_widget->set_altitude_agl_visibility (_altitude_agl_ft.valid());

	_efis_widget->set_landing_altitude (*_landing_altitude_ft);
	_efis_widget->set_landing_altitude_visibility (_landing_altitude_ft.valid());

	_efis_widget->set_pressure (*_pressure_inhg);
	_efis_widget->set_pressure_visibility (_pressure_inhg.valid());

	_efis_widget->set_climb_rate (*_cbr_fpm);
	_efis_widget->set_climb_rate_visibility (_cbr_fpm.valid());

	_efis_widget->set_ap_altitude (*_autopilot_alt_setting_ft);
	_efis_widget->set_ap_altitude_visibility (_autopilot_alt_setting_ft.valid());

	_efis_widget->set_at_speed (*_autopilot_speed_setting_kt);
	_efis_widget->set_at_speed_visibility (_autopilot_speed_setting_kt.valid());

	_efis_widget->set_ap_climb_rate (*_autopilot_cbr_setting_fpm);
	_efis_widget->set_ap_climb_rate_visibility (_autopilot_cbr_setting_fpm.valid());

	_efis_widget->set_flight_director_pitch (*_flight_director_pitch_deg);
	_efis_widget->set_flight_director_pitch_visibility (_flight_director_pitch_deg.valid());

	_efis_widget->set_flight_director_roll (*_flight_director_roll_deg);
	_efis_widget->set_flight_director_roll_visibility (_flight_director_roll_deg.valid());

	_efis_widget->set_navigation_needles_visibility (*_navigation_needles_enabled);
	_efis_widget->set_navigation_hint (*_navigation_needles_enabled ? "ILS" : "");

	_efis_widget->set_dme_distance (*_dme_distance_nm);
	_efis_widget->set_dme_distance_visibility (_dme_distance_nm.valid());

	_efis_widget->set_navigation_glideslope_needle (*_navigation_gs_needle);
	_efis_widget->set_navigation_glideslope_needle_visibility (_navigation_gs_needle.valid());

	_efis_widget->set_navigation_heading_needle (*_navigation_hd_needle);
	_efis_widget->set_navigation_heading_needle_visibility (_navigation_hd_needle.valid());

	_efis_widget->set_navigation_runway_visibility (*_navigation_needles_enabled && _navigation_hd_needle.valid() &&
													_altitude_agl_ft.valid() && *_altitude_agl_ft <= 500.f);

	_efis_nav_widget->set_heading (*_heading_deg);
	_efis_nav_widget->set_heading_visibility (_heading_deg.valid());

	_efis_nav_widget->set_ap_heading (*_autopilot_heading_setting_deg);
	_efis_nav_widget->set_ap_heading_visibility (_autopilot_heading_setting_deg.valid());

	_efis_nav_widget->set_track (*_track_deg);
	_efis_nav_widget->set_track_visibility (_track_deg.valid());

	_efis_nav_widget->set_ground_speed (*_gs_kt);
	_efis_nav_widget->set_ground_speed_visibility (_gs_kt.valid());

	_efis_nav_widget->set_true_air_speed (*_tas_kt);
	_efis_nav_widget->set_true_air_speed_visibility (_tas_kt.valid());
}


void
EFIS::set_path (QString const& path)
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
}


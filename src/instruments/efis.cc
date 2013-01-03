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

// Qt:
#include <QtCore/QTimer>
#include <QtGui/QLayout>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/application/services.h>
#include <xefis/utility/numeric.h>
#include <xefis/utility/text_painter.h>
#include <xefis/utility/qdom.h>

// Local:
#include "efis.h"


EFIS::EFIS (QDomElement const& config, QWidget* parent):
	Instrument (parent)
{
	for (QDomElement& e: config)
	{
		if (e == "properties")
		{
			parse_properties (e, {
				{ "speed-ladder-line-every", _speed_ladder_line_every, false },
				{ "speed-ladder-number-every", _speed_ladder_number_every, false },
				{ "speed-ladder-extent", _speed_ladder_extent, false },
				{ "altitude-ladder-line-every", _altitude_ladder_line_every, false },
				{ "altitude-ladder-number-every", _altitude_ladder_number_every, false },
				{ "altitude-ladder-bold-every", _altitude_ladder_bold_every, false },
				{ "altitude-ladder-extent", _altitude_ladder_extent, false },
				{ "heading-numbers-visible", _heading_numbers_visible, false },
				{ "ias", _ias_kt, false },
				{ "ias-lookahead", _ias_lookahead_kt, false },
				{ "ias-minimum", _minimum_ias_kt, false },
				{ "ias-warning", _warning_ias_kt, false },
				{ "ias-maximum", _maximum_ias_kt, false },
				{ "gs", _gs_kt, false },
				{ "tas", _tas_kt, false },
				{ "mach", _mach, false },
				{ "orientation-pitch", _pitch_deg, false },
				{ "orientation-roll", _roll_deg, false },
				{ "orientation-roll-limit", _roll_limit_deg, false },
				{ "orientation-heading", _heading_deg, false },
				{ "slip-skid", _slip_skid_g, false },
				{ "slip-skid-limit", _slip_skid_limit_g, false },
				{ "flight-path-marker-visible", _fpm_visible, false },
				{ "flight-path-marker-alpha", _fpm_alpha_deg, false },
				{ "flight-path-marker-beta", _fpm_beta_deg, false },
				{ "track", _track_deg, false },
				{ "altitude", _altitude_ft, false },
				{ "altitude-agl", _altitude_agl_ft, false },
				{ "landing-altitude", _landing_altitude_ft, false },
				{ "pressure", _pressure_inhg, false },
				{ "cbr", _cbr_fpm, false },
				{ "autopilot-visible", _autopilot_visible, false },
				{ "autopilot-setting-altitude", _autopilot_alt_setting_ft, false },
				{ "autopilot-setting-ias", _autopilot_speed_setting_kt, false },
				{ "autopilot-setting-heading", _autopilot_heading_setting_deg, false },
				{ "autopilot-setting-cbr", _autopilot_cbr_setting_fpm, false },
				{ "flight-director-visible", _flight_director_visible, false },
				{ "flight-director-pitch", _flight_director_pitch_deg, false },
				{ "flight-director-roll", _flight_director_roll_deg, false },
				{ "navigation-needles-visible", _navigation_needles_visible, false },
				{ "navigation-type-hint", _navigation_type_hint, false },
				{ "navigation-glide-slope-needle", _navigation_gs_needle, false },
				{ "navigation-heading-needle", _navigation_hd_needle, false },
				{ "dme-distance", _dme_distance_nm, false },
				{ "position-latitude", _position_lat_deg, false },
				{ "position-longitude", _position_lng_deg, false },
				{ "position-sea-level-radius", _position_sea_level_radius_ft, false }
			});
		}
	}

	_efis_widget = new EFISWidget (this);
	_hsi_widget = new HSIWidget (this);

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (0);
	layout->addWidget (_efis_widget, 80);
	layout->addWidget (_hsi_widget, 50);

	QTimer* t = new QTimer (this);
	t->setInterval (50);
	QObject::connect (t, SIGNAL (timeout()), this, SLOT (read()));
	t->start();
}


void
EFIS::read()
{
	estimate_track();

	bool fpm_ok = _fpm_alpha_deg.valid() && _fpm_beta_deg.valid();
	Degrees fpm_alpha = 0.f;
	Degrees fpm_beta = 0.f;
	if (fpm_ok)
	{
		fpm_alpha = *_fpm_alpha_deg;
		fpm_beta = *_fpm_beta_deg;
	}

	if (_track_deg.valid() && _roll_deg.valid() && _heading_deg.valid())
	{
		fpm_alpha -= std::sin (*_roll_deg / 180.f * M_PI) * (*_track_deg - *_heading_deg);
		fpm_beta -= std::cos (*_roll_deg / 180.f * M_PI) * (*_track_deg - *_heading_deg);
	}

	_efis_widget->set_speed_ladder_line_every (_speed_ladder_line_every.valid() ? *_speed_ladder_line_every : 10);
	_efis_widget->set_speed_ladder_number_every (_speed_ladder_number_every.valid() ? *_speed_ladder_number_every : 20);
	_efis_widget->set_speed_ladder_extent (_speed_ladder_extent.valid() ? *_speed_ladder_extent : 124);

	_efis_widget->set_altitude_ladder_line_every (_altitude_ladder_line_every.valid() ? *_altitude_ladder_line_every : 100);
	_efis_widget->set_altitude_ladder_number_every (_altitude_ladder_number_every.valid() ? *_altitude_ladder_number_every : 200);
	_efis_widget->set_altitude_ladder_bold_every (_altitude_ladder_bold_every.valid() ? *_altitude_ladder_bold_every : 500);
	_efis_widget->set_altitude_ladder_extent (_altitude_ladder_extent.valid() ? *_altitude_ladder_extent : 825);

	_efis_widget->set_heading_numbers_visible (_heading_numbers_visible.valid() ? *_heading_numbers_visible : false);

	_efis_widget->set_speed_visible (_ias_kt.valid());
	if (_ias_kt.valid())
		_efis_widget->set_speed (*_ias_kt);

	_efis_widget->set_minimum_speed_visible (_minimum_ias_kt.valid());
	if (_minimum_ias_kt.valid())
		_efis_widget->set_minimum_speed (*_minimum_ias_kt);

	_efis_widget->set_warning_speed_visible (_warning_ias_kt.valid());
	if (_warning_ias_kt.valid())
		_efis_widget->set_warning_speed (*_warning_ias_kt);

	_efis_widget->set_maximum_speed_visible (_maximum_ias_kt.valid());
	if (_maximum_ias_kt.valid())
		_efis_widget->set_maximum_speed (*_maximum_ias_kt);

	_efis_widget->set_speed_tendency_visible (_ias_lookahead_kt.valid());
	if (_ias_lookahead_kt.valid())
		_efis_widget->set_speed_tendency (*_ias_lookahead_kt);

	_efis_widget->set_mach_visible (_mach.valid());
	if (_mach.valid())
		_efis_widget->set_mach (*_mach);

	_efis_widget->set_pitch_visible (_pitch_deg.valid());
	if (_pitch_deg.valid())
		_efis_widget->set_pitch (*_pitch_deg);

	_efis_widget->set_roll_visible (_roll_deg.valid());
	if (_roll_deg.valid())
		_efis_widget->set_roll (*_roll_deg);

	_efis_widget->set_roll_limit (_roll_limit_deg.valid() ? *_roll_limit_deg : 0.f);

	_efis_widget->set_heading_visible (_heading_deg.valid());
	if (_heading_deg.valid())
		_efis_widget->set_heading (*_heading_deg);

	_efis_widget->set_slip_skid_visible (_slip_skid_g.valid());
	if (_slip_skid_g.valid())
		_efis_widget->set_slip_skid (*_slip_skid_g);

	_efis_widget->set_slip_skid_limit (_slip_skid_limit_g.valid() ? *_slip_skid_limit_g : 0.f);

	bool fpm_visible = _fpm_visible.valid() && *_fpm_visible;

	_efis_widget->set_flight_path_marker_visible (fpm_visible && fpm_ok);
	if (fpm_ok)
	{
		_efis_widget->set_flight_path_alpha (fpm_alpha);
		_efis_widget->set_flight_path_beta (fpm_beta);
	}

	_efis_widget->set_altitude_visible (_altitude_ft.valid());
	if (_altitude_ft.valid())
		_efis_widget->set_altitude (*_altitude_ft);

	_efis_widget->set_altitude_agl_visible (_altitude_agl_ft.valid());
	if (_altitude_agl_ft.valid())
		_efis_widget->set_altitude_agl (*_altitude_agl_ft);

	_efis_widget->set_landing_altitude_visible (_landing_altitude_ft.valid());
	if (_landing_altitude_ft.valid())
		_efis_widget->set_landing_altitude (*_landing_altitude_ft);

	_efis_widget->set_pressure_visible (_pressure_inhg.valid());
	if (_pressure_inhg.valid())
		_efis_widget->set_pressure (*_pressure_inhg);

	_efis_widget->set_climb_rate_visible (_cbr_fpm.valid());
	if (_cbr_fpm.valid())
		_efis_widget->set_climb_rate (*_cbr_fpm);

	bool autopilot_visible = _autopilot_visible.valid() && *_autopilot_visible;

	_efis_widget->set_ap_altitude_visible (autopilot_visible && _autopilot_alt_setting_ft.valid());
	if (_autopilot_alt_setting_ft.valid())
		_efis_widget->set_ap_altitude (*_autopilot_alt_setting_ft);

	_efis_widget->set_at_speed_visible (autopilot_visible && _autopilot_speed_setting_kt.valid());
	if (_autopilot_speed_setting_kt.valid())
		_efis_widget->set_at_speed (*_autopilot_speed_setting_kt);

	_efis_widget->set_ap_climb_rate_visible (autopilot_visible && _autopilot_cbr_setting_fpm.valid());
	if (_autopilot_cbr_setting_fpm.valid())
		_efis_widget->set_ap_climb_rate (*_autopilot_cbr_setting_fpm);

	bool flight_director_visible = _flight_director_visible.valid() && *_flight_director_visible;

	_efis_widget->set_flight_director_pitch_visible (flight_director_visible && _flight_director_pitch_deg.valid());
	if (_flight_director_pitch_deg.valid())
		_efis_widget->set_flight_director_pitch (*_flight_director_pitch_deg);

	_efis_widget->set_flight_director_roll_visible (flight_director_visible && _flight_director_roll_deg.valid());
	if (_flight_director_roll_deg.valid())
		_efis_widget->set_flight_director_roll (*_flight_director_roll_deg);

	if (_navigation_needles_visible.valid() && *_navigation_needles_visible)
	{
		_efis_widget->set_navigation_hint (_navigation_type_hint.valid() ? (*_navigation_type_hint).c_str() : "");
		_efis_widget->set_navigation_needles_visible (true);
		_efis_widget->set_navigation_runway_visible (_navigation_hd_needle.valid() &&
													 _altitude_agl_ft.valid() &&
													 *_altitude_agl_ft <= 150.f);
	}
	else
	{
		_efis_widget->set_navigation_needles_visible (false);
		_efis_widget->set_navigation_hint ("");
	}

	_efis_widget->set_dme_distance_visible (_dme_distance_nm.valid());
	if (_dme_distance_nm.valid())
		_efis_widget->set_dme_distance (*_dme_distance_nm);

	_efis_widget->set_navigation_glideslope_needle_visible (_navigation_gs_needle.valid());
	if (_navigation_gs_needle.valid())
		_efis_widget->set_navigation_glideslope_needle (*_navigation_gs_needle);

	_efis_widget->set_navigation_heading_needle_visible (_navigation_hd_needle.valid());
	if (_navigation_hd_needle.valid())
		_efis_widget->set_navigation_heading_needle (*_navigation_hd_needle);

	_hsi_widget->set_heading_visible (_heading_deg.valid());
	if (_heading_deg.valid())
		_hsi_widget->set_heading (*_heading_deg);

	_hsi_widget->set_ap_heading_visible (autopilot_visible && _autopilot_heading_setting_deg.valid());
	if (_autopilot_heading_setting_deg.valid())
		_hsi_widget->set_ap_heading (*_autopilot_heading_setting_deg);

	_hsi_widget->set_track_visible (_track_deg.valid());
	if (_track_deg.valid())
		_hsi_widget->set_track (*_track_deg);

	_hsi_widget->set_ground_speed_visible (_gs_kt.valid());
	if (_gs_kt.valid())
		_hsi_widget->set_ground_speed (*_gs_kt);

	_hsi_widget->set_true_air_speed_visible (_tas_kt.valid());
	if (_tas_kt.valid())
		_hsi_widget->set_true_air_speed (*_tas_kt);
}


void
EFIS::estimate_track()
{
	if (_position_lat_deg.is_singular() || _position_lng_deg.is_singular())
	{
		_hsi_widget->set_track_estimation_visible (false);
		return;
	}

	LatLng current_position (*_position_lat_deg, *_position_lng_deg);

	// Estimate only if the distance between last and current positions is > 0.02nm.
	if (haversine_nm (_positions[0], current_position) > 0.02) // TODO _nm
	{
		// Shift data in _positions:
		for (unsigned int i = _positions.size() - 1; i > 0; i--)
			_positions[i] = _positions[i - 1];
		_positions[0] = current_position;
	}

	double len10 = haversine_nm (_positions[1], _positions[0]);
	Degrees alpha = -180.0 + great_arcs_angle (_positions[2], _positions[1], _positions[0]);
	Degrees beta_per_mile = alpha / len10;

	if (!std::isinf (beta_per_mile) && !std::isnan (beta_per_mile))
		beta_per_mile = _track_estimation_smoother.process (beta_per_mile);

	_hsi_widget->set_track_estimation_visible (true);
	_hsi_widget->set_track_estimation_lookahead (1.f);
	_hsi_widget->set_track_deviation (bound (beta_per_mile, -180.0, +180.0));
}


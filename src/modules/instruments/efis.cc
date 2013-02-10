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
#include <QtWidgets/QLayout>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/application/services.h>
#include <xefis/utility/numeric.h>
#include <xefis/utility/text_painter.h>
#include <xefis/utility/qdom.h>

// Local:
#include "efis.h"


EFIS::EFIS (Xefis::ModuleManager* module_manager, QDomElement const& config, QWidget* parent):
	Instrument (module_manager, parent)
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
				{ "mach", _mach, false },
				{ "orientation-pitch", _pitch_deg, false },
				{ "orientation-roll", _roll_deg, false },
				{ "orientation-roll-limit", _roll_limit_deg, false },
				{ "orientation-magnetic-heading", _mag_heading_deg, false },
				{ "slip-skid", _slip_skid_g, false },
				{ "slip-skid-limit", _slip_skid_limit_g, false },
				{ "flight-path-marker-visible", _fpm_visible, false },
				{ "flight-path-marker-alpha", _fpm_alpha_deg, false },
				{ "flight-path-marker-beta", _fpm_beta_deg, false },
				{ "track", _track_deg, false },
				{ "altitude", _altitude_ft, false },
				{ "altitude-lookahead", _altitude_lookahead_ft, false },
				{ "altitude-agl", _altitude_agl_ft, false },
				{ "landing-altitude", _landing_altitude_ft, false },
				{ "transition-altitude", _transition_altitude_ft, false },
				{ "pressure", _pressure_inhg, false },
				{ "standard-pressure", _standard_pressure, false },
				{ "cbr", _cbr_fpm, false },
				{ "autopilot-visible", _autopilot_visible, false },
				{ "autopilot-setting-altitude", _autopilot_alt_setting_ft, false },
				{ "autopilot-setting-ias", _autopilot_speed_setting_kt, false },
				{ "autopilot-setting-cbr", _autopilot_cbr_setting_fpm, false },
				{ "flight-director-visible", _flight_director_visible, false },
				{ "flight-director-pitch", _flight_director_pitch_deg, false },
				{ "flight-director-roll", _flight_director_roll_deg, false },
				{ "control-stick-visible", _control_stick_visible, false },
				{ "control-stick-pitch", _control_stick_pitch_deg, false },
				{ "control-stick-roll", _control_stick_roll_deg, false },
				{ "navigation-needles-visible", _navigation_needles_visible, false },
				{ "navigation-type-hint", _navigation_type_hint, false },
				{ "navigation-glide-slope-needle", _navigation_gs_needle, false },
				{ "navigation-heading-needle", _navigation_hd_needle, false },
				{ "dme-distance", _dme_distance_nm, false },
				{ "control-hint-visible", _control_hint_visible, false },
				{ "control-hint", _control_hint, false }
			});
		}
	}

	_efis_widget = new EFISWidget (this);

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (0);
	layout->addWidget (_efis_widget);
}


void
EFIS::read()
{
	bool fpm_ok = _fpm_alpha_deg.valid() && _fpm_beta_deg.valid();
	Angle fpm_alpha = 0_deg;
	Angle fpm_beta = 0_deg;
	if (fpm_ok)
	{
		fpm_alpha = 1_deg * *_fpm_alpha_deg;
		fpm_beta = 1_deg * *_fpm_beta_deg;
	}

	if (_track_deg.valid() && _roll_deg.valid() && _mag_heading_deg.valid())
	{
		fpm_alpha -= 1_deg * std::sin ((1_deg * *_roll_deg).rad()) * (*_track_deg - *_mag_heading_deg);
		fpm_beta -= 1_deg * std::cos ((1_deg * *_roll_deg).rad()) * (*_track_deg - *_mag_heading_deg);
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
		_efis_widget->set_pitch (1_deg * *_pitch_deg);

	_efis_widget->set_roll_visible (_roll_deg.valid());
	if (_roll_deg.valid())
		_efis_widget->set_roll (1_deg * *_roll_deg);

	_efis_widget->set_roll_limit (1_deg * (_roll_limit_deg.valid() ? *_roll_limit_deg : 0.f));

	_efis_widget->set_heading_visible (_mag_heading_deg.valid());
	if (_mag_heading_deg.valid())
		_efis_widget->set_heading (1_deg * *_mag_heading_deg);

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

	_efis_widget->set_altitude_tendency_visible (_altitude_lookahead_ft.valid());
	if (_altitude_lookahead_ft.valid())
		_efis_widget->set_altitude_tendency (*_altitude_lookahead_ft);

	_efis_widget->set_altitude_agl_visible (_altitude_agl_ft.valid());
	if (_altitude_agl_ft.valid())
		_efis_widget->set_altitude_agl (*_altitude_agl_ft);

	_efis_widget->set_landing_altitude_visible (_landing_altitude_ft.valid());
	if (_landing_altitude_ft.valid())
		_efis_widget->set_landing_altitude (*_landing_altitude_ft);

	if (_standard_pressure.valid() && _transition_altitude_ft.valid())
	{
		_efis_widget->set_transition_altitude_visible (true);
		_efis_widget->set_standard_pressure (*_standard_pressure);
		_efis_widget->set_transition_altitude (*_transition_altitude_ft);
	}
	else
	{
		_efis_widget->set_transition_altitude_visible (false);
		_efis_widget->set_standard_pressure (false);
	}

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
		_efis_widget->set_flight_director_pitch (1_deg * *_flight_director_pitch_deg);

	_efis_widget->set_flight_director_roll_visible (flight_director_visible && _flight_director_roll_deg.valid());
	if (_flight_director_roll_deg.valid())
		_efis_widget->set_flight_director_roll (1_deg * *_flight_director_roll_deg);

	bool control_stick_visible = _control_stick_visible.valid() && *_control_stick_visible;

	_efis_widget->set_control_stick_visible (control_stick_visible && _control_stick_pitch_deg.valid() && _control_stick_roll_deg.valid());

	if (_control_stick_pitch_deg.valid())
		_efis_widget->set_control_stick_pitch (1_deg * *_control_stick_pitch_deg);

	if (_control_stick_roll_deg.valid())
		_efis_widget->set_control_stick_roll (1_deg * *_control_stick_roll_deg);

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
		_efis_widget->set_dme_distance (1_nm * *_dme_distance_nm);

	_efis_widget->set_navigation_glideslope_needle_visible (_navigation_gs_needle.valid());
	if (_navigation_gs_needle.valid())
		_efis_widget->set_navigation_glideslope_needle (*_navigation_gs_needle);

	_efis_widget->set_navigation_heading_needle_visible (_navigation_hd_needle.valid());
	if (_navigation_hd_needle.valid())
		_efis_widget->set_navigation_heading_needle (*_navigation_hd_needle);

	_efis_widget->set_control_hint_visible (_control_hint_visible.valid() && *_control_hint_visible);
	_efis_widget->set_control_hint (_control_hint.valid() ? (*_control_hint).c_str() : "");
}


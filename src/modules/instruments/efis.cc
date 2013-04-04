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
				{ "speed-ladder-minimum", _speed_ladder_minimum, false },
				{ "speed-ladder-maximum", _speed_ladder_maximum, false },
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
				{ "orientation-pitch-limit", _pitch_limit_deg, false },
				{ "orientation-magnetic-heading", _magnetic_heading_deg, false },
				{ "orientation-true-heading", _true_heading_deg, false },
				{ "slip-skid", _slip_skid_g, false },
				{ "slip-skid-limit", _slip_skid_limit_g, false },
				{ "flight-path-marker-visible", _fpm_visible, false },
				{ "flight-path-marker-alpha", _fpm_alpha_deg, false },
				{ "flight-path-marker-beta", _fpm_beta_deg, false },
				{ "altitude", _altitude_ft, false },
				{ "altitude-lookahead", _altitude_lookahead_ft, false },
				{ "altitude-agl", _altitude_agl_ft, false },
				{ "transition-altitude", _transition_altitude_ft, false },
				{ "pressure", _pressure_inhg, false },
				{ "pressure-display-hpa", _pressure_display_hpa, false },
				{ "standard-pressure", _standard_pressure, false },
				{ "cbr", _cbr_fpm, false },
				{ "cmd-settings-visible", _cmd_settings_visible, false },
				{ "cmd-setting-altitude", _cmd_alt_setting_ft, false },
				{ "cmd-setting-ias", _cmd_speed_setting_kt, false },
				{ "cmd-setting-cbr", _cmd_cbr_setting_fpm, false },
				{ "flight-director-visible", _flight_director_visible, false },
				{ "flight-director-pitch", _flight_director_pitch_deg, false },
				{ "flight-director-roll", _flight_director_roll_deg, false },
				{ "control-stick-visible", _control_stick_visible, false },
				{ "control-stick-pitch", _control_stick_pitch_deg, false },
				{ "control-stick-roll", _control_stick_roll_deg, false },
				{ "approach-reference-visible", _approach_reference_visible, false },
				{ "approach-type-hint", _approach_type_hint, false },
				{ "vertical-deviation", _vertical_deviation_deg, false },
				{ "lateral-deviation", _lateral_deviation_deg, false },
				{ "dme-distance", _dme_distance_nm, false },
				{ "control-hint-visible", _control_hint_visible, false },
				{ "control-hint", _control_hint, false },
				{ "fma-visible", _fma_visible, false },
				{ "fma-speed-hint", _fma_speed_hint, false },
				{ "fma-speed-small-hint", _fma_speed_small_hint, false },
				{ "fma-lateral-hint", _fma_lateral_hint, false },
				{ "fma-lateral-small-hint", _fma_lateral_small_hint, false },
				{ "fma-vertical-hint", _fma_vertical_hint, false },
				{ "fma-vertical-small-hint", _fma_vertical_small_hint, false },
				{ "localizer-id", _localizer_id, false },
				{ "novspd-flag", _novspd_flag, false },
				{ "speed-v1", _speed_v1_kt, false },
				{ "speed-vr", _speed_vr_kt, false },
				{ "speed-ref", _speed_ref_kt, false }
			});
		}
	}

	_efis_widget = new EFISWidget (this, work_performer());

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (0);
	layout->addWidget (_efis_widget);
}


void
EFIS::read()
{
	_efis_widget->set_speed_ladder_line_every (_speed_ladder_line_every.read (10));
	_efis_widget->set_speed_ladder_number_every (_speed_ladder_number_every.read (20));
	_efis_widget->set_speed_ladder_extent (_speed_ladder_extent.read (124));
	_efis_widget->set_speed_ladder_minimum (_speed_ladder_minimum.read (0));
	_efis_widget->set_speed_ladder_maximum (_speed_ladder_maximum.read (9999));

	_efis_widget->set_altitude_ladder_line_every (_altitude_ladder_line_every.valid() ? *_altitude_ladder_line_every : 100);
	_efis_widget->set_altitude_ladder_number_every (_altitude_ladder_number_every.valid() ? *_altitude_ladder_number_every : 200);
	_efis_widget->set_altitude_ladder_bold_every (_altitude_ladder_bold_every.valid() ? *_altitude_ladder_bold_every : 500);
	_efis_widget->set_altitude_ladder_extent (_altitude_ladder_extent.valid() ? *_altitude_ladder_extent : 825);

	_efis_widget->set_heading_numbers_visible (_heading_numbers_visible.valid() ? *_heading_numbers_visible : false);

	_efis_widget->set_speed_visible (_ias_kt.valid());
	if (_ias_kt.valid())
		_efis_widget->set_speed (1_kt * *_ias_kt);

	_efis_widget->set_minimum_speed_visible (_minimum_ias_kt.valid());
	if (_minimum_ias_kt.valid())
		_efis_widget->set_minimum_speed (1_kt * *_minimum_ias_kt);

	_efis_widget->set_warning_speed_visible (_warning_ias_kt.valid());
	if (_warning_ias_kt.valid())
		_efis_widget->set_warning_speed (1_kt * *_warning_ias_kt);

	_efis_widget->set_maximum_speed_visible (_maximum_ias_kt.valid());
	if (_maximum_ias_kt.valid())
		_efis_widget->set_maximum_speed (1_kt * *_maximum_ias_kt);

	_efis_widget->set_speed_tendency_visible (_ias_lookahead_kt.valid());
	if (_ias_lookahead_kt.valid())
		_efis_widget->set_speed_tendency (1_kt * *_ias_lookahead_kt);

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

	if (_pitch_limit_deg.valid())
	{
		_efis_widget->set_pitch_limit (1_deg * *_pitch_limit_deg);
		_efis_widget->set_pitch_limit_visible (true);
	}
	else
		_efis_widget->set_pitch_limit_visible (false);

	_efis_widget->set_heading_visible (_magnetic_heading_deg.valid());
	if (_magnetic_heading_deg.valid())
		_efis_widget->set_heading (1_deg * *_magnetic_heading_deg);

	_efis_widget->set_slip_skid_visible (_slip_skid_g.valid());
	if (_slip_skid_g.valid())
		_efis_widget->set_slip_skid (*_slip_skid_g);

	_efis_widget->set_slip_skid_limit (_slip_skid_limit_g.valid() ? *_slip_skid_limit_g : 0.f);

	bool fpm_visible = _fpm_visible.valid() && *_fpm_visible;

	if (_fpm_alpha_deg.valid() && _fpm_beta_deg.valid())
	{
		_efis_widget->set_flight_path_alpha (1_deg * *_fpm_alpha_deg);
		_efis_widget->set_flight_path_beta (1_deg * *_fpm_beta_deg);
		_efis_widget->set_flight_path_marker_visible (fpm_visible);
	}
	else
		_efis_widget->set_flight_path_marker_visible (false);

	_efis_widget->set_altitude_visible (_altitude_ft.valid());
	if (_altitude_ft.valid())
		_efis_widget->set_altitude (1_ft * *_altitude_ft);

	_efis_widget->set_altitude_tendency_visible (_altitude_lookahead_ft.valid());
	if (_altitude_lookahead_ft.valid())
		_efis_widget->set_altitude_tendency (1_ft * *_altitude_lookahead_ft);

	_efis_widget->set_altitude_agl_visible (_altitude_agl_ft.valid());
	if (_altitude_agl_ft.valid())
		_efis_widget->set_altitude_agl (1_ft * *_altitude_agl_ft);

	_efis_widget->set_altitude_warnings_visible (true);

	if (_standard_pressure.valid() && _transition_altitude_ft.valid())
	{
		_efis_widget->set_transition_altitude_visible (true);
		_efis_widget->set_standard_pressure (*_standard_pressure);
		_efis_widget->set_transition_altitude (1_ft * *_transition_altitude_ft);
	}
	else
	{
		_efis_widget->set_transition_altitude_visible (false);
		_efis_widget->set_standard_pressure (false);
	}

	_efis_widget->set_pressure_visible (_pressure_inhg.valid());
	if (_pressure_inhg.valid())
		_efis_widget->set_pressure (1_inhg * *_pressure_inhg);

	if (_pressure_display_hpa.valid())
		_efis_widget->set_pressure_display_hpa (*_pressure_display_hpa);

	_efis_widget->set_climb_rate_visible (_cbr_fpm.valid());
	if (_cbr_fpm.valid())
		_efis_widget->set_climb_rate (1_fpm * *_cbr_fpm);

	bool cmd_visible = _cmd_settings_visible.read (false);

	_efis_widget->set_cmd_altitude_visible (cmd_visible && _cmd_alt_setting_ft.valid());
	if (_cmd_alt_setting_ft.valid())
		_efis_widget->set_cmd_altitude (1_ft * *_cmd_alt_setting_ft);

	_efis_widget->set_cmd_speed_visible (cmd_visible && _cmd_speed_setting_kt.valid());
	if (_cmd_speed_setting_kt.valid())
		_efis_widget->set_cmd_speed (1_kt * *_cmd_speed_setting_kt);

	_efis_widget->set_cmd_climb_rate_visible (cmd_visible && _cmd_cbr_setting_fpm.valid());
	if (_cmd_cbr_setting_fpm.valid())
		_efis_widget->set_cmd_climb_rate (1_fpm * *_cmd_cbr_setting_fpm);

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

	if (_approach_reference_visible.valid() && *_approach_reference_visible)
	{
		_efis_widget->set_approach_hint (_approach_type_hint.read ("").c_str());
		_efis_widget->set_approach_reference_visible (true);
		if (_altitude_agl_ft.valid())
		{
			_efis_widget->set_runway_visible (_lateral_deviation_deg.valid() &&
											  *_altitude_agl_ft <= 1000.f);
			_efis_widget->set_runway_position (limit<float> (*_altitude_agl_ft, 0.f, 250.f) / 250.f * 25_deg);
		}
	}
	else
	{
		_efis_widget->set_approach_reference_visible (false);
		_efis_widget->set_approach_hint ("");
	}

	_efis_widget->set_dme_distance_visible (_dme_distance_nm.valid());
	if (_dme_distance_nm.valid())
		_efis_widget->set_dme_distance (1_nm * *_dme_distance_nm);

	if (_localizer_id.valid() && _true_heading_deg.valid() && _magnetic_heading_deg.valid())
	{
		Xefis::Navaid const* navaid = navaid_storage()->find_by_id (Xefis::Navaid::LOC, (*_localizer_id).c_str());
		if (navaid)
		{
			_efis_widget->set_localizer_id ((*_localizer_id).c_str());
			_efis_widget->set_localizer_magnetic_bearing (1_deg * (*_magnetic_heading_deg - *_true_heading_deg) + navaid->true_bearing());
			_efis_widget->set_localizer_info_visible (true);
		}
	}
	else
		_efis_widget->set_localizer_info_visible (false);

	_efis_widget->set_novspd_flag (_novspd_flag.read (false));

	if (_speed_v1_kt.valid())
		_efis_widget->add_speed_bug ("V1", 1_kt * *_speed_v1_kt);
	else
		_efis_widget->remove_speed_bug ("V1");

	if (_speed_vr_kt.valid())
		_efis_widget->add_speed_bug ("VR", 1_kt * *_speed_vr_kt);
	else
		_efis_widget->remove_speed_bug ("VR");

	if (_speed_ref_kt.valid())
		_efis_widget->add_speed_bug ("REF", 1_kt * *_speed_ref_kt);
	else
		_efis_widget->remove_speed_bug ("REF");

	_efis_widget->set_vertical_deviation_visible (_vertical_deviation_deg.valid());
	if (_vertical_deviation_deg.valid())
		_efis_widget->set_vertical_deviation (*_vertical_deviation_deg * 1_deg);

	_efis_widget->set_lateral_deviation_visible (_lateral_deviation_deg.valid());
	if (_lateral_deviation_deg.valid())
		_efis_widget->set_lateral_deviation (*_lateral_deviation_deg * 1_deg);

	_efis_widget->set_control_hint_visible (_control_hint_visible.read (false));
	_efis_widget->set_control_hint (_control_hint.read ("").c_str());

	_efis_widget->set_fma_visible (_fma_visible.read (false));
	_efis_widget->set_fma_speed_hint (_fma_speed_hint.read ("").c_str());
	_efis_widget->set_fma_speed_small_hint (_fma_speed_small_hint.read ("").c_str());
	_efis_widget->set_fma_lateral_hint (_fma_lateral_hint.read ("").c_str());
	_efis_widget->set_fma_lateral_small_hint (_fma_lateral_small_hint.read ("").c_str());
	_efis_widget->set_fma_vertical_hint (_fma_vertical_hint.read ("").c_str());
	_efis_widget->set_fma_vertical_small_hint (_fma_vertical_small_hint.read ("").c_str());
}


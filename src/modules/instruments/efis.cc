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
#include <xefis/core/services.h>
#include <xefis/utility/numeric.h>
#include <xefis/utility/painter.h>
#include <xefis/utility/qdom.h>

// Local:
#include "efis.h"


XEFIS_REGISTER_MODULE_CLASS ("instruments/efis", EFIS);


EFIS::EFIS (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Instrument (module_manager)
{
	for (QDomElement& e: config)
	{
		if (e == "properties")
		{
			parse_properties (e, {
				{ "speed-ladder.line-every", _speed_ladder_line_every, false },
				{ "speed-ladder.number-every", _speed_ladder_number_every, false },
				{ "speed-ladder.extent", _speed_ladder_extent, false },
				{ "speed-ladder.minimum", _speed_ladder_minimum, false },
				{ "speed-ladder.maximum", _speed_ladder_maximum, false },
				{ "speed-ladder.novspd-flag", _novspd_flag, false },
				{ "altitude-ladder.line-every", _altitude_ladder_line_every, false },
				{ "altitude-ladder.number-every", _altitude_ladder_number_every, false },
				{ "altitude-ladder.bold-every", _altitude_ladder_bold_every, false },
				{ "altitude-ladder.extent", _altitude_ladder_extent, false },
				{ "speed.ias", _ias, false },
				{ "speed.ias.lookahead", _ias_lookahead, false },
				{ "speed.ias.minimum", _minimum_ias, false },
				{ "speed.ias.warning", _warning_ias, false },
				{ "speed.ias.maximum", _maximum_ias, false },
				{ "speed.mach", _mach, false },
				{ "speed.v1", _speed_v1, false },
				{ "speed.vr", _speed_vr, false },
				{ "speed.ref", _speed_ref, false },
				{ "orientation.pitch", _pitch, false },
				{ "orientation.roll", _roll, false },
				{ "orientation.roll.limit", _roll_limit, false },
				{ "orientation.heading.magnetic", _magnetic_heading, false },
				{ "orientation.heading.true", _true_heading, false },
				{ "orientation.heading.numbers-visible", _heading_numbers_visible, false },
				{ "slip-skid", _slip_skid_g, false },
				{ "slip-skid.limit", _slip_skid_limit_g, false },
				{ "fpm.visible", _fpm_visible, false },
				{ "fpm.alpha", _fpm_alpha, false },
				{ "fpm.beta", _fpm_beta, false },
				{ "aoa.alpha", _aoa_alpha, false },
				{ "aoa.warning-threshold", _aoa_warning_threshold, false },
				{ "aoa.pitch-limit", _pitch_limit, false },
				{ "aoa.pitch-limit.visible", _pitch_limit_visible, false },
				{ "altitude", _altitude, false },
				{ "altitude.lookahead", _altitude_lookahead, false },
				{ "altitude.agl", _altitude_agl, false },
				{ "altitude.cbr", _cbr, false },
				{ "altitude.variometer", _variometer, false },
				{ "altitude.minimums", _minimums_altitude, false },
				{ "pressure.qnh", _pressure_qnh, false },
				{ "pressure.display-hpa", _pressure_display_hpa, false },
				{ "pressure.use-std", _use_standard_pressure, false },
				{ "cmd.visible", _cmd_settings_visible, false },
				{ "cmd.altitude", _cmd_alt_setting, false },
				{ "cmd.ias", _cmd_speed_setting, false },
				{ "cmd.cbr", _cmd_cbr_setting, false },
				{ "flight-director.visible", _flight_director_visible, false },
				{ "flight-director.pitch", _flight_director_pitch, false },
				{ "flight-director.roll", _flight_director_roll, false },
				{ "control-stick.visible", _control_stick_visible, false },
				{ "control-stick.pitch", _control_stick_pitch, false },
				{ "control-stick.roll", _control_stick_roll, false },
				{ "approach.reference-visible", _approach_reference_visible, false },
				{ "approach.type-hint", _approach_type_hint, false },
				{ "approach.localizer-id", _localizer_id, false },
				{ "approach.deviation.vertical", _vertical_deviation, false },
				{ "approach.deviation.lateral", _lateral_deviation, false },
				{ "approach.dme-distance", _dme_distance, false },
				{ "control-hint.visible", _control_hint_visible, false },
				{ "control-hint", _control_hint, false },
				{ "fma.visible", _fma_visible, false },
				{ "fma.speed-hint", _fma_speed_hint, false },
				{ "fma.speed-small-hint", _fma_speed_small_hint, false },
				{ "fma.lateral-hint", _fma_lateral_hint, false },
				{ "fma.lateral-small-hint", _fma_lateral_small_hint, false },
				{ "fma.vertical-hint", _fma_vertical_hint, false },
				{ "fma.vertical-small-hint", _fma_vertical_small_hint, false },
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

	_efis_widget->set_speed_visible (_ias.valid());
	if (_ias.valid())
		_efis_widget->set_speed (*_ias);

	_efis_widget->set_minimum_speed_visible (_minimum_ias.valid());
	if (_minimum_ias.valid())
		_efis_widget->set_minimum_speed (*_minimum_ias);

	_efis_widget->set_warning_speed_visible (_warning_ias.valid());
	if (_warning_ias.valid())
		_efis_widget->set_warning_speed (*_warning_ias);

	_efis_widget->set_maximum_speed_visible (_maximum_ias.valid());
	if (_maximum_ias.valid())
		_efis_widget->set_maximum_speed (*_maximum_ias);

	_efis_widget->set_speed_tendency_visible (_ias_lookahead.valid());
	if (_ias_lookahead.valid())
		_efis_widget->set_speed_tendency (*_ias_lookahead);

	_efis_widget->set_mach_visible (_mach.valid());
	if (_mach.valid())
		_efis_widget->set_mach (*_mach);

	_efis_widget->set_pitch_visible (_pitch.valid());
	if (_pitch.valid())
		_efis_widget->set_pitch (*_pitch);

	_efis_widget->set_roll_visible (_roll.valid());
	if (_roll.valid())
		_efis_widget->set_roll (*_roll);

	_efis_widget->set_roll_limit (_roll_limit.valid() ? *_roll_limit : 0_deg);

	if (_pitch_limit.valid() && _pitch_limit_visible.valid())
	{
		_efis_widget->set_pitch_limit (*_pitch_limit);
		bool visible = false;
		if (_aoa_warning_threshold.valid() && _aoa_alpha.valid() && _pitch.valid())
			visible = *_aoa_alpha > *_aoa_warning_threshold;
		else
			visible = *_pitch_limit_visible;
		_efis_widget->set_pitch_limit_visible (visible);
	}
	else
		_efis_widget->set_pitch_limit_visible (false);

	_efis_widget->set_heading_visible (_magnetic_heading.valid());
	if (_magnetic_heading.valid())
		_efis_widget->set_heading (*_magnetic_heading);

	_efis_widget->set_slip_skid_visible (_slip_skid_g.valid());
	if (_slip_skid_g.valid())
		_efis_widget->set_slip_skid (*_slip_skid_g);

	_efis_widget->set_slip_skid_limit (_slip_skid_limit_g.valid() ? *_slip_skid_limit_g : 0.f);

	bool fpm_visible = _fpm_visible.valid() && *_fpm_visible;

	if (_fpm_alpha.valid() && _fpm_beta.valid())
	{
		_efis_widget->set_flight_path_alpha (*_fpm_alpha);
		_efis_widget->set_flight_path_beta (*_fpm_beta);
		_efis_widget->set_flight_path_marker_visible (fpm_visible);
	}
	else
		_efis_widget->set_flight_path_marker_visible (false);

	_efis_widget->set_altitude_visible (_altitude.valid());
	if (_altitude.valid())
		_efis_widget->set_altitude (*_altitude);

	_efis_widget->set_altitude_tendency_visible (_altitude_lookahead.valid());
	if (_altitude_lookahead.valid())
		_efis_widget->set_altitude_tendency (*_altitude_lookahead);

	_efis_widget->set_altitude_agl_visible (_altitude_agl.valid());
	if (_altitude_agl.valid())
		_efis_widget->set_altitude_agl (*_altitude_agl);

	_efis_widget->set_altitude_warnings_visible (true);

	if (_use_standard_pressure.valid() && _minimums_altitude.valid())
	{
		_efis_widget->set_minimums_altitude_visible (true);
		_efis_widget->set_standard_pressure (*_use_standard_pressure);
		_efis_widget->set_minimums_altitude (*_minimums_altitude);
	}
	else
	{
		_efis_widget->set_minimums_altitude_visible (false);
		_efis_widget->set_standard_pressure (false);
	}

	_efis_widget->set_pressure_visible (_pressure_qnh.valid());
	if (_pressure_qnh.valid())
		_efis_widget->set_pressure (*_pressure_qnh);

	if (_pressure_display_hpa.valid())
		_efis_widget->set_pressure_display_hpa (*_pressure_display_hpa);

	_efis_widget->set_climb_rate_visible (_cbr.valid());
	if (_cbr.valid())
		_efis_widget->set_climb_rate (*_cbr);

	_efis_widget->set_variometer_visible (_variometer.valid());
	if (_variometer.valid())
		_efis_widget->set_variometer_rate (*_variometer);

	bool cmd_visible = _cmd_settings_visible.read (false);

	_efis_widget->set_cmd_altitude_visible (cmd_visible && _cmd_alt_setting.valid());
	if (_cmd_alt_setting.valid())
		_efis_widget->set_cmd_altitude (*_cmd_alt_setting);

	_efis_widget->set_cmd_speed_visible (cmd_visible && _cmd_speed_setting.valid());
	if (_cmd_speed_setting.valid())
		_efis_widget->set_cmd_speed (*_cmd_speed_setting);

	_efis_widget->set_cmd_climb_rate_visible (cmd_visible && _cmd_cbr_setting.valid());
	if (_cmd_cbr_setting.valid())
		_efis_widget->set_cmd_climb_rate (*_cmd_cbr_setting);

	bool flight_director_visible = _flight_director_visible.valid() && *_flight_director_visible;

	_efis_widget->set_flight_director_pitch_visible (flight_director_visible && _flight_director_pitch.valid());
	if (_flight_director_pitch.valid())
		_efis_widget->set_flight_director_pitch (*_flight_director_pitch);

	_efis_widget->set_flight_director_roll_visible (flight_director_visible && _flight_director_roll.valid());
	if (_flight_director_roll.valid())
		_efis_widget->set_flight_director_roll (*_flight_director_roll);

	bool control_stick_visible = _control_stick_visible.valid() && *_control_stick_visible;

	_efis_widget->set_control_stick_visible (control_stick_visible && _control_stick_pitch.valid() && _control_stick_roll.valid());

	if (_control_stick_pitch.valid())
		_efis_widget->set_control_stick_pitch (*_control_stick_pitch);

	if (_control_stick_roll.valid())
		_efis_widget->set_control_stick_roll (*_control_stick_roll);

	if (_approach_reference_visible.valid() && *_approach_reference_visible)
	{
		_efis_widget->set_approach_hint (_approach_type_hint.read ("").c_str());
		_efis_widget->set_approach_reference_visible (true);
		if (_altitude_agl.valid())
		{
			_efis_widget->set_runway_visible (_lateral_deviation.valid() && *_altitude_agl <= 1000_ft);
			_efis_widget->set_runway_position (Xefis::limit<Length> (*_altitude_agl, 0_ft, 250_ft) / 250_ft * 25_deg);
		}
	}
	else
	{
		_efis_widget->set_approach_reference_visible (false);
		_efis_widget->set_approach_hint ("");
	}

	_efis_widget->set_dme_distance_visible (_dme_distance.valid());
	if (_dme_distance.valid())
		_efis_widget->set_dme_distance (*_dme_distance);

	if (_localizer_id.valid() && _true_heading.valid() && _magnetic_heading.valid())
	{
		Xefis::Navaid const* navaid = navaid_storage()->find_by_id (Xefis::Navaid::LOC, (*_localizer_id).c_str());
		if (navaid)
		{
			_efis_widget->set_localizer_id ((*_localizer_id).c_str());
			_efis_widget->set_localizer_magnetic_bearing (*_magnetic_heading - *_true_heading + navaid->true_bearing());
			_efis_widget->set_localizer_info_visible (true);
		}
	}
	else
		_efis_widget->set_localizer_info_visible (false);

	_efis_widget->set_novspd_flag (_novspd_flag.read (false));

	if (_speed_v1.valid())
		_efis_widget->add_speed_bug ("V1", *_speed_v1);
	else
		_efis_widget->remove_speed_bug ("V1");

	if (_speed_vr.valid())
		_efis_widget->add_speed_bug ("VR", *_speed_vr);
	else
		_efis_widget->remove_speed_bug ("VR");

	if (_speed_ref.valid())
		_efis_widget->add_speed_bug ("REF", *_speed_ref);
	else
		_efis_widget->remove_speed_bug ("REF");

	_efis_widget->set_vertical_deviation_visible (_vertical_deviation.valid());
	if (_vertical_deviation.valid())
		_efis_widget->set_vertical_deviation (*_vertical_deviation);

	_efis_widget->set_lateral_deviation_visible (_lateral_deviation.valid());
	if (_lateral_deviation.valid())
		_efis_widget->set_lateral_deviation (*_lateral_deviation);

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


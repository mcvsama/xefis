/* vim:ts=4
 *
 * Copyleft 2012…2016  Michał Gawron
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
#include <xefis/core/module_manager.h>
#include <xefis/utility/numeric.h>
#include <xefis/utility/painter.h>
#include <xefis/utility/qdom.h>

// Local:
#include "adi.h"


XEFIS_REGISTER_MODULE_CLASS ("instruments/adi", ADI)


ADI::ADI (xf::ModuleManager* module_manager, QDomElement const& config):
	Instrument (module_manager, config)
{
	parse_settings (config, {
		{ "speed-ladder.line-every", _speed_ladder_line_every, false },
		{ "speed-ladder.number-every", _speed_ladder_number_every, false },
		{ "speed-ladder.extent", _speed_ladder_extent, false },
		{ "speed-ladder.minimum", _speed_ladder_minimum, false },
		{ "speed-ladder.maximum", _speed_ladder_maximum, false },
		{ "altitude-ladder.line-every", _altitude_ladder_line_every, false },
		{ "altitude-ladder.number-every", _altitude_ladder_number_every, false },
		{ "altitude-ladder.emphasis-every", _altitude_ladder_emphasis_every, false },
		{ "altitude-ladder.bold-every", _altitude_ladder_bold_every, false },
		{ "altitude-ladder.extent", _altitude_ladder_extent, false },
		{ "altitude.landing.warning.hi", _altitude_landing_warning_hi, false },
		{ "altitude.landing.warning.lo", _altitude_landing_warning_lo, false },
		{ "raising-runway.visibility", _raising_runway_visibility, false },
		{ "raising-runway.threshold", _raising_runway_threshold, false },
		{ "aoa.visibility-threshold", _aoa_visibility_threshold, false },
		{ "show-mach-above", _show_mach_above, false },
		{ "energy-variometer.1000-fpm-power", _1000_fpm_power, false },
	});

	parse_properties (config, {
		{ "speed.ias.serviceable", _speed_ias_serviceable, false },
		{ "speed.ias", _speed_ias, false },
		{ "speed.ias.lookahead", _speed_ias_lookahead, false },
		{ "speed.ias.minimum", _speed_ias_minimum, false },
		{ "speed.ias.minimum-maneuver", _speed_ias_minimum_maneuver, false },
		{ "speed.ias.maximum-maneuver", _speed_ias_maximum_maneuver, false },
		{ "speed.ias.maximum", _speed_ias_maximum, false },
		{ "speed.mach", _speed_mach, false },
		{ "speed.ground", _speed_ground, false },
		{ "speed.v1", _speed_v1, false },
		{ "speed.vr", _speed_vr, false },
		{ "speed.vref", _speed_vref, false },
		{ "speed.flaps.up.label", _speed_flaps_up_label, false },
		{ "speed.flaps.up.speed", _speed_flaps_up_speed, false },
		{ "speed.flaps.a.label", _speed_flaps_a_label, false },
		{ "speed.flaps.a.speed", _speed_flaps_a_speed, false },
		{ "speed.flaps.b.label", _speed_flaps_b_label, false },
		{ "speed.flaps.b.speed", _speed_flaps_b_speed, false },
		{ "orientation.serviceable", _orientation_serviceable, false },
		{ "orientation.pitch", _orientation_pitch, false },
		{ "orientation.roll", _orientation_roll, false },
		{ "orientation.heading.magnetic", _orientation_heading_magnetic, false },
		{ "orientation.heading.true", _orientation_heading_true, false },
		{ "orientation.heading.numbers-visible", _orientation_heading_numbers_visible, false },
		{ "track.lateral.magnetic", _track_lateral_magnetic, false },
		{ "track.lateral.true", _track_lateral_true, false },
		{ "track.vertical", _track_vertical, false },
		{ "slip-skid", _slip_skid, false },
		{ "fpv.visible", _fpv_visible, false },
		{ "weight-on-wheels", _weight_on_wheels, false },
		{ "aoa.alpha", _aoa_alpha, false },
		{ "aoa.alpha.maximum", _aoa_alpha_maximum, false },
		{ "aoa.alpha.visible", _aoa_alpha_visible, false },
		{ "altitude.amsl.serviceable", _altitude_amsl_serviceable, false },
		{ "altitude.amsl", _altitude_amsl, false },
		{ "altitude.amsl.lookahead", _altitude_amsl_lookahead, false },
		{ "altitude.agl.serviceable", _altitude_agl_serviceable, false },
		{ "altitude.agl", _altitude_agl, false },
		{ "altitude.minimums.type", _altitude_minimums_type, false },
		{ "altitude.minimums.setting", _altitude_minimums_setting, false },
		{ "altitude.minimums.amsl", _altitude_minimums_amsl, false },
		{ "altitude.landing.amsl", _altitude_landing_amsl, false },
		{ "vertical-speed.serviceable", _vertical_speed_serviceable, false },
		{ "vertical-speed", _vertical_speed, false },
		{ "vertical-speed.energy-variometer", _vertical_speed_energy_variometer, false },
		{ "pressure.qnh", _pressure_qnh, false },
		{ "pressure.display-hpa", _pressure_display_hpa, false },
		{ "pressure.use-std", _pressure_use_std, false },
		{ "flight-director.serviceable", _flight_director_serviceable, false },
		{ "flight-director.cmd.visible", _flight_director_cmd_visible, false },
		{ "flight-director.cmd.altitude", _flight_director_cmd_altitude, false },
		{ "flight-director.cmd.altitude.acquired", _flight_director_cmd_altitude_acquired, false },
		{ "flight-director.cmd.ias", _flight_director_cmd_ias, false },
		{ "flight-director.cmd.mach", _flight_director_cmd_mach, false },
		{ "flight-director.cmd.vertical-speed", _flight_director_cmd_vertical_speed, false },
		{ "flight-director.cmd.fpa", _flight_director_cmd_fpa, false },
		{ "flight-director.guidance.visible", _flight_director_guidance_visible, false },
		{ "flight-director.guidance.pitch", _flight_director_guidance_pitch, false },
		{ "flight-director.guidance.roll", _flight_director_guidance_roll, false },
		{ "control-stick.visible", _control_stick_visible, false },
		{ "control-stick.pitch", _control_stick_pitch, false },
		{ "control-stick.roll", _control_stick_roll, false },
		{ "navaid.reference-visible", _navaid_reference_visible, false },
		{ "navaid.course.magnetic", _navaid_course_magnetic, false },
		{ "navaid.type-hint", _navaid_type_hint, false },
		{ "navaid.localizer-id", _navaid_identifier, false },
		{ "navaid.distance", _navaid_distance, false },
		{ "flight-path.deviation.vertical.serviceable", _flight_path_deviation_vertical_serviceable, false },
		{ "flight-path.deviation.vertical.app", _flight_path_deviation_vertical_app, false },
		{ "flight-path.deviation.vertical.fp", _flight_path_deviation_vertical_fp, false },
		{ "flight-path.deviation.lateral.serviceable", _flight_path_deviation_lateral_serviceable, false },
		{ "flight-path.deviation.lateral.app", _flight_path_deviation_lateral_app, false },
		{ "flight-path.deviation.lateral.fp", _flight_path_deviation_lateral_fp, false },
		{ "flight-path.deviation.mixed-mode", _flight_path_deviation_mixed_mode, false },
		{ "flight-mode.hint.visible", _flight_mode_hint_visible, false },
		{ "flight-mode.hint", _flight_mode_hint, false },
		{ "flight-mode.fma.visible", _flight_mode_fma_visible, false },
		{ "flight-mode.fma.speed-hint", _flight_mode_fma_speed_hint, false },
		{ "flight-mode.fma.speed-small-hint", _flight_mode_fma_speed_small_hint, false },
		{ "flight-mode.fma.lateral-hint", _flight_mode_fma_lateral_hint, false },
		{ "flight-mode.fma.lateral-small-hint", _flight_mode_fma_lateral_small_hint, false },
		{ "flight-mode.fma.vertical-hint", _flight_mode_fma_vertical_hint, false },
		{ "flight-mode.fma.vertical-small-hint", _flight_mode_fma_vertical_small_hint, false },
		{ "tcas.resolution-advisory.pitch.minimum", _tcas_resolution_advisory_pitch_minimum, false },
		{ "tcas.resolution-advisory.pitch.maximum", _tcas_resolution_advisory_pitch_maximum, false },
		{ "tcas.resolution-advisory.vertical-speed.minimum", _tcas_resolution_advisory_vertical_speed_minimum, false },
		{ "tcas.resolution-advisory.vertical-speed.maximum", _tcas_resolution_advisory_vertical_speed_maximum, false },
		{ "warning.novspd-flag", _warning_novspd_flag, false },
		{ "warning.ldgalt-flag", _warning_ldgalt_flag, false },
		{ "warning.pitch-disagree", _warning_pitch_disagree, false },
		{ "warning.roll-disagree", _warning_roll_disagree, false },
		{ "warning.ias-disagree", _warning_ias_disagree, false },
		{ "warning.altitude-disagree", _warning_altitude_disagree, false },
		{ "warning.roll", _warning_roll, false },
		{ "warning.slip-skid", _warning_slip_skid, false },
		{ "style.old", _style_old, false },
		{ "style.show-metric", _style_show_metric, false },
	});

	_adi_widget = new ADIWidget (this, work_performer());

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (0);
	layout->addWidget (_adi_widget);

	_fpv_computer.set_callback (std::bind (&ADI::compute_fpv, this));
	_fpv_computer.observe ({
		&_orientation_heading_magnetic,
		&_orientation_heading_true,
		&_orientation_pitch,
		&_orientation_roll,
		&_track_lateral_magnetic,
		&_track_lateral_true,
		&_track_vertical,
		&_fpv_visible,
		&_weight_on_wheels,
	});
}


void
ADI::read()
{
	_fpv_computer.data_updated (update_time());

	ADIWidget::Parameters params;

	params.old_style = _style_old.read (false);
	params.show_metric = _style_show_metric.read (false);
	// Speed
	params.speed_failure = !_speed_ias_serviceable.read (true);
	params.speed_visible = _speed_ias.valid();
	params.speed = *_speed_ias;
	params.speed_lookahead_visible = _speed_ias_lookahead.valid();
	params.speed_lookahead = *_speed_ias_lookahead;
	params.speed_minimum_visible = _speed_ias_minimum.valid();
	params.speed_minimum = *_speed_ias_minimum;
	params.speed_minimum_maneuver = _speed_ias_minimum_maneuver.get_optional();
	params.speed_maximum_maneuver = _speed_ias_maximum_maneuver.get_optional();
	params.speed_maximum_visible = _speed_ias_maximum.valid();
	params.speed_maximum = *_speed_ias_maximum;
	params.speed_mach_visible = _speed_mach.valid() && *_speed_mach > _show_mach_above;
	params.speed_mach = *_speed_mach;
	params.speed_ground = _speed_ground.get_optional();
	// V1
	if (_speed_v1.valid())
		params.speed_bugs["V1"] = *_speed_v1;
	else
		params.speed_bugs.erase ("V1");
	// Vr
	if (_speed_vr.valid())
		params.speed_bugs["VR"] = *_speed_vr;
	else
		params.speed_bugs.erase ("VR");
	// Vref
	if (_speed_vref.valid())
		params.speed_bugs["REF"] = *_speed_vref;
	else
		params.speed_bugs.erase ("REF");
	// Flaps UP bug:
	if (_speed_flaps_up_speed.valid() && _speed_flaps_up_label.valid())
	{
		_speed_flaps_up_current_label = QString::fromStdString (*_speed_flaps_up_label);
		params.speed_bugs[_speed_flaps_up_current_label] = *_speed_flaps_up_speed;
	}
	else
		params.speed_bugs.erase (_speed_flaps_up_current_label);
	// Flaps "a" bug:
	if (_speed_flaps_a_speed.valid() && _speed_flaps_a_label.valid())
	{
		_speed_flaps_a_current_label = QString::fromStdString (*_speed_flaps_a_label);
		params.speed_bugs[_speed_flaps_a_current_label] = *_speed_flaps_a_speed;
	}
	else
		params.speed_bugs.erase (_speed_flaps_a_current_label);
	// Flaps "b" bug:
	if (_speed_flaps_b_speed.valid() && _speed_flaps_b_label.valid())
	{
		_speed_flaps_b_current_label = QString::fromStdString (*_speed_flaps_b_label);
		params.speed_bugs[_speed_flaps_b_current_label] = *_speed_flaps_b_speed;
	}
	else
		params.speed_bugs.erase (_speed_flaps_b_current_label);
	// Orientation
	params.orientation_failure = !_orientation_serviceable.read (true);
	params.orientation_pitch_visible = _orientation_pitch.valid();
	params.orientation_pitch = *_orientation_pitch;
	params.orientation_roll_visible = _orientation_roll.valid();
	params.orientation_roll = *_orientation_roll;
	params.orientation_heading_visible = _orientation_heading_magnetic.valid();
	params.orientation_heading = *_orientation_heading_magnetic;
	params.orientation_heading_numbers_visible = _orientation_heading_numbers_visible.read (false);
	// Slip-skid
	params.slip_skid_visible = _slip_skid.valid();
	params.slip_skid = *_slip_skid;
	// Flight path vector:
	params.flight_path_marker_failure = _computed_fpv_failure;
	params.flight_path_visible = _computed_fpv_visible;
	params.flight_path_alpha = _computed_fpv_alpha;
	params.flight_path_beta = _computed_fpv_beta;
	// AOA limit
	params.critical_aoa_visible = _aoa_alpha.valid() && _aoa_alpha_maximum.valid() && _aoa_alpha_visible.read (false) &&
								  (*_aoa_alpha_maximum - *_aoa_alpha <= _aoa_visibility_threshold);
	params.critical_aoa = *_aoa_alpha_maximum;
	params.aoa_alpha = *_aoa_alpha;
	// Altitude
	params.altitude_failure = !_altitude_amsl_serviceable.read (true);
	params.altitude_visible = _altitude_amsl.valid();
	params.altitude = *_altitude_amsl;
	params.altitude_lookahead_visible = _altitude_amsl_lookahead.valid();
	params.altitude_lookahead = *_altitude_amsl_lookahead;
	params.altitude_agl_failure = !_altitude_agl_serviceable.read (true);
	params.altitude_agl_visible = _altitude_agl.valid();
	params.altitude_agl = *_altitude_agl;
	params.altitude_landing_visible = _altitude_landing_amsl.valid();
	params.altitude_landing_amsl = *_altitude_landing_amsl;
	params.altitude_landing_warning_hi = _altitude_landing_warning_hi;
	params.altitude_landing_warning_lo = _altitude_landing_warning_lo;
	// Minimums
	params.minimums_altitude_visible = _altitude_minimums_setting.valid() && _altitude_minimums_amsl.valid();
	params.minimums_type = QString::fromStdString (_altitude_minimums_type.read (""));
	params.minimums_amsl = *_altitude_minimums_amsl;
	params.minimums_setting = *_altitude_minimums_setting;
	// Vertical speed
	params.vertical_speed_failure = !_vertical_speed_serviceable.read (true);
	params.vertical_speed_visible = _vertical_speed.valid();
	params.vertical_speed = *_vertical_speed;
	params.energy_variometer_visible = _vertical_speed_energy_variometer.valid();
	params.energy_variometer_rate = *_vertical_speed_energy_variometer;
	params.energy_variometer_1000_fpm_power = _1000_fpm_power;
	// Pressure settings
	params.pressure_visible = _pressure_qnh.valid();
	params.pressure_qnh = *_pressure_qnh;
	params.pressure_display_hpa = _pressure_display_hpa.read (false);
	params.use_standard_pressure = _pressure_use_std.read (false);
	// Command settings
	bool cmd_visible = _flight_director_cmd_visible.read (false);
	if (cmd_visible)
	{
		params.cmd_speed = _flight_director_cmd_ias.get_optional();
		params.cmd_mach = _flight_director_cmd_mach.get_optional();
		params.cmd_altitude = _flight_director_cmd_altitude.get_optional();
		params.cmd_vertical_speed = _flight_director_cmd_vertical_speed.get_optional();
		params.cmd_fpa = _flight_director_cmd_fpa.get_optional();
	}
	else
	{
		params.cmd_speed.reset();
		params.cmd_mach.reset();
		params.cmd_altitude.reset();
		params.cmd_vertical_speed.reset();
		params.cmd_fpa.reset();
	}
	params.cmd_altitude_acquired = _flight_director_cmd_altitude_acquired.read (false);
	// Flight director
	bool guidance_visible = _flight_director_guidance_visible.read (false);
	params.flight_director_failure = !_flight_director_serviceable.read (true);
	params.flight_director_pitch_visible = guidance_visible && _flight_director_guidance_pitch.valid();
	params.flight_director_pitch = *_flight_director_guidance_pitch;
	params.flight_director_roll_visible = guidance_visible && _flight_director_guidance_roll.valid();
	params.flight_director_roll = *_flight_director_guidance_roll;
	// Control stick
	params.control_stick_visible = _control_stick_visible.read (false) && _control_stick_pitch.valid() && _control_stick_roll.valid();
	params.control_stick_pitch = *_control_stick_pitch;
	params.control_stick_roll = *_control_stick_roll;
	// Approach/navaid reference
	params.navaid_reference_visible = _navaid_reference_visible.read (false);
	params.navaid_course_magnetic = _navaid_course_magnetic.get_optional();
	params.navaid_distance = _navaid_distance.get_optional();
	params.navaid_hint = QString::fromStdString (_navaid_type_hint.read (""));
	params.navaid_identifier = QString::fromStdString (*_navaid_identifier);
	// Approach, flight path deviations
	params.deviation_vertical_failure = !_flight_path_deviation_vertical_serviceable.read (true);
	params.deviation_vertical_approach = _flight_path_deviation_vertical_app.get_optional();
	params.deviation_vertical_flight_path = _flight_path_deviation_vertical_fp.get_optional();
	params.deviation_lateral_failure = !_flight_path_deviation_lateral_serviceable.read (true);
	params.deviation_lateral_approach = _flight_path_deviation_lateral_app.get_optional();
	params.deviation_lateral_flight_path = _flight_path_deviation_lateral_fp.get_optional();
	params.deviation_mixed_mode = _flight_path_deviation_mixed_mode.read (false);
	// Raising runway
	params.runway_visible = _navaid_reference_visible.read (false) && _altitude_agl.valid() &&
							_flight_path_deviation_lateral_app.valid() && *_altitude_agl <= _raising_runway_visibility;
	params.runway_position = xf::clamped<Length> (*_altitude_agl, 0_ft, _raising_runway_threshold) / _raising_runway_threshold * 25_deg;
	// Control hint
	params.control_hint_visible = _flight_mode_hint_visible.read (false);
	params.control_hint = QString::fromStdString (_flight_mode_hint.read (""));
	// FMA
	params.fma_visible = _flight_mode_fma_visible.read (false);
	params.fma_speed_hint = QString::fromStdString (_flight_mode_fma_speed_hint.read (""));
	params.fma_speed_small_hint = QString::fromStdString (_flight_mode_fma_speed_small_hint.read (""));
	params.fma_lateral_hint = QString::fromStdString (_flight_mode_fma_lateral_hint.read (""));
	params.fma_lateral_small_hint = QString::fromStdString (_flight_mode_fma_lateral_small_hint.read (""));
	params.fma_vertical_hint = QString::fromStdString (_flight_mode_fma_vertical_hint.read (""));
	params.fma_vertical_small_hint = QString::fromStdString (_flight_mode_fma_vertical_small_hint.read (""));
	// TCAS
	params.tcas_ra_pitch_minimum = _tcas_resolution_advisory_pitch_minimum.get_optional();
	params.tcas_ra_pitch_maximum = _tcas_resolution_advisory_pitch_maximum.get_optional();
	params.tcas_ra_vertical_speed_minimum = _tcas_resolution_advisory_vertical_speed_minimum.get_optional();
	params.tcas_ra_vertical_speed_maximum = _tcas_resolution_advisory_vertical_speed_maximum.get_optional();
	// Warning flags
	params.novspd_flag = _warning_novspd_flag.read (false);
	params.ldgalt_flag = _warning_ldgalt_flag.read (false);
	params.pitch_disagree = _warning_pitch_disagree.read (false);
	params.roll_disagree = _warning_roll_disagree.read (false);
	params.ias_disagree = _warning_ias_disagree.read (false);
	params.altitude_disagree = _warning_altitude_disagree.read (false);
	params.roll_warning = _warning_roll.read (false);
	params.slip_skid_warning = _warning_slip_skid.read (false);
	// Settings:
	params.sl_extent = 1_kt * _speed_ladder_extent;
	params.sl_minimum = _speed_ladder_minimum;
	params.sl_maximum = _speed_ladder_maximum;
	params.sl_line_every = _speed_ladder_line_every;
	params.sl_number_every = _speed_ladder_number_every;
	params.al_extent = 1_ft * _altitude_ladder_extent;
	params.al_emphasis_every = _altitude_ladder_emphasis_every;
	params.al_bold_every = _altitude_ladder_bold_every;
	params.al_line_every = _altitude_ladder_line_every;
	params.al_number_every = _altitude_ladder_number_every;

	_adi_widget->set_params (params);
}


void
ADI::compute_fpv()
{
	xf::PropertyAngle* heading = nullptr;
	xf::PropertyAngle* track_lateral = nullptr;
	if (_orientation_heading_magnetic.valid() && _track_lateral_magnetic.valid())
	{
		heading = &_orientation_heading_magnetic;
		track_lateral = &_track_lateral_magnetic;
	}
	else if (_orientation_heading_true.valid() && _track_lateral_true.valid())
	{
		heading = &_orientation_heading_true;
		track_lateral = &_track_lateral_true;
	}

	if (_orientation_pitch.valid() && _orientation_roll.valid() && _track_vertical.valid() && heading && track_lateral)
	{
		Angle vdiff = xf::floored_mod<Angle> (*_orientation_pitch - *_track_vertical, -180_deg, +180_deg);
		Angle hdiff = xf::floored_mod<Angle> (**heading - **track_lateral, -180_deg, +180_deg);
		Angle roll = *_orientation_roll;

		using std::sin;
		using std::cos;

		_computed_fpv_alpha = vdiff * cos (roll) + hdiff * sin (roll);
		_computed_fpv_beta = -vdiff * sin (roll) + hdiff * cos (roll);
		_computed_fpv_failure = false;
		_computed_fpv_visible = _fpv_visible.read (false);
	}
	else
	{
		_computed_fpv_visible = false;
		_computed_fpv_failure = _fpv_visible.read (false);
	}

	// Hide FPV if weight-on-wheels:
	if (_weight_on_wheels.valid() && *_weight_on_wheels)
		_computed_fpv_visible = false;
}


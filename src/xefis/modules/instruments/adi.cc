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
#include <xefis/utility/numeric.h>
#include <xefis/utility/painter.h>
#include <xefis/utility/qdom.h>

// Local:
#include "adi.h"


ADI::ADI (xf::Xefis* xefis, std::string const& instance):
	v2::Instrument (instance)
{
	_adi_widget = new ADIWidget (this, xefis->work_performer());

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
ADI::process (v2::Cycle const& cycle)
{
	_fpv_computer.process (cycle.update_time());

	ADIWidget::Parameters params;

	params.old_style = _style_old.value_or (false);
	params.show_metric = _style_show_metric.value_or (false);
	// Speed
	params.speed_failure = !_speed_ias_serviceable.value_or (true);
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
	params.speed_mach_visible = _speed_mach && *_speed_mach > _show_mach_above;
	params.speed_mach = *_speed_mach;
	params.speed_ground = _speed_ground.get_optional();
	// V1
	if (_speed_v1)
		params.speed_bugs["V1"] = *_speed_v1;
	else
		params.speed_bugs.erase ("V1");
	// Vr
	if (_speed_vr)
		params.speed_bugs["VR"] = *_speed_vr;
	else
		params.speed_bugs.erase ("VR");
	// Vref
	if (_speed_vref)
		params.speed_bugs["REF"] = *_speed_vref;
	else
		params.speed_bugs.erase ("REF");
	// Flaps UP bug:
	if (_speed_flaps_up_speed && _speed_flaps_up_label)
	{
		_speed_flaps_up_current_label = QString::fromStdString (*_speed_flaps_up_label);
		params.speed_bugs[_speed_flaps_up_current_label] = *_speed_flaps_up_speed;
	}
	else
		params.speed_bugs.erase (_speed_flaps_up_current_label);
	// Flaps "a" bug:
	if (_speed_flaps_a_speed && _speed_flaps_a_label)
	{
		_speed_flaps_a_current_label = QString::fromStdString (*_speed_flaps_a_label);
		params.speed_bugs[_speed_flaps_a_current_label] = *_speed_flaps_a_speed;
	}
	else
		params.speed_bugs.erase (_speed_flaps_a_current_label);
	// Flaps "b" bug:
	if (_speed_flaps_b_speed && _speed_flaps_b_label)
	{
		_speed_flaps_b_current_label = QString::fromStdString (*_speed_flaps_b_label);
		params.speed_bugs[_speed_flaps_b_current_label] = *_speed_flaps_b_speed;
	}
	else
		params.speed_bugs.erase (_speed_flaps_b_current_label);
	// Orientation
	params.orientation_failure = !_orientation_serviceable.value_or (true);
	params.orientation_pitch_visible = _orientation_pitch.valid();
	params.orientation_pitch = *_orientation_pitch;
	params.orientation_roll_visible = _orientation_roll.valid();
	params.orientation_roll = *_orientation_roll;
	params.orientation_heading_visible = _orientation_heading_magnetic.valid();
	params.orientation_heading = *_orientation_heading_magnetic;
	params.orientation_heading_numbers_visible = _orientation_heading_numbers_visible.value_or (false);
	// Slip-skid
	params.slip_skid_visible = _slip_skid.valid();
	params.slip_skid = *_slip_skid;
	// Flight path vector:
	params.flight_path_marker_failure = _computed_fpv_failure;
	params.flight_path_visible = _computed_fpv_visible;
	params.flight_path_alpha = _computed_fpv_alpha;
	params.flight_path_beta = _computed_fpv_beta;
	// AOA limit
	params.critical_aoa_visible = _aoa_alpha && _aoa_alpha_maximum && _aoa_alpha_visible.value_or (false) &&
								  (*_aoa_alpha_maximum - *_aoa_alpha <= *_aoa_visibility_threshold);
	params.critical_aoa = *_aoa_alpha_maximum;
	params.aoa_alpha = *_aoa_alpha;
	// Altitude
	params.altitude_failure = !_altitude_amsl_serviceable.value_or (true);
	params.altitude_visible = _altitude_amsl.valid();
	params.altitude = *_altitude_amsl;
	params.altitude_lookahead_visible = _altitude_amsl_lookahead.valid();
	params.altitude_lookahead = *_altitude_amsl_lookahead;
	params.altitude_agl_failure = !_altitude_agl_serviceable.value_or (true);
	params.altitude_agl_visible = _altitude_agl.valid();
	params.altitude_agl = *_altitude_agl;
	params.altitude_landing_visible = _altitude_landing_amsl.valid();
	params.altitude_landing_amsl = *_altitude_landing_amsl;
	params.altitude_landing_warning_hi = *_altitude_landing_warning_hi;
	params.altitude_landing_warning_lo = *_altitude_landing_warning_lo;
	// Minimums
	params.minimums_altitude_visible = _altitude_minimums_setting && _altitude_minimums_amsl;
	params.minimums_type = QString::fromStdString (_altitude_minimums_type.value_or (""));
	params.minimums_amsl = *_altitude_minimums_amsl;
	params.minimums_setting = *_altitude_minimums_setting;
	// Vertical speed
	params.vertical_speed_failure = !_vertical_speed_serviceable.value_or (true);
	params.vertical_speed_visible = _vertical_speed.valid();
	params.vertical_speed = *_vertical_speed;
	params.energy_variometer_visible = _vertical_speed_energy_variometer.valid();
	params.energy_variometer_rate = *_vertical_speed_energy_variometer;
	params.energy_variometer_1000_fpm_power = *_1000_fpm_power;
	// Pressure settings
	params.pressure_visible = _pressure_qnh.valid();
	params.pressure_qnh = *_pressure_qnh;
	params.pressure_display_hpa = _pressure_display_hpa.value_or (false);
	params.use_standard_pressure = _pressure_use_std.value_or (false);
	// Command settings
	bool cmd_visible = _flight_director_cmd_visible.value_or (false);
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
	params.cmd_altitude_acquired = _flight_director_cmd_altitude_acquired.value_or (false);
	// Flight director
	bool guidance_visible = _flight_director_guidance_visible.value_or (false);
	params.flight_director_failure = !_flight_director_serviceable.value_or (true);
	params.flight_director_pitch_visible = guidance_visible && _flight_director_guidance_pitch.valid();
	params.flight_director_pitch = *_flight_director_guidance_pitch;
	params.flight_director_roll_visible = guidance_visible && _flight_director_guidance_roll.valid();
	params.flight_director_roll = *_flight_director_guidance_roll;
	// Control stick
	params.control_stick_visible = _control_stick_visible.value_or (false) && _control_stick_pitch && _control_stick_roll;
	params.control_stick_pitch = *_control_stick_pitch;
	params.control_stick_roll = *_control_stick_roll;
	// Approach/navaid reference
	params.navaid_reference_visible = _navaid_reference_visible.value_or (false);
	params.navaid_course_magnetic = _navaid_course_magnetic.get_optional();
	params.navaid_distance = _navaid_distance.get_optional();
	params.navaid_hint = QString::fromStdString (_navaid_type_hint.value_or (""));
	params.navaid_identifier = QString::fromStdString (*_navaid_identifier);
	// Approach, flight path deviations
	params.deviation_vertical_failure = !_flight_path_deviation_vertical_serviceable.value_or (true);
	params.deviation_vertical_approach = _flight_path_deviation_vertical_app.get_optional();
	params.deviation_vertical_flight_path = _flight_path_deviation_vertical_fp.get_optional();
	params.deviation_lateral_failure = !_flight_path_deviation_lateral_serviceable.value_or (true);
	params.deviation_lateral_approach = _flight_path_deviation_lateral_app.get_optional();
	params.deviation_lateral_flight_path = _flight_path_deviation_lateral_fp.get_optional();
	params.deviation_mixed_mode = _flight_path_deviation_mixed_mode.value_or (false);
	// Raising runway
	params.runway_visible = _navaid_reference_visible.value_or (false) && _altitude_agl &&
							_flight_path_deviation_lateral_app && *_altitude_agl <= *_raising_runway_visibility;
	params.runway_position = xf::clamped<Length> (*_altitude_agl, 0_ft, *_raising_runway_threshold) / *_raising_runway_threshold * 25_deg;
	// Control hint
	params.control_hint_visible = _flight_mode_hint_visible.value_or (false);
	params.control_hint = QString::fromStdString (_flight_mode_hint.value_or (""));
	// FMA
	params.fma_visible = _flight_mode_fma_visible.value_or (false);
	params.fma_speed_hint = QString::fromStdString (_flight_mode_fma_speed_hint.value_or (""));
	params.fma_speed_armed_hint = QString::fromStdString (_flight_mode_fma_speed_armed_hint.value_or (""));
	params.fma_lateral_hint = QString::fromStdString (_flight_mode_fma_lateral_hint.value_or (""));
	params.fma_lateral_armed_hint = QString::fromStdString (_flight_mode_fma_lateral_armed_hint.value_or (""));
	params.fma_vertical_hint = QString::fromStdString (_flight_mode_fma_vertical_hint.value_or (""));
	params.fma_vertical_armed_hint = QString::fromStdString (_flight_mode_fma_vertical_armed_hint.value_or (""));
	// TCAS
	params.tcas_ra_pitch_minimum = _tcas_resolution_advisory_pitch_minimum.get_optional();
	params.tcas_ra_pitch_maximum = _tcas_resolution_advisory_pitch_maximum.get_optional();
	params.tcas_ra_vertical_speed_minimum = _tcas_resolution_advisory_vertical_speed_minimum.get_optional();
	params.tcas_ra_vertical_speed_maximum = _tcas_resolution_advisory_vertical_speed_maximum.get_optional();
	// Warning flags
	params.novspd_flag = _warning_novspd_flag.value_or (false);
	params.ldgalt_flag = _warning_ldgalt_flag.value_or (false);
	params.pitch_disagree = _warning_pitch_disagree.value_or (false);
	params.roll_disagree = _warning_roll_disagree.value_or (false);
	params.ias_disagree = _warning_ias_disagree.value_or (false);
	params.altitude_disagree = _warning_altitude_disagree.value_or (false);
	params.roll_warning = _warning_roll.value_or (false);
	params.slip_skid_warning = _warning_slip_skid.value_or (false);
	// Settings:
	params.sl_extent = 1_kt * *_speed_ladder_extent;
	params.sl_minimum = *_speed_ladder_minimum;
	params.sl_maximum = *_speed_ladder_maximum;
	params.sl_line_every = *_speed_ladder_line_every;
	params.sl_number_every = *_speed_ladder_number_every;
	params.al_extent = 1_ft * *_altitude_ladder_extent;
	params.al_emphasis_every = *_altitude_ladder_emphasis_every;
	params.al_bold_every = *_altitude_ladder_bold_every;
	params.al_line_every = *_altitude_ladder_line_every;
	params.al_number_every = *_altitude_ladder_number_every;

	_adi_widget->set_params (params);
}


void
ADI::compute_fpv()
{
	v2::PropertyIn<si::Angle>* heading = nullptr;
	v2::PropertyIn<si::Angle>* track_lateral = nullptr;
	if (_orientation_heading_magnetic && _track_lateral_magnetic)
	{
		heading = &_orientation_heading_magnetic;
		track_lateral = &_track_lateral_magnetic;
	}
	else if (_orientation_heading_true && _track_lateral_true)
	{
		heading = &_orientation_heading_true;
		track_lateral = &_track_lateral_true;
	}

	if (_orientation_pitch && _orientation_roll && _track_vertical && heading && track_lateral)
	{
		Angle vdiff = xf::floored_mod<Angle> (*_orientation_pitch - *_track_vertical, -180_deg, +180_deg);
		Angle hdiff = xf::floored_mod<Angle> (**heading - **track_lateral, -180_deg, +180_deg);
		Angle roll = *_orientation_roll;

		_computed_fpv_alpha = vdiff * si::cos (roll) + hdiff * si::sin (roll);
		_computed_fpv_beta = -vdiff * si::sin (roll) + hdiff * si::cos (roll);
		_computed_fpv_failure = false;
		_computed_fpv_visible = _fpv_visible.value_or (false);
	}
	else
	{
		_computed_fpv_visible = false;
		_computed_fpv_failure = _fpv_visible.value_or (false);
	}

	// Hide FPV if weight-on-wheels:
	if (_weight_on_wheels && *_weight_on_wheels)
		_computed_fpv_visible = false;
}


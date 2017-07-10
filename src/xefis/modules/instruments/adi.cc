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
		&orientation_heading_magnetic,
		&orientation_heading_true,
		&orientation_pitch,
		&orientation_roll,
		&track_lateral_magnetic,
		&track_lateral_true,
		&track_vertical,
		&fpv_visible,
		&weight_on_wheels,
	});
}


void
ADI::process (v2::Cycle const& cycle)
{
	_fpv_computer.process (cycle.update_time());

	ADIWidget::Parameters params;

	params.old_style = style_old.value_or (false);
	params.show_metric = style_show_metric.value_or (false);
	// Speed
	params.speed_failure = !speed_ias_serviceable.value_or (true);
	params.speed_visible = speed_ias.valid();
	params.speed = *speed_ias;
	params.speed_lookahead_visible = speed_ias_lookahead.valid();
	params.speed_lookahead = *speed_ias_lookahead;
	params.speed_minimum_visible = speed_ias_minimum.valid();
	params.speed_minimum = *speed_ias_minimum;
	params.speed_minimum_maneuver = speed_ias_minimum_maneuver.get_optional();
	params.speed_maximum_maneuver = speed_ias_maximum_maneuver.get_optional();
	params.speed_maximum_visible = speed_ias_maximum.valid();
	params.speed_maximum = *speed_ias_maximum;
	params.speed_mach_visible = speed_mach && *speed_mach > show_mach_above;
	params.speed_mach = *speed_mach;
	params.speed_ground = speed_ground.get_optional();
	// V1
	if (speed_v1)
		params.speed_bugs["V1"] = *speed_v1;
	else
		params.speed_bugs.erase ("V1");
	// Vr
	if (speed_vr)
		params.speed_bugs["VR"] = *speed_vr;
	else
		params.speed_bugs.erase ("VR");
	// Vref
	if (speed_vref)
		params.speed_bugs["REF"] = *speed_vref;
	else
		params.speed_bugs.erase ("REF");
	// Flaps UP bug:
	if (speed_flaps_up_speed && speed_flaps_up_label)
	{
		_speed_flaps_up_current_label = QString::fromStdString (*speed_flaps_up_label);
		params.speed_bugs[_speed_flaps_up_current_label] = *speed_flaps_up_speed;
	}
	else
		params.speed_bugs.erase (_speed_flaps_up_current_label);
	// Flaps "a" bug:
	if (speed_flaps_a_speed && speed_flaps_a_label)
	{
		_speed_flaps_a_current_label = QString::fromStdString (*speed_flaps_a_label);
		params.speed_bugs[_speed_flaps_a_current_label] = *speed_flaps_a_speed;
	}
	else
		params.speed_bugs.erase (_speed_flaps_a_current_label);
	// Flaps "b" bug:
	if (speed_flaps_b_speed && speed_flaps_b_label)
	{
		_speed_flaps_b_current_label = QString::fromStdString (*speed_flaps_b_label);
		params.speed_bugs[_speed_flaps_b_current_label] = *speed_flaps_b_speed;
	}
	else
		params.speed_bugs.erase (_speed_flaps_b_current_label);
	// Orientation
	params.orientation_failure = !orientation_serviceable.value_or (true);
	params.orientation_pitch_visible = orientation_pitch.valid();
	params.orientation_pitch = *orientation_pitch;
	params.orientation_roll_visible = orientation_roll.valid();
	params.orientation_roll = *orientation_roll;
	params.orientation_heading_visible = orientation_heading_magnetic.valid();
	params.orientation_heading = *orientation_heading_magnetic;
	params.orientation_heading_numbers_visible = orientation_heading_numbers_visible.value_or (false);
	// Slip-skid
	params.slip_skid_visible = slip_skid.valid();
	params.slip_skid = *slip_skid;
	// Flight path vector:
	params.flight_path_marker_failure = _computed_fpv_failure;
	params.flight_path_visible = _computed_fpv_visible;
	params.flight_path_alpha = _computed_fpv_alpha;
	params.flight_path_beta = _computed_fpv_beta;
	// AOA limit
	params.critical_aoa_visible = aoa_alpha && aoa_alpha_maximum && aoa_alpha_visible.value_or (false) &&
								  (*aoa_alpha_maximum - *aoa_alpha <= *aoa_visibility_threshold);
	params.critical_aoa = *aoa_alpha_maximum;
	params.aoa_alpha = *aoa_alpha;
	// Altitude
	params.altitude_failure = !altitude_amsl_serviceable.value_or (true);
	params.altitude_visible = altitude_amsl.valid();
	params.altitude = *altitude_amsl;
	params.altitude_lookahead_visible = altitude_amsl_lookahead.valid();
	params.altitude_lookahead = *altitude_amsl_lookahead;
	params.altitude_agl_failure = !altitude_agl_serviceable.value_or (true);
	params.altitude_agl_visible = altitude_agl.valid();
	params.altitude_agl = *altitude_agl;
	params.altitude_landing_visible = altitude_landing_amsl.valid();
	params.altitude_landing_amsl = *altitude_landing_amsl;
	params.altitude_landing_warning_hi = *altitude_landing_warning_hi;
	params.altitude_landing_warning_lo = *altitude_landing_warning_lo;
	// Minimums
	params.minimums_altitude_visible = altitude_minimums_setting && altitude_minimums_amsl;
	params.minimums_type = QString::fromStdString (altitude_minimums_type.value_or (""));
	params.minimums_amsl = *altitude_minimums_amsl;
	params.minimums_setting = *altitude_minimums_setting;
	// Vertical speed
	params.vertical_speed_failure = !vertical_speed_serviceable.value_or (true);
	params.vertical_speed_visible = vertical_speed.valid();
	params.vertical_speed = *vertical_speed;
	params.energy_variometer_visible = vertical_speed_energy_variometer.valid();
	params.energy_variometer_rate = *vertical_speed_energy_variometer;
	params.energy_variometer_1000_fpm_power = *power_eq_1000_fpm;
	// Pressure settings
	params.pressure_visible = pressure_qnh.valid();
	params.pressure_qnh = *pressure_qnh;
	params.pressure_display_hpa = pressure_display_hpa.value_or (false);
	params.use_standard_pressure = pressure_use_std.value_or (false);
	// Command settings
	bool cmd_visible = flight_director_cmd_visible.value_or (false);
	if (cmd_visible)
	{
		params.cmd_speed = flight_director_cmd_ias.get_optional();
		params.cmd_mach = flight_director_cmd_mach.get_optional();
		params.cmd_altitude = flight_director_cmd_altitude.get_optional();
		params.cmd_vertical_speed = flight_director_cmd_vertical_speed.get_optional();
		params.cmd_fpa = flight_director_cmd_fpa.get_optional();
	}
	else
	{
		params.cmd_speed.reset();
		params.cmd_mach.reset();
		params.cmd_altitude.reset();
		params.cmd_vertical_speed.reset();
		params.cmd_fpa.reset();
	}
	params.cmd_altitude_acquired = flight_director_cmd_altitude_acquired.value_or (false);
	// Flight director
	bool guidance_visible = flight_director_guidance_visible.value_or (false);
	params.flight_director_failure = !flight_director_serviceable.value_or (true);
	params.flight_director_pitch_visible = guidance_visible && flight_director_guidance_pitch.valid();
	params.flight_director_pitch = *flight_director_guidance_pitch;
	params.flight_director_roll_visible = guidance_visible && flight_director_guidance_roll.valid();
	params.flight_director_roll = *flight_director_guidance_roll;
	// Control stick
	params.control_stick_visible = control_stick_visible.value_or (false) && control_stick_pitch && control_stick_roll;
	params.control_stick_pitch = *control_stick_pitch;
	params.control_stick_roll = *control_stick_roll;
	// Approach/navaid reference
	params.navaid_reference_visible = navaid_reference_visible.value_or (false);
	params.navaid_course_magnetic = navaid_course_magnetic.get_optional();
	params.navaid_distance = navaid_distance.get_optional();
	params.navaid_hint = QString::fromStdString (navaid_type_hint.value_or (""));
	params.navaid_identifier = QString::fromStdString (*navaid_identifier);
	// Approach, flight path deviations
	params.deviation_vertical_failure = !flight_path_deviation_vertical_serviceable.value_or (true);
	params.deviation_vertical_approach = flight_path_deviation_vertical_app.get_optional();
	params.deviation_vertical_flight_path = flight_path_deviation_vertical_fp.get_optional();
	params.deviation_lateral_failure = !flight_path_deviation_lateral_serviceable.value_or (true);
	params.deviation_lateral_approach = flight_path_deviation_lateral_app.get_optional();
	params.deviation_lateral_flight_path = flight_path_deviation_lateral_fp.get_optional();
	params.deviation_mixed_mode = flight_path_deviation_mixed_mode.value_or (false);
	// Raising runway
	params.runway_visible = navaid_reference_visible.value_or (false) && altitude_agl &&
							flight_path_deviation_lateral_app && *altitude_agl <= *raising_runway_visibility;
	params.runway_position = xf::clamped<Length> (*altitude_agl, 0_ft, *raising_runway_threshold) / *raising_runway_threshold * 25_deg;
	// Control hint
	params.control_hint_visible = flight_mode_hint_visible.value_or (false);
	params.control_hint = QString::fromStdString (flight_mode_hint.value_or (""));
	// FMA
	params.fma_visible = flight_mode_fma_visible.value_or (false);
	params.fma_speed_hint = QString::fromStdString (flight_mode_fma_speed_hint.value_or (""));
	params.fma_speed_armed_hint = QString::fromStdString (flight_mode_fma_speed_armed_hint.value_or (""));
	params.fma_lateral_hint = QString::fromStdString (flight_mode_fma_lateral_hint.value_or (""));
	params.fma_lateral_armed_hint = QString::fromStdString (flight_mode_fma_lateral_armed_hint.value_or (""));
	params.fma_vertical_hint = QString::fromStdString (flight_mode_fma_vertical_hint.value_or (""));
	params.fma_vertical_armed_hint = QString::fromStdString (flight_mode_fma_vertical_armed_hint.value_or (""));
	// TCAS
	params.tcas_ra_pitch_minimum = tcas_resolution_advisory_pitch_minimum.get_optional();
	params.tcas_ra_pitch_maximum = tcas_resolution_advisory_pitch_maximum.get_optional();
	params.tcas_ra_vertical_speed_minimum = tcas_resolution_advisory_vertical_speed_minimum.get_optional();
	params.tcas_ra_vertical_speed_maximum = tcas_resolution_advisory_vertical_speed_maximum.get_optional();
	// Warning flags
	params.novspd_flag = warning_novspd_flag.value_or (false);
	params.ldgalt_flag = warning_ldgalt_flag.value_or (false);
	params.pitch_disagree = warning_pitch_disagree.value_or (false);
	params.roll_disagree = warning_roll_disagree.value_or (false);
	params.ias_disagree = warning_ias_disagree.value_or (false);
	params.altitude_disagree = warning_altitude_disagree.value_or (false);
	params.roll_warning = warning_roll.value_or (false);
	params.slip_skid_warning = warning_slip_skid.value_or (false);
	// Settings:
	params.sl_extent = 1_kt * *speed_ladder_extent;
	params.sl_minimum = *speed_ladder_minimum;
	params.sl_maximum = *speed_ladder_maximum;
	params.sl_line_every = *speed_ladder_line_every;
	params.sl_number_every = *speed_ladder_number_every;
	params.al_extent = 1_ft * *altitude_ladder_extent;
	params.al_emphasis_every = *altitude_ladder_emphasis_every;
	params.al_bold_every = *altitude_ladder_bold_every;
	params.al_line_every = *altitude_ladder_line_every;
	params.al_number_every = *altitude_ladder_number_every;

	_adi_widget->set_params (params);
}


void
ADI::compute_fpv()
{
	v2::PropertyIn<si::Angle>* heading = nullptr;
	v2::PropertyIn<si::Angle>* track_lateral = nullptr;
	if (orientation_heading_magnetic && track_lateral_magnetic)
	{
		heading = &orientation_heading_magnetic;
		track_lateral = &track_lateral_magnetic;
	}
	else if (orientation_heading_true && track_lateral_true)
	{
		heading = &orientation_heading_true;
		track_lateral = &track_lateral_true;
	}

	if (orientation_pitch && orientation_roll && track_vertical && heading && track_lateral)
	{
		Angle vdiff = xf::floored_mod<Angle> (*orientation_pitch - *track_vertical, -180_deg, +180_deg);
		Angle hdiff = xf::floored_mod<Angle> (**heading - **track_lateral, -180_deg, +180_deg);
		Angle roll = *orientation_roll;

		_computed_fpv_alpha = vdiff * si::cos (roll) + hdiff * si::sin (roll);
		_computed_fpv_beta = -vdiff * si::sin (roll) + hdiff * si::cos (roll);
		_computed_fpv_failure = false;
		_computed_fpv_visible = fpv_visible.value_or (false);
	}
	else
	{
		_computed_fpv_visible = false;
		_computed_fpv_failure = fpv_visible.value_or (false);
	}

	// Hide FPV if weight-on-wheels:
	if (weight_on_wheels && *weight_on_wheels)
		_computed_fpv_visible = false;
}


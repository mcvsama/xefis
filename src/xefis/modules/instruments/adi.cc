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


ADI::ADI (std::unique_ptr<ADI_IO> module_io, xf::Xefis* xefis, std::string const& instance):
	Instrument (std::move (module_io), instance)
{
	_adi_widget = new ADIWidget (this, xefis->work_performer());

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (0);
	layout->addWidget (_adi_widget);

	_fpv_computer.set_callback (std::bind (&ADI::compute_fpv, this));
	_fpv_computer.observe ({
		&io.orientation_heading_magnetic,
		&io.orientation_heading_true,
		&io.orientation_pitch,
		&io.orientation_roll,
		&io.track_lateral_magnetic,
		&io.track_lateral_true,
		&io.track_vertical,
		&io.fpv_visible,
		&io.weight_on_wheels,
	});
}


void
ADI::process (v2::Cycle const& cycle)
{
	_fpv_computer.process (cycle.update_time());

	ADIWidget::Parameters params;

	params.old_style = io.style_old.value_or (false);
	params.show_metric = io.style_show_metric.value_or (false);
	// Speed
	params.speed_failure = !io.speed_ias_serviceable.value_or (true);
	params.speed_visible = io.speed_ias.valid();
	params.speed = io.speed_ias.value_or (0_kt);
	params.speed_lookahead_visible = io.speed_ias_lookahead.valid();
	params.speed_lookahead = io.speed_ias_lookahead.value_or (0_kt);
	params.speed_minimum_visible = io.speed_ias_minimum.valid();
	params.speed_minimum = io.speed_ias_minimum.value_or (0_kt);
	params.speed_minimum_maneuver = io.speed_ias_minimum_maneuver.get_optional();
	params.speed_maximum_maneuver = io.speed_ias_maximum_maneuver.get_optional();
	params.speed_maximum_visible = io.speed_ias_maximum.valid();
	params.speed_maximum = io.speed_ias_maximum.value_or (0_kt);
	params.speed_mach_visible = io.speed_mach && *io.speed_mach > io.show_mach_above;
	params.speed_mach = io.speed_mach.value_or (0.0);
	params.speed_ground = io.speed_ground.get_optional();

	// V1
	if (io.speed_v1)
		params.speed_bugs["V1"] = *io.speed_v1;
	else
		params.speed_bugs.erase ("V1");

	// Vr
	if (io.speed_vr)
		params.speed_bugs["VR"] = *io.speed_vr;
	else
		params.speed_bugs.erase ("VR");

	// Vref
	if (io.speed_vref)
		params.speed_bugs["REF"] = *io.speed_vref;
	else
		params.speed_bugs.erase ("REF");

	// Flaps UP bug:
	if (io.speed_flaps_up_speed && io.speed_flaps_up_label)
	{
		_speed_flaps_up_current_label = QString::fromStdString (*io.speed_flaps_up_label);
		params.speed_bugs[_speed_flaps_up_current_label] = *io.speed_flaps_up_speed;
	}
	else
		params.speed_bugs.erase (_speed_flaps_up_current_label);

	// Flaps "a" bug:
	if (io.speed_flaps_a_speed && io.speed_flaps_a_label)
	{
		_speed_flaps_a_current_label = QString::fromStdString (*io.speed_flaps_a_label);
		params.speed_bugs[_speed_flaps_a_current_label] = *io.speed_flaps_a_speed;
	}
	else
		params.speed_bugs.erase (_speed_flaps_a_current_label);

	// Flaps "b" bug:
	if (io.speed_flaps_b_speed && io.speed_flaps_b_label)
	{
		_speed_flaps_b_current_label = QString::fromStdString (*io.speed_flaps_b_label);
		params.speed_bugs[_speed_flaps_b_current_label] = *io.speed_flaps_b_speed;
	}
	else
		params.speed_bugs.erase (_speed_flaps_b_current_label);

	// Orientation
	params.orientation_failure = !io.orientation_serviceable.value_or (true);
	params.orientation_pitch_visible = io.orientation_pitch.valid();
	params.orientation_pitch = io.orientation_pitch.value_or (0_deg);
	params.orientation_roll_visible = io.orientation_roll.valid();
	params.orientation_roll = io.orientation_roll.value_or (0_deg);
	params.orientation_heading_visible = io.orientation_heading_magnetic.valid();
	params.orientation_heading = io.orientation_heading_magnetic.value_or (0_deg);
	params.orientation_heading_numbers_visible = io.orientation_heading_numbers_visible.value_or (false);
	// Slip-skid
	params.slip_skid_visible = io.slip_skid.valid();
	params.slip_skid = io.slip_skid.value_or (0_deg);
	// Flight path vector:
	params.flight_path_marker_failure = _computed_fpv_failure;
	params.flight_path_visible = _computed_fpv_visible;
	params.flight_path_alpha = _computed_fpv_alpha;
	params.flight_path_beta = _computed_fpv_beta;
	// AOA limit
	params.critical_aoa_visible = io.aoa_alpha && io.aoa_alpha_maximum && io.aoa_alpha_visible.value_or (false) &&
								  (*io.aoa_alpha_maximum - *io.aoa_alpha <= *io.aoa_visibility_threshold);
	params.critical_aoa = io.aoa_alpha_maximum.value_or (0_deg);
	params.aoa_alpha = io.aoa_alpha.value_or (0_deg);
	// Altitude
	params.altitude_failure = !io.altitude_amsl_serviceable.value_or (true);
	params.altitude_visible = io.altitude_amsl.valid();
	params.altitude = io.altitude_amsl.value_or (0_ft);
	params.altitude_lookahead_visible = io.altitude_amsl_lookahead.valid();
	params.altitude_lookahead = io.altitude_amsl_lookahead.value_or (0_ft);
	params.altitude_agl_failure = !io.altitude_agl_serviceable.value_or (true);
	params.altitude_agl_visible = io.altitude_agl.valid();
	params.altitude_agl = io.altitude_agl.value_or (0_ft);
	params.altitude_landing_visible = io.altitude_landing_amsl.valid();
	params.altitude_landing_amsl = io.altitude_landing_amsl.value_or (0_ft);
	params.altitude_landing_warning_hi = *io.altitude_landing_warning_hi;
	params.altitude_landing_warning_lo = *io.altitude_landing_warning_lo;
	// Minimums
	params.minimums_altitude_visible = io.altitude_minimums_setting && io.altitude_minimums_amsl;
	params.minimums_type = QString::fromStdString (io.altitude_minimums_type.value_or (""));
	params.minimums_amsl = io.altitude_minimums_amsl.value_or (0_ft);
	params.minimums_setting = io.altitude_minimums_setting.value_or (0_ft);
	// Vertical speed
	params.vertical_speed_failure = !io.vertical_speed_serviceable.value_or (true);
	params.vertical_speed_visible = io.vertical_speed.valid();
	params.vertical_speed = io.vertical_speed.value_or (0_fpm);
	params.energy_variometer_visible = io.vertical_speed_energy_variometer.valid();
	params.energy_variometer_rate = io.vertical_speed_energy_variometer.value_or (0_W);
	params.energy_variometer_1000_fpm_power = *io.power_eq_1000_fpm;
	// Pressure settings
	params.pressure_visible = io.pressure_qnh.valid();
	params.pressure_qnh = io.pressure_qnh.value_or (0_Pa);
	params.pressure_display_hpa = io.pressure_display_hpa.value_or (false);
	params.use_standard_pressure = io.pressure_use_std.value_or (false);
	// Command settings
	bool cmd_visible = io.flight_director_cmd_visible.value_or (false);

	if (cmd_visible)
	{
		params.cmd_speed = io.flight_director_cmd_ias.get_optional();
		params.cmd_mach = io.flight_director_cmd_mach.get_optional();
		params.cmd_altitude = io.flight_director_cmd_altitude.get_optional();
		params.cmd_vertical_speed = io.flight_director_cmd_vertical_speed.get_optional();
		params.cmd_fpa = io.flight_director_cmd_fpa.get_optional();
	}
	else
	{
		params.cmd_speed.reset();
		params.cmd_mach.reset();
		params.cmd_altitude.reset();
		params.cmd_vertical_speed.reset();
		params.cmd_fpa.reset();
	}

	params.cmd_altitude_acquired = io.flight_director_cmd_altitude_acquired.value_or (false);
	// Flight director
	bool guidance_visible = io.flight_director_guidance_visible.value_or (false);
	params.flight_director_failure = !io.flight_director_serviceable.value_or (true);
	params.flight_director_pitch_visible = guidance_visible && io.flight_director_guidance_pitch.valid();
	params.flight_director_pitch = io.flight_director_guidance_pitch.value_or (0_deg);
	params.flight_director_roll_visible = guidance_visible && io.flight_director_guidance_roll.valid();
	params.flight_director_roll = io.flight_director_guidance_roll.value_or (0_deg);
	// Control stick
	params.control_stick_visible = io.control_stick_visible.value_or (false) && io.control_stick_pitch && io.control_stick_roll;
	params.control_stick_pitch = io.control_stick_pitch.value_or (0_deg);
	params.control_stick_roll = io.control_stick_roll.value_or (0_deg);
	// Approach/navaid reference
	params.navaid_reference_visible = io.navaid_reference_visible.value_or (false);
	params.navaid_course_magnetic = io.navaid_course_magnetic.get_optional();
	params.navaid_distance = io.navaid_distance.get_optional();
	params.navaid_hint = QString::fromStdString (io.navaid_type_hint.value_or (""));
	params.navaid_identifier = QString::fromStdString (io.navaid_identifier.value_or (""));
	// Approach, flight path deviations
	params.deviation_vertical_failure = !io.flight_path_deviation_vertical_serviceable.value_or (true);
	params.deviation_vertical_approach = io.flight_path_deviation_vertical_app.get_optional();
	params.deviation_vertical_flight_path = io.flight_path_deviation_vertical_fp.get_optional();
	params.deviation_lateral_failure = !io.flight_path_deviation_lateral_serviceable.value_or (true);
	params.deviation_lateral_approach = io.flight_path_deviation_lateral_app.get_optional();
	params.deviation_lateral_flight_path = io.flight_path_deviation_lateral_fp.get_optional();
	params.deviation_mixed_mode = io.flight_path_deviation_mixed_mode.value_or (false);
	// Raising runway
	params.runway_visible = io.navaid_reference_visible.value_or (false) && io.altitude_agl &&
							io.flight_path_deviation_lateral_app && *io.altitude_agl <= *io.raising_runway_visibility;
	params.runway_position = xf::clamped<Length> (io.altitude_agl.value_or (0_ft), 0_ft, *io.raising_runway_threshold) / *io.raising_runway_threshold * 25_deg;
	// Control hint
	params.control_hint_visible = io.flight_mode_hint_visible.value_or (false);
	params.control_hint = QString::fromStdString (io.flight_mode_hint.value_or (""));
	// FMA
	params.fma_visible = io.flight_mode_fma_visible.value_or (false);
	params.fma_speed_hint = QString::fromStdString (io.flight_mode_fma_speed_hint.value_or (""));
	params.fma_speed_armed_hint = QString::fromStdString (io.flight_mode_fma_speed_armed_hint.value_or (""));
	params.fma_lateral_hint = QString::fromStdString (io.flight_mode_fma_lateral_hint.value_or (""));
	params.fma_lateral_armed_hint = QString::fromStdString (io.flight_mode_fma_lateral_armed_hint.value_or (""));
	params.fma_vertical_hint = QString::fromStdString (io.flight_mode_fma_vertical_hint.value_or (""));
	params.fma_vertical_armed_hint = QString::fromStdString (io.flight_mode_fma_vertical_armed_hint.value_or (""));
	// TCAS
	params.tcas_ra_pitch_minimum = io.tcas_resolution_advisory_pitch_minimum.get_optional();
	params.tcas_ra_pitch_maximum = io.tcas_resolution_advisory_pitch_maximum.get_optional();
	params.tcas_ra_vertical_speed_minimum = io.tcas_resolution_advisory_vertical_speed_minimum.get_optional();
	params.tcas_ra_vertical_speed_maximum = io.tcas_resolution_advisory_vertical_speed_maximum.get_optional();
	// Warning flags
	params.novspd_flag = io.warning_novspd_flag.value_or (false);
	params.ldgalt_flag = io.warning_ldgalt_flag.value_or (false);
	params.pitch_disagree = io.warning_pitch_disagree.value_or (false);
	params.roll_disagree = io.warning_roll_disagree.value_or (false);
	params.ias_disagree = io.warning_ias_disagree.value_or (false);
	params.altitude_disagree = io.warning_altitude_disagree.value_or (false);
	params.roll_warning = io.warning_roll.value_or (false);
	params.slip_skid_warning = io.warning_slip_skid.value_or (false);
	// Settings:
	params.sl_extent = 1_kt * *io.speed_ladder_extent;
	params.sl_minimum = *io.speed_ladder_minimum;
	params.sl_maximum = *io.speed_ladder_maximum;
	params.sl_line_every = *io.speed_ladder_line_every;
	params.sl_number_every = *io.speed_ladder_number_every;
	params.al_extent = 1_ft * *io.altitude_ladder_extent;
	params.al_emphasis_every = *io.altitude_ladder_emphasis_every;
	params.al_bold_every = *io.altitude_ladder_bold_every;
	params.al_line_every = *io.altitude_ladder_line_every;
	params.al_number_every = *io.altitude_ladder_number_every;

	_adi_widget->set_params (params);
}


void
ADI::compute_fpv()
{
	v2::PropertyIn<si::Angle>* heading = nullptr;
	v2::PropertyIn<si::Angle>* track_lateral = nullptr;

	if (io.orientation_heading_magnetic && io.track_lateral_magnetic)
	{
		heading = &io.orientation_heading_magnetic;
		track_lateral = &io.track_lateral_magnetic;
	}
	else if (io.orientation_heading_true && io.track_lateral_true)
	{
		heading = &io.orientation_heading_true;
		track_lateral = &io.track_lateral_true;
	}

	if (io.orientation_pitch && io.orientation_roll && io.track_vertical && heading && track_lateral)
	{
		Angle vdiff = xf::floored_mod<Angle> (*io.orientation_pitch - *io.track_vertical, -180_deg, +180_deg);
		Angle hdiff = xf::floored_mod<Angle> (**heading - **track_lateral, -180_deg, +180_deg);
		Angle roll = *io.orientation_roll;

		_computed_fpv_alpha = vdiff * si::cos (roll) + hdiff * si::sin (roll);
		_computed_fpv_beta = -vdiff * si::sin (roll) + hdiff * si::cos (roll);
		_computed_fpv_failure = false;
		_computed_fpv_visible = io.fpv_visible.value_or (false);
	}
	else
	{
		_computed_fpv_visible = false;
		_computed_fpv_failure = io.fpv_visible.value_or (false);
	}

	// Hide FPV if weight-on-wheels:
	if (io.weight_on_wheels && *io.weight_on_wheels)
		_computed_fpv_visible = false;
}


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

#ifndef XEFIS__MODULES__INSTRUMENTS__ADI_H__INCLUDED
#define XEFIS__MODULES__INSTRUMENTS__ADI_H__INCLUDED

// Standard:
#include <cstddef>
#include <array>

// Qt:
#include <QtWidgets/QWidget>
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/instrument.h>
#include <xefis/core/property.h>
#include <xefis/core/property_observer.h>
#include <xefis/core/setting.h>
#include <xefis/core/xefis.h>

// Local:
#include "adi_widget.h"


class ADI_IO: public xf::ModuleIO
{
  public:
	/*
	 * Settings
	 */

	// TODO change to si::Velocity
	xf::Setting<int64_t>			speed_ladder_line_every								{ this, "setting_speed_ladder_line_every", 10 };
	xf::Setting<int64_t>			speed_ladder_number_every							{ this, "setting_speed_ladder_number_every", 20 };
	xf::Setting<int64_t>			speed_ladder_extent									{ this, "setting_speed_ladder_extent", 124 };
	xf::Setting<int64_t>			speed_ladder_minimum								{ this, "setting_speed_ladder_minimum", 20 };
	xf::Setting<int64_t>			speed_ladder_maximum								{ this, "setting_speed_ladder_maximum", 350 };
	xf::Setting<int64_t>			altitude_ladder_line_every							{ this, "setting_altitude_ladder_line_every", 100 };
	xf::Setting<int64_t>			altitude_ladder_number_every						{ this, "setting_altitude_ladder_number_every", 200 };
	xf::Setting<int64_t>			altitude_ladder_emphasis_every						{ this, "setting_altitude_ladder_emphasis_every", 1000 };
	xf::Setting<int64_t>			altitude_ladder_bold_every							{ this, "setting_altitude_ladder_bold_every", 500 };
	xf::Setting<int64_t>			altitude_ladder_extent								{ this, "setting_altitude_ladder_extent", 825 };
	xf::Setting<si::Length>			altitude_landing_warning_hi							{ this, "setting_altitude_landing_warning_hi", 1000_ft };
	xf::Setting<si::Length>			altitude_landing_warning_lo							{ this, "setting_altitude_landing_warning_lo", 500_ft };
	xf::Setting<si::Length>			raising_runway_visibility							{ this, "setting_raising_runway_visibility", 1000_ft };
	xf::Setting<si::Length>			raising_runway_threshold							{ this, "setting_raising_runway_threshold", 250_ft };
	xf::Setting<si::Angle>			aoa_visibility_threshold							{ this, "setting_aoa_visibility_threshold", 17.5_deg };
	xf::Setting<double>				show_mach_above										{ this, "setting_show_mach_above", 0.4 };
	xf::Setting<si::Power>			power_eq_1000_fpm									{ this, "setting_power_eq_1000_fpm", 1000_W };

	/*
	 * Input
	 */

	// Speed
	xf::PropertyIn<bool>			speed_ias_serviceable								{ this, "/speed/ias.serviceable" };
	xf::PropertyIn<si::Velocity>	speed_ias											{ this, "/speed/ias" };
	xf::PropertyIn<si::Velocity>	speed_ias_lookahead									{ this, "/speed/ias.lookahead" };
	xf::PropertyIn<si::Velocity>	speed_ias_minimum									{ this, "/speed/ias.minimum" };
	xf::PropertyIn<si::Velocity>	speed_ias_minimum_maneuver							{ this, "/speed/ias.minimum.maneuver" };
	xf::PropertyIn<si::Velocity>	speed_ias_maximum_maneuver							{ this, "/speed/ias.maximum.maneuver" };
	xf::PropertyIn<si::Velocity>	speed_ias_maximum									{ this, "/speed/ias.maximum" };
	xf::PropertyIn<double>			speed_mach											{ this, "/speed/mach" };
	xf::PropertyIn<si::Velocity>	speed_ground										{ this, "/speed/ground-speed" };
	// Velocity bugs
	xf::PropertyIn<si::Velocity>	speed_v1											{ this, "/speed-bugs/v1" };
	xf::PropertyIn<si::Velocity>	speed_vr											{ this, "/speed-bugs/vr" };
	xf::PropertyIn<si::Velocity>	speed_vref											{ this, "/speed-bugs/vref" };
	xf::PropertyIn<std::string>		speed_flaps_up_label								{ this, "/speed-bugs/flaps-up.label" };
	xf::PropertyIn<si::Velocity>	speed_flaps_up_speed								{ this, "/speed-bugs/flaps-up.speed" };
	xf::PropertyIn<std::string>		speed_flaps_a_label									{ this, "/speed-bugs/flaps.a.label" };
	xf::PropertyIn<si::Velocity>	speed_flaps_a_speed									{ this, "/speed-bugs/flaps.a.speed" };
	xf::PropertyIn<std::string>		speed_flaps_b_label									{ this, "/speed-bugs/flaps.b.label" };
	xf::PropertyIn<si::Velocity>	speed_flaps_b_speed									{ this, "/speed-bugs/flaps.b.speed" };
	// Attitude and heading
	xf::PropertyIn<bool>			orientation_serviceable								{ this, "/orientation/serviceable" };
	xf::PropertyIn<si::Angle>		orientation_pitch									{ this, "/orientation/pitch" };
	xf::PropertyIn<si::Angle>		orientation_roll									{ this, "/orientation/roll" };
	xf::PropertyIn<si::Angle>		orientation_heading_magnetic						{ this, "/orientation/heading.magnetic" };
	xf::PropertyIn<si::Angle>		orientation_heading_true							{ this, "/orientation/heading.true" };
	xf::PropertyIn<bool>			orientation_heading_numbers_visible					{ this, "/orientation/heading-numbers-visible" };
	// Track
	xf::PropertyIn<si::Angle>		track_lateral_magnetic								{ this, "/track/lateral.magnetic" };
	xf::PropertyIn<si::Angle>		track_lateral_true									{ this, "/track/lateral.true" };
	xf::PropertyIn<si::Angle>		track_vertical										{ this, "/track/vertical" };
	// Slip-skid indicator
	xf::PropertyIn<si::Angle>		slip_skid											{ this, "/slip-skid/angle" };
	// Flight Path Vector
	xf::PropertyIn<bool>			fpv_visible											{ this, "/fpv/fpv-visible" };
	xf::PropertyIn<bool>			weight_on_wheels									{ this, "/fpv/weight-on-wheels" };
	// Angle of Attack
	xf::PropertyIn<si::Angle>		aoa_alpha											{ this, "/aoa/alpha" };
	xf::PropertyIn<si::Angle>		aoa_alpha_maximum									{ this, "/aoa/alpha.maximum" };
	xf::PropertyIn<bool>			aoa_alpha_visible									{ this, "/aoa/alpha.visible" };
	// Pressure and radio altitude
	xf::PropertyIn<bool>			altitude_amsl_serviceable							{ this, "/altitude/amsl.serviceable" };
	xf::PropertyIn<si::Length>		altitude_amsl										{ this, "/altitude/amsl" };
	xf::PropertyIn<si::Length>		altitude_amsl_lookahead								{ this, "/altitude/amsl.lookahead" };
	xf::PropertyIn<bool>			altitude_agl_serviceable							{ this, "/altitude/agl.serviceable" };
	xf::PropertyIn<si::Length>		altitude_agl										{ this, "/altitude/agl" };
	xf::PropertyIn<std::string>		altitude_minimums_type								{ this, "/altitude/minimums.type" };
	xf::PropertyIn<si::Length>		altitude_minimums_setting							{ this, "/altitude/minimums.setting" };
	xf::PropertyIn<si::Length>		altitude_minimums_amsl								{ this, "/altitude/minimums.amsl" };
	xf::PropertyIn<si::Length>		altitude_landing_amsl								{ this, "/altitude/landing-altitude.amsl" };
	// Vertical speed
	xf::PropertyIn<bool>			vertical_speed_serviceable							{ this, "/vertical-speed/serviceable" };
	xf::PropertyIn<si::Velocity>	vertical_speed										{ this, "/vertical-speed/speed" };
	xf::PropertyIn<si::Power>		vertical_speed_energy_variometer					{ this, "/vertical-speed/energy-variometer" };
	// Air pressure settings
	xf::PropertyIn<si::Pressure>	pressure_qnh										{ this, "/pressure/qnh" };
	xf::PropertyIn<bool>			pressure_display_hpa								{ this, "/pressure/display-hpa" };
	xf::PropertyIn<bool>			pressure_use_std									{ this, "/pressure/use-std" };
	// Flight director
	xf::PropertyIn<bool>			flight_director_serviceable							{ this, "/flight-director/serviceable" };
	xf::PropertyIn<bool>			flight_director_cmd_visible							{ this, "/flight-director/cmd-visible" };
	xf::PropertyIn<si::Length>		flight_director_cmd_altitude						{ this, "/flight-director/cmd.altitude" };
	xf::PropertyIn<bool>			flight_director_cmd_altitude_acquired				{ this, "/flight-director/cmd.altitude-acquired" };
	xf::PropertyIn<si::Velocity>	flight_director_cmd_ias								{ this, "/flight-director/cmd.ias" };
	xf::PropertyIn<double>			flight_director_cmd_mach							{ this, "/flight-director/cmd.mach" };
	xf::PropertyIn<si::Velocity>	flight_director_cmd_vertical_speed					{ this, "/flight-director/cmd.vertical-speed" };
	xf::PropertyIn<si::Angle>		flight_director_cmd_fpa								{ this, "/flight-director/cmd.fpa" };
	xf::PropertyIn<bool>			flight_director_guidance_visible					{ this, "/flight-director/guidance.visible" };
	xf::PropertyIn<si::Angle>		flight_director_guidance_pitch						{ this, "/flight-director/guidance.pitch" };
	xf::PropertyIn<si::Angle>		flight_director_guidance_roll						{ this, "/flight-director/guidance.roll" };
	// Stick position indicator TODO rename to control_surfaces+deflection.ailerons+deflection.elevator
	xf::PropertyIn<bool>			control_stick_visible								{ this, "/control-stick/visible" };
	xf::PropertyIn<si::Angle>		control_stick_pitch									{ this, "/control-stick/pitch" };
	xf::PropertyIn<si::Angle>		control_stick_roll									{ this, "/control-stick/roll" };
	// Approach information
	xf::PropertyIn<bool>			navaid_reference_visible							{ this, "/navaid/reference-visible" };
	xf::PropertyIn<si::Angle>		navaid_course_magnetic								{ this, "/navaid/course-magnetic" };
	xf::PropertyIn<std::string>		navaid_type_hint									{ this, "/navaid/type-hint" };
	xf::PropertyIn<std::string>		navaid_identifier									{ this, "/navaid/identifier" };
	xf::PropertyIn<si::Length>		navaid_distance										{ this, "/navaid/distance" };
	// Flight path deviation
	xf::PropertyIn<bool>			flight_path_deviation_lateral_serviceable			{ this, "/flight-path-deviation/lateral/serviceable" };
	xf::PropertyIn<si::Angle>		flight_path_deviation_lateral_app					{ this, "/flight-path-deviation/lateral/app" };
	xf::PropertyIn<si::Angle>		flight_path_deviation_lateral_fp					{ this, "/flight-path-deviation/lateral/fp" };
	xf::PropertyIn<bool>			flight_path_deviation_vertical_serviceable			{ this, "/flight-path-deviation/vertical/serviceable" };
	xf::PropertyIn<si::Angle>		flight_path_deviation_vertical						{ this, "/flight-path-deviation/vertical/deviation" };
	xf::PropertyIn<si::Angle>		flight_path_deviation_vertical_app					{ this, "/flight-path-deviation/vertical/app" };
	xf::PropertyIn<si::Angle>		flight_path_deviation_vertical_fp					{ this, "/flight-path-deviation/vertical/fp" };
	xf::PropertyIn<bool>			flight_path_deviation_mixed_mode					{ this, "/flight-path-deviation/mixed-mode" };
	// Flight mode information
	xf::PropertyIn<bool>			flight_mode_hint_visible							{ this, "/flight-mode/hint-visible" };
	xf::PropertyIn<std::string>		flight_mode_hint									{ this, "/flight-mode/hint" };
	xf::PropertyIn<bool>			flight_mode_fma_visible								{ this, "/flight-mode/fma.visible" };
	xf::PropertyIn<std::string>		flight_mode_fma_speed_hint							{ this, "/flight-mode/fma.speed-hint" };
	xf::PropertyIn<std::string>		flight_mode_fma_speed_armed_hint					{ this, "/flight-mode/fma.speed-armed-hint" };
	xf::PropertyIn<std::string>		flight_mode_fma_lateral_hint						{ this, "/flight-mode/fma.lateral-hint" };
	xf::PropertyIn<std::string>		flight_mode_fma_lateral_armed_hint					{ this, "/flight-mode/fma.lateral-armed-hint" };
	xf::PropertyIn<std::string>		flight_mode_fma_vertical_hint						{ this, "/flight-mode/fma.vertical-hint" };
	xf::PropertyIn<std::string>		flight_mode_fma_vertical_armed_hint					{ this, "/flight-mode/fma.vertical-armed-hint" };
	// TCAS
	xf::PropertyIn<si::Angle>		tcas_resolution_advisory_pitch_minimum				{ this, "/tcas/resolution-advisory/pitch.minimum" };
	xf::PropertyIn<si::Angle>		tcas_resolution_advisory_pitch_maximum				{ this, "/tcas/resolution-advisory/pitch.maximum" };
	xf::PropertyIn<si::Velocity>	tcas_resolution_advisory_vertical_speed_minimum		{ this, "/tcas/resolution-advisory/vertical-speed.minimum" };
	xf::PropertyIn<si::Velocity>	tcas_resolution_advisory_vertical_speed_maximum		{ this, "/tcas/resolution-advisory/vertical-speed.maximum" };
	// General warning/failure flags
	xf::PropertyIn<bool>			warning_novspd_flag									{ this, "/warnings/novspd-flag" };
	xf::PropertyIn<bool>			warning_ldgalt_flag									{ this, "/warnings/ldgalt-flag" };
	xf::PropertyIn<bool>			warning_pitch_disagree								{ this, "/warnings/pitch-disagree-flag" };
	xf::PropertyIn<bool>			warning_roll_disagree								{ this, "/warnings/roll-disagree-flag" };
	xf::PropertyIn<bool>			warning_ias_disagree								{ this, "/warnings/ias-disagree-flag" };
	xf::PropertyIn<bool>			warning_altitude_disagree							{ this, "/warnings/altitude-disagree-flag" };
	xf::PropertyIn<bool>			warning_roll										{ this, "/warnings/roll" };
	xf::PropertyIn<bool>			warning_slip_skid									{ this, "/warnings/slip-skid" };
	// Style
	xf::PropertyIn<bool>			style_old											{ this, "/style/use-old-style" };
	xf::PropertyIn<bool>			style_show_metric									{ this, "/style/show-metric-values" };
};


class ADI: public xf::Instrument<ADI_IO>
{
	Q_OBJECT

  public:
	// Ctor
	explicit
	ADI (std::unique_ptr<ADI_IO>, xf::WorkPerformer&, std::string const& instance = {});

  protected:
	// Module API
	void
	process (xf::Cycle const&) override;

  private:
	void
	compute_fpv();

  private:
	ADIWidget*				_adi_widget				{ nullptr };
	xf::PropertyObserver	_fpv_computer;
	bool					_computed_fpv_failure	{ false };
	bool					_computed_fpv_visible	{ false };
	Angle					_computed_fpv_alpha		{ 0_deg };
	Angle					_computed_fpv_beta		{ 0_deg };
	QString					_speed_flaps_up_current_label;
	QString					_speed_flaps_a_current_label;
	QString					_speed_flaps_b_current_label;
};

#endif

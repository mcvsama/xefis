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
#include <xefis/core/v2/instrument.h>
#include <xefis/core/v2/property.h>
#include <xefis/core/v2/setting.h>
#include <xefis/core/v2/property_observer.h>
#include <xefis/core/xefis.h>

// Local:
#include "adi_widget.h"


class ADI_IO: public v2::ModuleIO
{
  public:
	/*
	 * Settings
	 */

	// TODO change to si::Velocity
	v2::Setting<int64_t>			speed_ladder_line_every								{ this, 10 };
	v2::Setting<int64_t>			speed_ladder_number_every							{ this, 20 };
	v2::Setting<int64_t>			speed_ladder_extent									{ this, 124 };
	v2::Setting<int64_t>			speed_ladder_minimum								{ this, 20 };
	v2::Setting<int64_t>			speed_ladder_maximum								{ this, 350 };
	v2::Setting<int64_t>			altitude_ladder_line_every							{ this, 100 };
	v2::Setting<int64_t>			altitude_ladder_number_every						{ this, 200 };
	v2::Setting<int64_t>			altitude_ladder_emphasis_every						{ this, 1000 };
	v2::Setting<int64_t>			altitude_ladder_bold_every							{ this, 500 };
	v2::Setting<int64_t>			altitude_ladder_extent								{ this, 825 };
	v2::Setting<si::Length>			altitude_landing_warning_hi							{ this, 1000_ft };
	v2::Setting<si::Length>			altitude_landing_warning_lo							{ this, 500_ft };
	v2::Setting<si::Length>			raising_runway_visibility							{ this, 1000_ft };
	v2::Setting<si::Length>			raising_runway_threshold							{ this, 250_ft };
	v2::Setting<si::Angle>			aoa_visibility_threshold							{ this, 17.5_deg };
	v2::Setting<double>				show_mach_above										{ this, 0.4 };
	v2::Setting<si::Power>			power_eq_1000_fpm									{ this, 1000_W };

	/*
	 * Input
	 */

	// Speed
	v2::PropertyIn<bool>			speed_ias_serviceable								{ this, "/speed/ias.serviceable" };
	v2::PropertyIn<si::Velocity>	speed_ias											{ this, "/speed/ias" };
	v2::PropertyIn<si::Velocity>	speed_ias_lookahead									{ this, "/speed/ias.lookahead" };
	v2::PropertyIn<si::Velocity>	speed_ias_minimum									{ this, "/speed/ias.minimum" };
	v2::PropertyIn<si::Velocity>	speed_ias_minimum_maneuver							{ this, "/speed/ias.minimum.maneuver" };
	v2::PropertyIn<si::Velocity>	speed_ias_maximum_maneuver							{ this, "/speed/ias.maximum.maneuver" };
	v2::PropertyIn<si::Velocity>	speed_ias_maximum									{ this, "/speed/ias.maximum" };
	v2::PropertyIn<double>			speed_mach											{ this, "/speed/mach" };
	v2::PropertyIn<si::Velocity>	speed_ground										{ this, "/speed/ground-speed" };
	// Velocity bugs
	v2::PropertyIn<si::Velocity>	speed_v1											{ this, "/speed-bugs/v1" };
	v2::PropertyIn<si::Velocity>	speed_vr											{ this, "/speed-bugs/vr" };
	v2::PropertyIn<si::Velocity>	speed_vref											{ this, "/speed-bugs/vref" };
	v2::PropertyIn<std::string>		speed_flaps_up_label								{ this, "/speed-bugs/flaps-up.label" };
	v2::PropertyIn<si::Velocity>	speed_flaps_up_speed								{ this, "/speed-bugs/flaps-up.speed" };
	v2::PropertyIn<std::string>		speed_flaps_a_label									{ this, "/speed-bugs/flaps.a.label" };
	v2::PropertyIn<si::Velocity>	speed_flaps_a_speed									{ this, "/speed-bugs/flaps.a.speed" };
	v2::PropertyIn<std::string>		speed_flaps_b_label									{ this, "/speed-bugs/flaps.b.label" };
	v2::PropertyIn<si::Velocity>	speed_flaps_b_speed									{ this, "/speed-bugs/flaps.b.speed" };
	// Attitude and heading
	v2::PropertyIn<bool>			orientation_serviceable								{ this, "/orientation/serviceable" };
	v2::PropertyIn<si::Angle>		orientation_pitch									{ this, "/orientation/pitch" };
	v2::PropertyIn<si::Angle>		orientation_roll									{ this, "/orientation/roll" };
	v2::PropertyIn<si::Angle>		orientation_heading_magnetic						{ this, "/orientation/heading.magnetic" };
	v2::PropertyIn<si::Angle>		orientation_heading_true							{ this, "/orientation/heading.true" };
	v2::PropertyIn<bool>			orientation_heading_numbers_visible					{ this, "/orientation/heading-numbers-visible" };
	// Track
	v2::PropertyIn<si::Angle>		track_lateral_magnetic								{ this, "/track/lateral.magnetic" };
	v2::PropertyIn<si::Angle>		track_lateral_true									{ this, "/track/lateral.true" };
	v2::PropertyIn<si::Angle>		track_vertical										{ this, "/track/vertical" };
	// Slip-skid indicator
	v2::PropertyIn<si::Angle>		slip_skid											{ this, "/slip-skid/angle" };
	// Flight Path Vector
	v2::PropertyIn<bool>			fpv_visible											{ this, "/fpv/fpv-visible" };
	v2::PropertyIn<bool>			weight_on_wheels									{ this, "/fpv/weight-on-wheels" };
	// Angle of Attack
	v2::PropertyIn<si::Angle>		aoa_alpha											{ this, "/aoa/alpha" };
	v2::PropertyIn<si::Angle>		aoa_alpha_maximum									{ this, "/aoa/alpha.maximum" };
	v2::PropertyIn<bool>			aoa_alpha_visible									{ this, "/aoa/alpha.visible" };
	// Pressure and radio altitude
	v2::PropertyIn<bool>			altitude_amsl_serviceable							{ this, "/altitude/amsl.serviceable" };
	v2::PropertyIn<si::Length>		altitude_amsl										{ this, "/altitude/amsl" };
	v2::PropertyIn<si::Length>		altitude_amsl_lookahead								{ this, "/altitude/amsl.lookahead" };
	v2::PropertyIn<bool>			altitude_agl_serviceable							{ this, "/altitude/agl.serviceable" };
	v2::PropertyIn<si::Length>		altitude_agl										{ this, "/altitude/agl" };
	v2::PropertyIn<std::string>		altitude_minimums_type								{ this, "/altitude/minimums.type" };
	v2::PropertyIn<si::Length>		altitude_minimums_setting							{ this, "/altitude/minimums.setting" };
	v2::PropertyIn<si::Length>		altitude_minimums_amsl								{ this, "/altitude/minimums.amsl" };
	v2::PropertyIn<si::Length>		altitude_landing_amsl								{ this, "/altitude/landing-altitude.amsl" };
	// Vertical speed
	v2::PropertyIn<bool>			vertical_speed_serviceable							{ this, "/vertical-speed/serviceable" };
	v2::PropertyIn<si::Velocity>	vertical_speed										{ this, "/vertical-speed/speed" };
	v2::PropertyIn<si::Power>		vertical_speed_energy_variometer					{ this, "/vertical-speed/energy-variometer" };
	// Air pressure settings
	v2::PropertyIn<si::Pressure>	pressure_qnh										{ this, "/pressure/qnh" };
	v2::PropertyIn<bool>			pressure_display_hpa								{ this, "/pressure/display-hpa" };
	v2::PropertyIn<bool>			pressure_use_std									{ this, "/pressure/use-std" };
	// Flight director
	v2::PropertyIn<bool>			flight_director_serviceable							{ this, "/flight-director/serviceable" };
	v2::PropertyIn<bool>			flight_director_cmd_visible							{ this, "/flight-director/cmd-visible" };
	v2::PropertyIn<si::Length>		flight_director_cmd_altitude						{ this, "/flight-director/cmd.altitude" };
	v2::PropertyIn<bool>			flight_director_cmd_altitude_acquired				{ this, "/flight-director/cmd.altitude-acquired" };
	v2::PropertyIn<si::Velocity>	flight_director_cmd_ias								{ this, "/flight-director/cmd.ias" };
	v2::PropertyIn<double>			flight_director_cmd_mach							{ this, "/flight-director/cmd.mach" };
	v2::PropertyIn<si::Velocity>	flight_director_cmd_vertical_speed					{ this, "/flight-director/cmd.vertical-speed" };
	v2::PropertyIn<si::Angle>		flight_director_cmd_fpa								{ this, "/flight-director/cmd.fpa" };
	v2::PropertyIn<bool>			flight_director_guidance_visible					{ this, "/flight-director/guidance.visible" };
	v2::PropertyIn<si::Angle>		flight_director_guidance_pitch						{ this, "/flight-director/guidance.pitch" };
	v2::PropertyIn<si::Angle>		flight_director_guidance_roll						{ this, "/flight-director/guidance.roll" };
	// Stick position indicator TODO rename to control_surfaces+deflection.ailerons+deflection.elevator
	v2::PropertyIn<bool>			control_stick_visible								{ this, "/control-stick/visible" };
	v2::PropertyIn<si::Angle>		control_stick_pitch									{ this, "/control-stick/pitch" };
	v2::PropertyIn<si::Angle>		control_stick_roll									{ this, "/control-stick/roll" };
	// Approach information
	v2::PropertyIn<bool>			navaid_reference_visible							{ this, "/navaid/reference-visible" };
	v2::PropertyIn<si::Angle>		navaid_course_magnetic								{ this, "/navaid/course-magnetic" };
	v2::PropertyIn<std::string>		navaid_type_hint									{ this, "/navaid/type-hint" };
	v2::PropertyIn<std::string>		navaid_identifier									{ this, "/navaid/identifier" };
	v2::PropertyIn<si::Length>		navaid_distance										{ this, "/navaid/distance" };
	// Flight path deviation
	v2::PropertyIn<bool>			flight_path_deviation_lateral_serviceable			{ this, "/flight-path-deviation/lateral/serviceable" };
	v2::PropertyIn<si::Angle>		flight_path_deviation_lateral_app					{ this, "/flight-path-deviation/lateral/app" };
	v2::PropertyIn<si::Angle>		flight_path_deviation_lateral_fp					{ this, "/flight-path-deviation/lateral/fp" };
	v2::PropertyIn<bool>			flight_path_deviation_vertical_serviceable			{ this, "/flight-path-deviation/vertical/serviceable" };
	v2::PropertyIn<si::Angle>		flight_path_deviation_vertical						{ this, "/flight-path-deviation/vertical/deviation" };
	v2::PropertyIn<si::Angle>		flight_path_deviation_vertical_app					{ this, "/flight-path-deviation/vertical/app" };
	v2::PropertyIn<si::Angle>		flight_path_deviation_vertical_fp					{ this, "/flight-path-deviation/vertical/fp" };
	v2::PropertyIn<bool>			flight_path_deviation_mixed_mode					{ this, "/flight-path-deviation/mixed-mode" };
	// Flight mode information
	v2::PropertyIn<bool>			flight_mode_hint_visible							{ this, "/flight-mode/hint-visible" };
	v2::PropertyIn<std::string>		flight_mode_hint									{ this, "/flight-mode/hint" };
	v2::PropertyIn<bool>			flight_mode_fma_visible								{ this, "/flight-mode/fma.visible" };
	v2::PropertyIn<std::string>		flight_mode_fma_speed_hint							{ this, "/flight-mode/fma.speed-hint" };
	v2::PropertyIn<std::string>		flight_mode_fma_speed_armed_hint					{ this, "/flight-mode/fma.speed-armed-hint" };
	v2::PropertyIn<std::string>		flight_mode_fma_lateral_hint						{ this, "/flight-mode/fma.lateral-hint" };
	v2::PropertyIn<std::string>		flight_mode_fma_lateral_armed_hint					{ this, "/flight-mode/fma.lateral-armed-hint" };
	v2::PropertyIn<std::string>		flight_mode_fma_vertical_hint						{ this, "/flight-mode/fma.vertical-hint" };
	v2::PropertyIn<std::string>		flight_mode_fma_vertical_armed_hint					{ this, "/flight-mode/fma.vertical-armed-hint" };
	// TCAS
	v2::PropertyIn<si::Angle>		tcas_resolution_advisory_pitch_minimum				{ this, "/tcas/resolution-advisory/pitch.minimum" };
	v2::PropertyIn<si::Angle>		tcas_resolution_advisory_pitch_maximum				{ this, "/tcas/resolution-advisory/pitch.maximum" };
	v2::PropertyIn<si::Velocity>	tcas_resolution_advisory_vertical_speed_minimum		{ this, "/tcas/resolution-advisory/vertical-speed.minimum" };
	v2::PropertyIn<si::Velocity>	tcas_resolution_advisory_vertical_speed_maximum		{ this, "/tcas/resolution-advisory/vertical-speed.maximum" };
	// General warning/failure flags
	v2::PropertyIn<bool>			warning_novspd_flag									{ this, "/warnings/novspd-flag" };
	v2::PropertyIn<bool>			warning_ldgalt_flag									{ this, "/warnings/ldgalt-flag" };
	v2::PropertyIn<bool>			warning_pitch_disagree								{ this, "/warnings/pitch-disagree-flag" };
	v2::PropertyIn<bool>			warning_roll_disagree								{ this, "/warnings/roll-disagree-flag" };
	v2::PropertyIn<bool>			warning_ias_disagree								{ this, "/warnings/ias-disagree-flag" };
	v2::PropertyIn<bool>			warning_altitude_disagree							{ this, "/warnings/altitude-disagree-flag" };
	v2::PropertyIn<bool>			warning_roll										{ this, "/warnings/roll" };
	v2::PropertyIn<bool>			warning_slip_skid									{ this, "/warnings/slip-skid" };
	// Style
	v2::PropertyIn<bool>			style_old											{ this, "/style/use-old-style" };
	v2::PropertyIn<bool>			style_show_metric									{ this, "/style/show-metric-values" };
};


class ADI: public v2::Instrument<ADI_IO>
{
	Q_OBJECT

  public:
	// Ctor
	explicit
	ADI (std::unique_ptr<ADI_IO>, xf::Xefis*, std::string const& instance = {});

  protected:
	// Module API
	void
	process (v2::Cycle const&) override;

  private:
	void
	compute_fpv();

  private:
	ADIWidget*				_adi_widget				{ nullptr };
	v2::PropertyObserver	_fpv_computer;
	bool					_computed_fpv_failure	{ false };
	bool					_computed_fpv_visible	{ false };
	Angle					_computed_fpv_alpha		{ 0_deg };
	Angle					_computed_fpv_beta		{ 0_deg };
	QString					_speed_flaps_up_current_label;
	QString					_speed_flaps_a_current_label;
	QString					_speed_flaps_b_current_label;
};

#endif

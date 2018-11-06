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
#include <array>
#include <cstddef>

// Qt:
#include <QtGui/QColor>
#include <QtGui/QPainterPath>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/graphics.h>
#include <xefis/core/instrument.h>
#include <xefis/core/property.h>
#include <xefis/core/property_observer.h>
#include <xefis/core/setting.h>
#include <xefis/core/xefis.h>
#include <xefis/support/instrument/instrument_support.h>
#include <xefis/utility/event_timestamper.h>
#include <xefis/utility/synchronized.h>


class ADI_IO: public xf::ModuleIO
{
  public:
	/*
	 * Settings
	 */

	xf::Setting<int64_t>			speed_ladder_line_every								{ this, "speed_ladder_line_every", 10 };
	xf::Setting<int64_t>			speed_ladder_number_every							{ this, "speed_ladder_number_every", 20 };
	xf::Setting<int64_t>			speed_ladder_extent									{ this, "speed_ladder_extent", 124 };
	xf::Setting<int64_t>			speed_ladder_minimum								{ this, "speed_ladder_minimum", 20 };
	xf::Setting<int64_t>			speed_ladder_maximum								{ this, "speed_ladder_maximum", 350 };
	xf::Setting<int64_t>			altitude_ladder_line_every							{ this, "altitude_ladder_line_every", 100 };
	xf::Setting<int64_t>			altitude_ladder_number_every						{ this, "altitude_ladder_number_every", 200 };
	xf::Setting<int64_t>			altitude_ladder_emphasis_every						{ this, "altitude_ladder_emphasis_every", 1000 };
	xf::Setting<int64_t>			altitude_ladder_bold_every							{ this, "altitude_ladder_bold_every", 500 };
	xf::Setting<int64_t>			altitude_ladder_extent								{ this, "altitude_ladder_extent", 825 };
	xf::Setting<si::Length>			altitude_landing_warning_hi							{ this, "altitude_landing_warning_hi", 1000_ft };
	xf::Setting<si::Length>			altitude_landing_warning_lo							{ this, "altitude_landing_warning_lo", 500_ft };
	xf::Setting<si::Length>			raising_runway_visibility							{ this, "raising_runway_visibility", 1000_ft };
	xf::Setting<si::Length>			raising_runway_threshold							{ this, "raising_runway_threshold", 250_ft };
	xf::Setting<si::Angle>			aoa_visibility_threshold							{ this, "aoa_visibility_threshold", 17.5_deg };
	xf::Setting<double>				show_mach_above										{ this, "show_mach_above", 0.4 };
	xf::Setting<si::Power>			power_eq_1000_fpm									{ this, "power_eq_1000_fpm", 1000_W };
	xf::Setting<bool>				show_minimum_speeds_only_if_no_weight_on_wheels		{ this, "show_minimum_speeds_only_if_no_weight_on_wheels", true };
	xf::Setting<si::Time>			focus_duration										{ this, "focus_duration", 10_s };
	xf::Setting<si::Time>			focus_short_duration								{ this, "focus_short_duration", 5_s };

	/*
	 * Input
	 */

	xf::PropertyIn<bool>			weight_on_wheels									{ this, "weight-on-wheels" };
	// Speed
	xf::PropertyIn<bool>			speed_ias_serviceable								{ this, "speed/ias.serviceable" };
	xf::PropertyIn<si::Velocity>	speed_ias											{ this, "speed/ias" };
	xf::PropertyIn<si::Velocity>	speed_ias_lookahead									{ this, "speed/ias.lookahead" };
	xf::PropertyIn<si::Velocity>	speed_ias_minimum									{ this, "speed/ias.minimum" };
	xf::PropertyIn<si::Velocity>	speed_ias_minimum_maneuver							{ this, "speed/ias.minimum.maneuver" };
	xf::PropertyIn<si::Velocity>	speed_ias_maximum_maneuver							{ this, "speed/ias.maximum.maneuver" };
	xf::PropertyIn<si::Velocity>	speed_ias_maximum									{ this, "speed/ias.maximum" };
	xf::PropertyIn<double>			speed_mach											{ this, "speed/mach" };
	xf::PropertyIn<si::Velocity>	speed_ground										{ this, "speed/ground-speed" };
	// Velocity bugs
	xf::PropertyIn<si::Velocity>	speed_v1											{ this, "speed-bugs/v1" };
	xf::PropertyIn<si::Velocity>	speed_vr											{ this, "speed-bugs/vr" };
	xf::PropertyIn<si::Velocity>	speed_vref											{ this, "speed-bugs/vref" };
	xf::PropertyIn<std::string>		speed_flaps_up_label								{ this, "speed-bugs/flaps-up.label" };
	xf::PropertyIn<si::Velocity>	speed_flaps_up_speed								{ this, "speed-bugs/flaps-up.speed" };
	xf::PropertyIn<std::string>		speed_flaps_a_label									{ this, "speed-bugs/flaps.a.label" };
	xf::PropertyIn<si::Velocity>	speed_flaps_a_speed									{ this, "speed-bugs/flaps.a.speed" };
	xf::PropertyIn<std::string>		speed_flaps_b_label									{ this, "speed-bugs/flaps.b.label" };
	xf::PropertyIn<si::Velocity>	speed_flaps_b_speed									{ this, "speed-bugs/flaps.b.speed" };
	// Attitude and heading
	xf::PropertyIn<bool>			orientation_serviceable								{ this, "orientation/serviceable" };
	xf::PropertyIn<si::Angle>		orientation_pitch									{ this, "orientation/pitch" };
	xf::PropertyIn<si::Angle>		orientation_roll									{ this, "orientation/roll" };
	xf::PropertyIn<si::Angle>		orientation_heading_magnetic						{ this, "orientation/heading.magnetic" };
	xf::PropertyIn<si::Angle>		orientation_heading_true							{ this, "orientation/heading.true" };
	xf::PropertyIn<bool>			orientation_heading_numbers_visible					{ this, "orientation/heading-numbers-visible" };
	// Track
	xf::PropertyIn<si::Angle>		track_lateral_magnetic								{ this, "track/lateral.magnetic" };
	xf::PropertyIn<si::Angle>		track_lateral_true									{ this, "track/lateral.true" };
	xf::PropertyIn<si::Angle>		track_vertical										{ this, "track/vertical" };
	// Flight Path Vector
	xf::PropertyIn<bool>			fpv_visible											{ this, "fpv/fpv-visible" };
	// Slip-skid indicator
	xf::PropertyIn<si::Angle>		slip_skid											{ this, "slip-skid/angle" };
	// Angle of Attack
	xf::PropertyIn<si::Angle>		aoa_alpha											{ this, "aoa/alpha" };
	xf::PropertyIn<si::Angle>		aoa_alpha_maximum									{ this, "aoa/alpha.maximum" };
	xf::PropertyIn<bool>			aoa_alpha_visible									{ this, "aoa/alpha.visible" };
	// Pressure and radio altitude
	xf::PropertyIn<bool>			altitude_amsl_serviceable							{ this, "altitude/amsl.serviceable" };
	xf::PropertyIn<si::Length>		altitude_amsl										{ this, "altitude/amsl" };
	xf::PropertyIn<si::Length>		altitude_amsl_lookahead								{ this, "altitude/amsl.lookahead" };
	xf::PropertyIn<bool>			altitude_agl_serviceable							{ this, "altitude/agl.serviceable" };
	xf::PropertyIn<si::Length>		altitude_agl										{ this, "altitude/agl" };
	// Decision height:
	xf::PropertyIn<std::string>		decision_height_type								{ this, "decision-height/type" };
	xf::PropertyIn<si::Length>		decision_height_setting								{ this, "decision-height/setting" };
	xf::PropertyIn<si::Length>		decision_height_amsl								{ this, "decision-height/amsl" };
	// Landing altitude:
	xf::PropertyIn<si::Length>		landing_amsl										{ this, "landing-altitude/amsl" };
	// Vertical speed
	xf::PropertyIn<bool>			vertical_speed_serviceable							{ this, "vertical-speed/serviceable" };
	xf::PropertyIn<si::Velocity>	vertical_speed										{ this, "vertical-speed/speed" };
	xf::PropertyIn<si::Power>		vertical_speed_energy_variometer					{ this, "vertical-speed/energy-variometer" };
	// Air pressure settings
	xf::PropertyIn<si::Pressure>	pressure_qnh										{ this, "pressure/qnh" };
	xf::PropertyIn<bool>			pressure_display_hpa								{ this, "pressure/display-hpa" };
	xf::PropertyIn<bool>			pressure_use_std									{ this, "pressure/use-std" };
	// Flight director
	xf::PropertyIn<bool>			flight_director_serviceable							{ this, "flight-director/serviceable" };
	xf::PropertyIn<std::string>		flight_director_active_name							{ this, "flight-director/active-name" };
	xf::PropertyIn<bool>			flight_director_cmd_visible							{ this, "flight-director/cmd-visible" };
	xf::PropertyIn<si::Length>		flight_director_cmd_altitude						{ this, "flight-director/cmd.altitude" };
	xf::PropertyIn<bool>			flight_director_cmd_altitude_acquired				{ this, "flight-director/cmd.altitude-acquired" };
	xf::PropertyIn<si::Velocity>	flight_director_cmd_ias								{ this, "flight-director/cmd.ias" };
	xf::PropertyIn<double>			flight_director_cmd_mach							{ this, "flight-director/cmd.mach" };
	xf::PropertyIn<si::Velocity>	flight_director_cmd_vertical_speed					{ this, "flight-director/cmd.vertical-speed" };
	xf::PropertyIn<si::Angle>		flight_director_cmd_fpa								{ this, "flight-director/cmd.fpa" };
	xf::PropertyIn<bool>			flight_director_guidance_visible					{ this, "flight-director/guidance.visible" };
	xf::PropertyIn<si::Angle>		flight_director_guidance_pitch						{ this, "flight-director/guidance.pitch" };
	xf::PropertyIn<si::Angle>		flight_director_guidance_roll						{ this, "flight-director/guidance.roll" };
	// Control surfaces deflection indicator
	xf::PropertyIn<bool>			control_surfaces_visible							{ this, "control-surfaces/visible" };
	xf::PropertyIn<double>			control_surfaces_elevator							{ this, "control-surfaces/elevator" };
	xf::PropertyIn<double>			control_surfaces_ailerons							{ this, "control-surfaces/ailerons" };
	// Approach information
	xf::PropertyIn<bool>			navaid_reference_visible							{ this, "navaid/reference-visible" };
	xf::PropertyIn<si::Angle>		navaid_course_magnetic								{ this, "navaid/course-magnetic" };
	xf::PropertyIn<std::string>		navaid_type_hint									{ this, "navaid/type-hint" };
	xf::PropertyIn<std::string>		navaid_identifier									{ this, "navaid/identifier" };
	xf::PropertyIn<si::Length>		navaid_distance										{ this, "navaid/distance" };
	// Flight path deviation
	xf::PropertyIn<bool>			flight_path_deviation_lateral_serviceable			{ this, "flight-path-deviation/lateral/serviceable" };
	xf::PropertyIn<si::Angle>		flight_path_deviation_lateral_approach				{ this, "flight-path-deviation/lateral/approach" };
	xf::PropertyIn<si::Angle>		flight_path_deviation_lateral_flight_path			{ this, "flight-path-deviation/lateral/flight-path" };
	xf::PropertyIn<bool>			flight_path_deviation_vertical_serviceable			{ this, "flight-path-deviation/vertical/serviceable" };
	xf::PropertyIn<si::Angle>		flight_path_deviation_vertical						{ this, "flight-path-deviation/vertical/deviation" };
	xf::PropertyIn<si::Angle>		flight_path_deviation_vertical_approach				{ this, "flight-path-deviation/vertical/approach" };
	xf::PropertyIn<si::Angle>		flight_path_deviation_vertical_flight_path			{ this, "flight-path-deviation/vertical/flight-path" };
	xf::PropertyIn<bool>			flight_path_deviation_mixed_mode					{ this, "flight-path-deviation/mixed-mode" };
	// Flight mode information
	xf::PropertyIn<bool>			flight_mode_hint_visible							{ this, "flight-mode/hint-visible" };
	xf::PropertyIn<std::string>		flight_mode_hint									{ this, "flight-mode/hint" };
	xf::PropertyIn<bool>			flight_mode_fma_visible								{ this, "flight-mode/fma.visible" };
	xf::PropertyIn<std::string>		flight_mode_fma_speed_hint							{ this, "flight-mode/fma.speed-hint" };
	xf::PropertyIn<std::string>		flight_mode_fma_speed_armed_hint					{ this, "flight-mode/fma.speed-armed-hint" };
	xf::PropertyIn<std::string>		flight_mode_fma_lateral_hint						{ this, "flight-mode/fma.lateral-hint" };
	xf::PropertyIn<std::string>		flight_mode_fma_lateral_armed_hint					{ this, "flight-mode/fma.lateral-armed-hint" };
	xf::PropertyIn<std::string>		flight_mode_fma_vertical_hint						{ this, "flight-mode/fma.vertical-hint" };
	xf::PropertyIn<std::string>		flight_mode_fma_vertical_armed_hint					{ this, "flight-mode/fma.vertical-armed-hint" };
	// TCAS
	xf::PropertyIn<si::Angle>		tcas_resolution_advisory_pitch_minimum				{ this, "tcas/resolution-advisory/pitch.minimum" };
	xf::PropertyIn<si::Angle>		tcas_resolution_advisory_pitch_maximum				{ this, "tcas/resolution-advisory/pitch.maximum" };
	xf::PropertyIn<si::Velocity>	tcas_resolution_advisory_vertical_speed_minimum		{ this, "tcas/resolution-advisory/vertical-speed.minimum" };
	xf::PropertyIn<si::Velocity>	tcas_resolution_advisory_vertical_speed_maximum		{ this, "tcas/resolution-advisory/vertical-speed.maximum" };
	// General warning/failure flags
	xf::PropertyIn<bool>			warning_novspd_flag									{ this, "warnings/novspd-flag" };
	xf::PropertyIn<bool>			warning_ldgalt_flag									{ this, "warnings/ldgalt-flag" };
	xf::PropertyIn<bool>			warning_pitch_disagree								{ this, "warnings/pitch-disagree-flag" };
	xf::PropertyIn<bool>			warning_roll_disagree								{ this, "warnings/roll-disagree-flag" };
	xf::PropertyIn<bool>			warning_ias_disagree								{ this, "warnings/ias-disagree-flag" };
	xf::PropertyIn<bool>			warning_altitude_disagree							{ this, "warnings/altitude-disagree-flag" };
	xf::PropertyIn<bool>			warning_roll										{ this, "warnings/roll" };
	xf::PropertyIn<bool>			warning_slip_skid									{ this, "warnings/slip-skid" };
	// Style
	xf::PropertyIn<bool>			style_old											{ this, "style/use-old-style" };
	xf::PropertyIn<bool>			style_show_metric									{ this, "style/show-metric-values" };
};


namespace adi_detail {

class Parameters
{
	// TODO change to vector<> for speed
	using VelocityBugs	= std::map<QString, Velocity>;
	using AltitudeBugs	= std::map<QString, Length>;

  public:
	si::Time					timestamp;
	si::Time					focus_duration;
	si::Time					focus_short_duration;
	// TODO perhaps remove _visible and use std::optional<>
	bool						old_style							= false;
	bool						show_metric							= false;
	si::Angle					fov									= 120_deg;
	bool						input_alert_visible					= false;
	// Velocity
	bool						speed_failure						= false;
	bool						speed_failure_focus					= false;
	bool						speed_visible						= false;
	si::Velocity				speed								= 0_kt;
	bool						speed_lookahead_visible				= false;
	si::Velocity				speed_lookahead						= 0_kt;
	bool						speed_minimum_visible				= false;
	Velocity					speed_minimum						= 0_kt;
	std::optional<si::Velocity>	speed_minimum_maneuver;
	std::optional<si::Velocity>	speed_maximum_maneuver;
	bool						speed_maximum_visible				= false;
	si::Velocity				speed_maximum						= 0_kt;
	bool						speed_mach_visible					= false;
	double						speed_mach							= 0.0;
	std::optional<si::Velocity>	speed_ground;
	VelocityBugs				speed_bugs;
	// Orientation
	bool						orientation_failure					= false;
	bool						orientation_failure_focus			= false;
	bool						orientation_pitch_visible			= false;
	si::Angle					orientation_pitch					= 0_deg;
	bool						orientation_roll_visible			= false;
	si::Angle					orientation_roll					= 0_deg;
	bool						orientation_heading_visible			= false;
	si::Angle					orientation_heading					= 0_deg;
	bool						orientation_heading_numbers_visible	= false;
	// Slip-skid
	bool						slip_skid_visible					= false;
	si::Angle					slip_skid							= 0_deg;
	// Flight path vector
	bool						flight_path_marker_failure			= false;
	bool						flight_path_marker_failure_focus	= false;
	bool						flight_path_visible					= false;
	si::Angle					flight_path_alpha					= 0_deg;
	si::Angle					flight_path_beta					= 0_deg;
	// AOA limit
	bool						critical_aoa_visible				= false;
	si::Angle					critical_aoa						= 0_deg;
	si::Angle					aoa_alpha							= 0_deg;
	// Altitude
	bool						altitude_failure					= false;
	bool						altitude_failure_focus				= false;
	bool						altitude_visible					= false;
	si::Length					altitude_amsl						= 0_ft;
	bool						altitude_lookahead_visible			= false;
	si::Length					altitude_lookahead					= 0_ft;
	bool						altitude_agl_failure				= false;
	bool						altitude_agl_failure_focus			= false;
	bool						altitude_agl_visible				= false;
	si::Length					altitude_agl						= 0_ft;
	bool						altitude_agl_focus					= false;
	bool						landing_visible						= false;
	si::Length					landing_amsl						= 0_ft;
	si::Length					altitude_landing_warning_hi			= 0_ft;
	si::Length					altitude_landing_warning_lo			= 0_ft;
	AltitudeBugs				altitude_bugs;
	// Decision height
	bool						decision_height_visible				= false;
	QString						decision_height_type;
	si::Length					decision_height_amsl				= 0_ft;
	bool						decision_height_focus				= false;
	bool						decision_height_focus_short			= false;
	si::Length					decision_height_setting				= 0_ft;
	// Vertical speed
	bool						vertical_speed_failure				= false;
	bool						vertical_speed_failure_focus		= false;
	bool						vertical_speed_visible				= false;
	si::Velocity				vertical_speed						= 0_fpm;
	bool						energy_variometer_visible			= false;
	si::Power					energy_variometer_rate				= 0_W;
	si::Power					energy_variometer_1000_fpm_power	= 0_W;
	// Pressure settings
	bool						pressure_visible					= false;
	si::Pressure				pressure_qnh						= 0_inHg;
	bool						pressure_display_hpa				= false;
	bool						use_standard_pressure				= false;
	// Command settings
	std::optional<std::string>	flight_director_active_name;
	std::optional<si::Velocity>	cmd_speed;
	std::optional<double>		cmd_mach;
	std::optional<si::Length>	cmd_altitude;
	std::optional<si::Velocity>	cmd_vertical_speed;
	std::optional<si::Angle>	cmd_fpa;
	bool						cmd_altitude_acquired				= false;
	// Flight director
	bool						flight_director_failure				= false;
	bool						flight_director_failure_focus		= false;
	bool						flight_director_pitch_visible		= false;
	si::Angle					flight_director_pitch				= 0_deg;
	bool						flight_director_roll_visible		= false;
	si::Angle					flight_director_roll				= 0_deg;
	// Control stick
	bool						control_surfaces_visible			= false;
	float						control_surfaces_elevator			= 0.0f;
	float						control_surfaces_ailerons			= 0.0f;
	// Approach reference
	bool						navaid_reference_visible			= false;
	std::optional<si::Angle>	navaid_course_magnetic;
	QString						navaid_hint;
	QString						navaid_identifier;
	std::optional<si::Length>	navaid_distance;
	// Approach, flight path deviations
	bool						deviation_vertical_failure			= false;
	bool						deviation_vertical_failure_focus	= false;
	std::optional<si::Angle>	deviation_vertical_approach;
	std::optional<si::Angle>	deviation_vertical_flight_path;
	bool						deviation_lateral_failure			= false;
	bool						deviation_lateral_failure_focus		= false;
	std::optional<si::Angle>	deviation_lateral_approach;
	std::optional<si::Angle>	deviation_lateral_flight_path;
	bool						deviation_mixed_mode				= false;
	// Raising runway
	bool						runway_visible						= false;
	si::Angle					runway_position						= 0_deg;
	// Control hint
	bool						control_hint_visible				= false;
	QString						control_hint;
	bool						control_hint_focus					= false;
	// FMA
	bool						fma_visible							= false;
	QString						fma_speed_hint;
	bool						fma_speed_focus						= false;
	QString						fma_speed_armed_hint;
	bool						fma_speed_armed_focus				= false;
	QString						fma_lateral_hint;
	bool						fma_lateral_focus					= false;
	QString						fma_lateral_armed_hint;
	bool						fma_lateral_armed_focus				= false;
	QString						fma_vertical_hint;
	bool						fma_vertical_focus					= false;
	QString						fma_vertical_armed_hint;
	bool						fma_vertical_armed_focus			= false;
	// TCAS
	std::optional<si::Angle>	tcas_ra_pitch_minimum;
	std::optional<si::Angle>	tcas_ra_pitch_maximum;
	std::optional<si::Velocity>	tcas_ra_vertical_speed_minimum;
	std::optional<si::Velocity>	tcas_ra_vertical_speed_maximum;
	// Warning flags
	bool						novspd_flag							= false;
	bool						ldgalt_flag							= false;
	bool						pitch_disagree						= false;
	bool						pitch_disagree_focus				= false;
	bool						roll_disagree						= false;
	bool						roll_disagree_focus					= false;
	bool						ias_disagree						= false;
	bool						altitude_disagree					= false;
	bool						roll_warning						= false;
	bool						slip_skid_warning					= false;
	// Velocity ladder
	si::Velocity				vl_extent							= 124_kt;
	int							vl_minimum							= 0;
	int							vl_maximum							= 9999;
	int							vl_line_every						= 10;
	int							vl_number_every						= 20;
	// Altitude ladder
	si::Length					al_extent							= 825_ft;
	int							al_emphasis_every					= 1000;
	int							al_bold_every						= 500;
	int							al_number_every						= 200;
	int							al_line_every						= 100;

  public:
	/**
	 * Sanitize all parameters.
	 */
	void
	sanitize();
};


/**
 * Data that gets changed when size changes or some majoro setting change.
 * Otherwise does not need to be recomputed on each repaint.
 */
class Precomputed
{
  public:
	QTransform	center_transform;
};


class Blinker
{
  public:
	// Ctor
	Blinker (si::Time period);

	/**
	 * True if blinking is active.
	 */
	bool
	active() const noexcept;

	/**
	 * True if blinked object should be visible at the moment.
	 */
	bool
	visibility_state() const noexcept;

	/**
	 * Update blinker with new condition to blink.
	 * If it's true, blinker starts unless already blinking,
	 * otherwise stops unless already stopped.
	 */
	void
	update (bool condition);

	/**
	 * Update current time information, needed to properly blink
	 * the blinker.
	 */
	void
	update_current_time (si::Time now);

  private:
	si::Time const			_period;
	std::optional<si::Time>	_start_timestamp;
	bool					_active				{ false };
	bool					_visibility_state	{ false };
};


/**
 * Includes some painting helpers and values that usually change between paints.
 */
class AdiPaintRequest
{
  public:
	// Ctor
	explicit
	AdiPaintRequest (xf::PaintRequest const&, xf::InstrumentSupport const&, Parameters const&, Precomputed const&, Blinker const&, Blinker const&);

  public:
	static inline QColor const				kLadderColor		{ 64, 51, 108, 0x80 };
	static inline QColor const				kLadderBorderColor	{ kLadderColor.darker (120) };

  public:
	xf::PaintRequest const&					paint_request;
	Parameters const&						params;
	Precomputed const&						precomputed;
	xf::InstrumentPainter					painter;
	std::shared_ptr<xf::InstrumentAids>		aids_ptr;
	xf::InstrumentAids&						aids;
	Blinker const&							speed_warning_blinker;
	Blinker const&							decision_height_warning_blinker;
	float const								q;
	xf::Shadow								default_shadow;
	xf::Shadow								black_shadow;

  public:
	float
	pitch_to_px (si::Angle degrees) const;

	float
	heading_to_px (si::Angle degrees) const;

	/**
	 * Render 'rotatable' value on speed/altitude black box.
	 *
	 * \param	painter
	 *			Painter to use.
	 * \param	position
	 *			Text position, [-0.5, 0.5].
	 * \param	next, curr, prev
	 *			Texts to render. Special value "G" paints green dashed zone, "R" paints red dashed zone.
	 */
	void
	paint_rotating_value (QRectF const& rect, float position, float height_scale,
						  QString const& next, QString const& curr, QString const& prev);

	/**
	 * \param	two_zeros
	 *			Two separate zeros, for positive and negative values.
	 * \param	zero_mark
	 *			Draw red/green/blank mark instead of zero.
	 */
	void
	paint_rotating_digit (QRectF const& box, float value, int round_target, float const height_scale, float const delta, float const phase,
						  bool two_zeros, bool zero_mark, bool black_zero = false);

	/**
	 * Used by paint_rotating_value().
	 */
	void
	paint_dashed_zone (QColor const&, QRectF const& target);

	/**
	 * Paint horizontal failure flag.
	 */
	void
	paint_horizontal_failure_flag (QString const& message, QPointF const& center, QFont const&, QColor, bool focused);

	/**
	 * Paint vertical failure flag.
	 */
	void
	paint_vertical_failure_flag (QString const& message, QPointF const& center, QFont const&, QColor, bool focused);

	/**
	 * Return green or yellow color for minimums marker, depending on current altitude and minimums settings.
	 */
	QColor
	get_decision_height_color() const;
};


class ArtificialHorizon
{
  public:
	void
	paint (AdiPaintRequest&) const;

  private:
	void
	precompute (AdiPaintRequest&) const;

	void
	precompute (AdiPaintRequest&);

	void
	clear (AdiPaintRequest&) const;

	void
	paint_horizon (AdiPaintRequest&) const;

	void
	paint_pitch_scale (AdiPaintRequest&) const;

	void
	paint_heading (AdiPaintRequest&) const;

	void
	paint_tcas_ra (AdiPaintRequest&) const;

	void
	paint_roll_scale (AdiPaintRequest&) const;

	void
	paint_pitch_disagree (AdiPaintRequest&) const;

	void
	paint_roll_disagree (AdiPaintRequest&) const;

	void
	paint_flight_path_marker (AdiPaintRequest&) const;

	void
	paint_orientation_failure (AdiPaintRequest&) const;

	void
	paint_flight_path_marker_failure (AdiPaintRequest&) const;

	void
	paint_flight_director_failure (AdiPaintRequest&) const;

	xf::Shadow
	get_shadow (AdiPaintRequest&, int degrees) const;

  private:
	static QColor
	get_darker_alpha (QColor color, int darker, int alpha)
	{
		color = color.darker (darker);
		color.setAlpha (alpha);
		return color;
	}

  private:
	static inline QColor const	kSkyColor		{ QColor::fromHsv (213, 230, 255) };
	static inline QColor const	kSkyShadow		{ get_darker_alpha (kSkyColor, 400, 127) };
	static inline QColor const	kGroundColor	{ QColor::fromHsv (34, 255, 125) };
	static inline QColor const	kGroundShadow	{ get_darker_alpha (kGroundColor, 400, 127) };

  private:
	xf::Synchronized<ArtificialHorizon*> mutable
					_mutable_this { this };

	QTransform		_pitch_transform;
	QTransform		_roll_transform;
	QTransform		_heading_transform;
	QTransform		_horizon_transform;
	QTransform		_fast_horizon_transform;
	QRectF			_sky_rect;
	QRectF			_gnd_rect;
	QPainterPath	_flight_path_marker_shape;
	QPointF			_flight_path_marker_position;
	QPainterPath	_old_horizon_clip;
	QPainterPath	_pitch_scale_clipping_path;
};


class VelocityLadder
{
  public:
	void
	paint (AdiPaintRequest&) const;

  private:
	void
	precompute (AdiPaintRequest&) const;

	void
	precompute (AdiPaintRequest&);

	void
	paint_black_box (AdiPaintRequest&, float x) const;

	void
	paint_ias_disagree (AdiPaintRequest&, float x) const;

	void
	paint_ladder_scale (AdiPaintRequest&, float x) const;

	void
	paint_speed_limits (AdiPaintRequest&, float x) const;

	void
	paint_speed_tendency (AdiPaintRequest&, float x) const;

	void
	paint_bugs (AdiPaintRequest&, float x) const;

	void
	paint_mach_or_gs (AdiPaintRequest&, float x) const;

	void
	paint_ap_setting (AdiPaintRequest&) const;

	void
	paint_novspd_flag (AdiPaintRequest&) const;

	void
	paint_failure (AdiPaintRequest&) const;

	float
	kt_to_px (AdiPaintRequest&, Velocity) const;

  private:
	xf::Synchronized<VelocityLadder*> mutable
				_mutable_this { this };

	QTransform		_transform;
	Velocity		_min_shown;
	Velocity		_max_shown;
	int				_rounded_speed;
	QRectF			_ladder_rect;
	QPainterPath	_ladder_clip_path;
	QPen			_ladder_pen;
	QRectF			_black_box_rect;
	QPen			_black_box_pen;
	QPen			_scale_pen;
	QPen			_speed_bug_pen;
	float			_margin;
	int				_digits;
	QPolygonF		_bug_shape;
};


class AltitudeLadder
{
  public:
	void
	paint (AdiPaintRequest&) const;

  private:
	void
	precompute (AdiPaintRequest&) const;

	void
	precompute (AdiPaintRequest&);

	void
	paint_black_box (AdiPaintRequest&, float x) const;

	void
	paint_altitude_disagree (AdiPaintRequest&, float x) const;

	void
	paint_ladder_scale (AdiPaintRequest&, float x) const;

	void
	paint_altitude_tendency (AdiPaintRequest&, float x) const;

	void
	paint_bugs (AdiPaintRequest&, float x) const;

	void
	paint_vertical_speed (AdiPaintRequest&, float x) const;

	void
	paint_vertical_ap_setting (AdiPaintRequest&, float const x) const;

	void
	paint_pressure (AdiPaintRequest&, float x) const;

	void
	paint_ap_setting (AdiPaintRequest&) const;

	void
	paint_ldgalt_flag (AdiPaintRequest&, float x) const;

	void
	paint_vertical_speed_failure (AdiPaintRequest&, float x) const;

	void
	paint_failure (AdiPaintRequest&) const;

	float
	scale_vertical_speed (si::Velocity vertical_speed, float max_value = 1.f) const;

	float
	scale_energy_variometer (AdiPaintRequest&, si::Power power, float max_value = 1.f) const;

	float
	ft_to_px (AdiPaintRequest&, si::Length) const;

  private:
	xf::Synchronized<AltitudeLadder*> mutable
						_mutable_this { this };

	QTransform			_transform;
	Length				_min_shown;
	Length				_max_shown;
	int					_rounded_altitude;
	QRectF				_ladder_rect;
	QPainterPath		_ladder_clip_path;
	QPainterPath		_decision_height_clip_path;
	QPen				_ladder_pen;
	QRectF				_black_box_rect;
	QRectF				_metric_box_rect;
	QPen				_black_box_pen;
	QPen				_scale_pen_1;
	QPen				_scale_pen_2; // Bold one, each 500 ft
	QPen				_negative_altitude_pen;
	QPen				_altitude_bug_pen;
	QPen				_ldg_alt_pen;
	QRectF				_b_digits_box;
	QRectF				_s_digits_box;
	float				_margin;
	std::optional<bool>	_previous_show_metric;
};


class PaintingWork
{
  public:
	// Ctor
	explicit
	PaintingWork (xf::Graphics const&);

	void
	paint (xf::PaintRequest const&, Parameters const& parameters) const;

  private:
	void
	precompute (AdiPaintRequest&, Parameters const&) const;

	void
	precompute (AdiPaintRequest&, Parameters const&);

	void
	paint_center_cross (AdiPaintRequest&, bool center_box, bool rest) const;

	void
	paint_flight_director (AdiPaintRequest&) const;

	void
	paint_control_surfaces (AdiPaintRequest&) const;

	void
	paint_altitude_agl (AdiPaintRequest&) const;

	void
	paint_decision_height_setting (AdiPaintRequest&) const;

	void
	paint_nav (AdiPaintRequest&) const;

	void
	paint_hints (AdiPaintRequest&) const;

	void
	paint_critical_aoa (AdiPaintRequest&) const;

	void
	paint_input_alert (AdiPaintRequest&) const;

	void
	paint_radar_altimeter_failure (AdiPaintRequest&) const;

  private:
	xf::Synchronized<PaintingWork*>	mutable
							_mutable_this						{ this };

	Parameters				_parameters;
	Precomputed				_precomputed;
	xf::InstrumentSupport	_instrument_support;

	ArtificialHorizon		_artificial_horizon;
	VelocityLadder			_velocity_ladder;
	AltitudeLadder			_altitude_ladder;
	Blinker					_speed_warning_blinker				{ 200_ms };
	Blinker					_decision_height_warning_blinker	{ 200_ms };
};

} // namespace adi_detail


class ADI: public xf::Instrument<ADI_IO>
{
  public:
	// Ctor
	explicit
	ADI (std::unique_ptr<ADI_IO>, xf::Graphics const&, std::string_view const& instance = {});

	// Module API
	void
	process (xf::Cycle const&) override;

	// Instrument API
	std::packaged_task<void()>
	paint (xf::PaintRequest) const override;

  private:
	void
	compute_fpv();

	/**
	 * True if property is not nil and is finite.
	 */
	template<class FloatingPoint>
		bool
		is_sane (xf::Property<FloatingPoint> const&);

	/**
	 * True if property is in given range (which implies not nil and finite also).
	 */
	template<class FloatingPoint>
		bool
		is_sane (xf::Property<FloatingPoint> const&, xf::Range<FloatingPoint> const&);

  private:
	xf::PropertyObserver						_fpv_computer;
	adi_detail::PaintingWork					_painting_work;
	xf::Synchronized<adi_detail::Parameters>	_parameters;
	xf::EventTimestamper						_decision_height_became_visible;
	xf::EventTimestamper						_altitude_agl_became_visible;
	xf::EventTimestamper						_speed_failure_timestamp;
	xf::EventTimestamper						_orientation_failure_timestamp;
	xf::EventTimestamper						_flight_path_marker_failure_timestamp;
	xf::EventTimestamper						_altitude_failure_timestamp;
	xf::EventTimestamper						_altitude_agl_failure_timestamp;
	xf::EventTimestamper						_vertical_speed_failure_timestamp;
	xf::EventTimestamper						_flight_director_failure_timestamp;
	xf::EventTimestamper						_deviation_vertical_failure_timestamp;
	xf::EventTimestamper						_deviation_lateral_failure_timestamp;
	xf::EventTimestamper						_pitch_disagree_timestamp;
	xf::EventTimestamper						_roll_disagree_timestamp;
	bool										_computed_fpv_failure	{ false };
	bool										_computed_fpv_visible	{ false };
	Angle										_computed_fpv_alpha		{ 0_deg };
	Angle										_computed_fpv_beta		{ 0_deg };
	QString										_speed_flaps_up_current_label;
	QString										_speed_flaps_a_current_label;
	QString										_speed_flaps_b_current_label;
};

#endif


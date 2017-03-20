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

#ifndef XEFIS__MODULES__SYSTEMS__AFCS_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__AFCS_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/module.h>
#include <xefis/core/v2/property.h>
#include <xefis/core/v2/setting.h>
#include <xefis/utility/v2/callback_actions.h>
#include <xefis/utility/v2/delta_decoder.h>
#include <xefis/utility/range.h>

// Local:
#include "afcs_api.h"


/**
 * Controls AFCS logic. Gets input from Mode Control Panel,
 * makes outputs for displays, LEDs, annunciators, also for commanded values (altitude, speed, etc).
 */
class AFCS: public x2::Module
{
  public:
	/*
	 * Settings
	 */

	x2::Setting<si::Velocity>		setting_acq_delta_ias				{ this, 2_kt };
	x2::Setting<double>				setting_acq_delta_mach				{ this, 0.0033 };
	x2::Setting<si::Angle>			setting_acq_delta_heading			{ this, 2_deg };
	x2::Setting<si::Length>			setting_acq_delta_altitude			{ this, 100_ft };
	x2::Setting<si::Velocity>		setting_vs_rounding					{ this, 100_fpm };
	x2::Setting<si::Angle>			setting_fpa_rounding				{ this, 0.1_deg };
	x2::Setting<std::string>		setting_mcp_speed_format_kias		{ this, "%d" };
	x2::Setting<std::string>		setting_mcp_speed_format_mach		{ this, "%.3f" };
	x2::Setting<std::string>		setting_mcp_heading_format			{ this, "%03d" };
	x2::Setting<std::string>		setting_mcp_altitude_format			{ this, "%d" };
	x2::Setting<std::string>		setting_mcp_vertical_format_vs		{ this, "%+d" };
	x2::Setting<std::string>		setting_mcp_vertical_format_fpa		{ this, "%.1f" };
	x2::Setting<si::Velocity>		setting_default_ias					{ this };
	x2::Setting<double>				setting_default_mach				{ this };

	/*
	 * Input - measurements
	 */

	x2::PropertyIn<si::Velocity>	input_measured_ias					{ this, "/measurements/ias" };
	x2::PropertyIn<double>			input_measured_mach					{ this, "/measurements/mach" };
	x2::PropertyIn<si::Angle>		input_measured_heading_magnetic		{ this, "/measurements/heading/magnetic" };
	x2::PropertyIn<si::Angle>		input_measured_track_magnetic		{ this, "/measurements/track/magnetic" };
	x2::PropertyIn<si::Length>		input_measured_altitude_amsl		{ this, "/measurements/altitude/amsl" };
	x2::PropertyIn<si::Velocity>	input_measured_vs					{ this, "/measurements/vertical-speed" };
	x2::PropertyIn<si::Angle>		input_measured_fpa					{ this, "/measurements/flight-path-angle" };

	/*
	 * Input - airplane configuration
	 */

	x2::PropertyIn<si::Force>		input_thr_ref_for_toga				{ this, "/configuration/thrust-reference-for-toga" };
	x2::PropertyIn<si::Force>		input_thr_ref_for_cont				{ this, "/configuration/thrust-reference-for-max-cont" };
	x2::PropertyIn<si::Force>		input_thr_ref_for_cruise			{ this, "/configuration/thrust-reference-for-cruise" };
	x2::PropertyIn<si::Force>		input_thr_ref_for_descent			{ this, "/configuration/thrust-reference-for-descent" };
	x2::PropertyIn<si::Velocity>	input_spd_ref_for_climbout			{ this, "/configuration/speed-reference-for-climbout" };
	x2::PropertyIn<si::Velocity>	input_spd_ref_for_cruise			{ this, "/configuration/speed-reference-for-cruise" };
	x2::PropertyIn<si::Velocity>	input_spd_ref_for_approach			{ this, "/configuration/speed-reference-for-approach" };

	/*
	 * Input - buttons
	 */

	x2::PropertyIn<bool>			input_button_ap						{ this, "/buttons/ap" };
	x2::PropertyIn<bool>			input_button_at						{ this, "/buttons/at" };
	x2::PropertyIn<bool>			input_button_yd						{ this, "/buttons/yd" };
	x2::PropertyIn<bool>			input_button_xchg_ias_mach			{ this, "/buttons/xchg-ias-mach" };
	x2::PropertyIn<bool>			input_button_toga					{ this, "/buttons/toga" };
	x2::PropertyIn<bool>			input_button_spd_sel				{ this, "/buttons/spd-sel" };
	x2::PropertyIn<bool>			input_button_spd_hold				{ this, "/buttons/spd-hold" };
	x2::PropertyIn<bool>			input_button_xchg_heading_step		{ this, "/buttons/xchg-heading-step" };
	x2::PropertyIn<bool>			input_button_xchg_hdg_trk			{ this, "/buttons/xchg-hdg-trk" };
	x2::PropertyIn<bool>			input_button_hdgtrk_sel				{ this, "/buttons/hdgtrk-sel" };
	x2::PropertyIn<bool>			input_button_hdgtrk_hold			{ this, "/buttons/hdgtrk-hold" };
	x2::PropertyIn<bool>			input_button_wng_lvl				{ this, "/buttons/wng-lvl" };
	x2::PropertyIn<bool>			input_button_loc					{ this, "/buttons/loc" };
	x2::PropertyIn<bool>			input_button_lnav					{ this, "/buttons/lnav" };
	x2::PropertyIn<bool>			input_button_vnav					{ this, "/buttons/vnav" };
	x2::PropertyIn<bool>			input_button_lvl_all				{ this, "/buttons/lvl-all" };
	x2::PropertyIn<bool>			input_button_to						{ this, "/buttons/to" };
	x2::PropertyIn<bool>			input_button_crz					{ this, "/buttons/crz" };
	x2::PropertyIn<bool>			input_button_app					{ this, "/buttons/app" };
	x2::PropertyIn<bool>			input_button_ils					{ this, "/buttons/ils" };
	x2::PropertyIn<bool>			input_button_xchg_altitude_step		{ this, "/buttons/xchg-altitude-step" };
	x2::PropertyIn<bool>			input_button_flch					{ this, "/buttons/flch" };
	x2::PropertyIn<bool>			input_button_altitude_hold			{ this, "/buttons/altitude-hold" };
	x2::PropertyIn<bool>			input_button_gs						{ this, "/buttons/gs" };
	x2::PropertyIn<bool>			input_button_xchg_vs_fpa			{ this, "/buttons/xchg-vs-fpa" };
	x2::PropertyIn<bool>			input_button_vertical_enable		{ this, "/buttons/vertical-enable" };
	x2::PropertyIn<bool>			input_button_vertical_sel			{ this, "/buttons/vertical-sel" };
	x2::PropertyIn<bool>			input_button_clb_con				{ this, "/buttons/clb-con" };

	/*
	 * Input - knobs
	 */

	x2::PropertyIn<int64_t>			input_knob_speed					{ this, "/knobs/speed" };
	x2::PropertyIn<int64_t>			input_knob_heading					{ this, "/knobs/heading" };
	x2::PropertyIn<int64_t>			input_knob_altitude					{ this, "/knobs/altitude" };
	x2::PropertyIn<int64_t>			input_knob_vertical					{ this, "/knobs/vertical" };

	/*
	 * Output - displays and LEDs
	 */

	x2::PropertyOut<double>			output_mcp_speed_display			{ this, "/mcp/speed-display" };
	x2::PropertyOut<double>			output_mcp_heading_display			{ this, "/mcp/heading-display" };
	x2::PropertyOut<double>			output_mcp_altitude_display			{ this, "/mcp/altitude-display" };
	x2::PropertyOut<double>			output_mcp_vertical_display			{ this, "/mcp/vertical-display" };
	x2::PropertyOut<std::string>	output_mcp_speed_format_out			{ this, "/mcp/speed-format" };		// String format for speed display on MCP.
	x2::PropertyOut<std::string>	output_mcp_heading_format_out		{ this, "/mcp/heading-format" };	// String format for heading display on MCP.
	x2::PropertyOut<std::string>	output_mcp_altitude_format_out		{ this, "/mcp/altitude-format" };	// String format for altitude display on MCP.
	x2::PropertyOut<std::string>	output_mcp_vertical_format_out		{ this, "/mcp/vertical-format" };	// String format for vertical speed display on MCP.
	x2::PropertyOut<bool>			output_mcp_led_ap					{ this, "/mcp/ap-led" };
	x2::PropertyOut<bool>			output_mcp_led_at					{ this, "/mcp/at-led" };
	x2::PropertyOut<bool>			output_mcp_led_yd					{ this, "/mcp/yd-led" };

	/*
	 * Output - settings forwarded to Flight Director (might be different than MCP settings)
	 */

	x2::PropertyOut<int64_t>		output_cmd_thrust_mode				{ this, "/cmd/thrust-mode" };
	x2::PropertyOut<int64_t>		output_cmd_roll_mode				{ this, "/cmd/roll-mode" };
	x2::PropertyOut<int64_t>		output_cmd_pitch_mode				{ this, "/cmd/pitch-mode" };
	x2::PropertyOut<si::Velocity>	output_cmd_ias						{ this, "/cmd/ias" };
	x2::PropertyOut<double>			output_cmd_mach						{ this, "/cmd/mach" };
	x2::PropertyOut<si::Angle>		output_cmd_heading_magnetic			{ this, "/cmd/heading-magnetic" };
	x2::PropertyOut<si::Angle>		output_cmd_track_magnetic			{ this, "/cmd/track-magnetic" };
	x2::PropertyOut<si::Length>		output_cmd_altitude					{ this, "/cmd/altitude" };
	x2::PropertyOut<si::Velocity>	output_cmd_vs						{ this, "/cmd/vs" };
	x2::PropertyOut<si::Angle>		output_cmd_fpa						{ this, "/cmd/fpa" };

	/*
	 * Output - EFIS bugs
	 */

	x2::PropertyOut<si::Force>		output_thr_ref						{ this, "/bugs/thr-ref" };
	x2::PropertyOut<si::Velocity>	output_spd_ref						{ this, "/bugs/spd-ref" };
	x2::PropertyOut<bool>			output_cmd_use_trk					{ this, "/bugs/use-trk" };

	/*
	 * Output - FMA strings
	 */

	x2::PropertyOut<std::string>	output_fma_hint						{ this, "/fma/hint" };
	x2::PropertyOut<std::string>	output_fma_speed_hint				{ this, "/fma/speed-hint" };
	x2::PropertyOut<std::string>	output_fma_roll_hint				{ this, "/fma/roll-hint" };
	x2::PropertyOut<std::string>	output_fma_roll_armed_hint			{ this, "/fma/roll-armed-hint" };
	x2::PropertyOut<std::string>	output_fma_pitch_hint				{ this, "/fma/pitch-hint" };
	x2::PropertyOut<std::string>	output_fma_pitch_armed_hint			{ this, "/fma/pitch-armed-hint" };

  private:
	static constexpr xf::Range<si::Velocity>	kSpeedRange		{ 10_kt, 300_kt };
	static constexpr xf::Range<double>			kMachRange		{ 0.000, 1.000 };
	static constexpr double						kMachStep		{ 0.001 };
	static constexpr xf::Range<si::Length>		kAltitudeRange	{ -5000_ft, 50'000_ft };
	static constexpr si::Velocity				kVSStep			{ 10_fpm };
	static constexpr xf::Range<si::Velocity>	kVSRange		{ -8'000_fpm, +8'000_fpm };
	static constexpr Angle						kFPAStep		{ 0.1_deg };
	static constexpr xf::Range<si::Angle>		kFPARange		{ -10_deg, +10_deg };

	// TODO ensure those classes are used
	class DisengageAP: public std::runtime_error
	{
	  public:
		using runtime_error::runtime_error;
	};

	class DisengageAT: public std::runtime_error
	{
	  public:
		using runtime_error::runtime_error;
	};

	class Disengage: public std::runtime_error
	{
	  public:
		using runtime_error::runtime_error;
	};

	enum class SpeedControl
	{
		KIAS,
		Mach,
	};

	enum class LateralControl
	{
		Heading,
		Track,
	};

	enum class VerticalControl
	{
		VS,
		FPA,
	};

	enum class AltitudeStep
	{
		Ft10,
		Ft100,
	};

	enum class HeadingStep
	{
		Deg1,
		Deg10,
	};

	enum class ThrustMode
	{
		None			= 0,
		TO_GA			= 1,
		CONT			= 2,
		IDLE			= 3,
		MCP_SPD			= 4,	// Displayed as "SPD SEL" or "SPD" on FMA.
		SPD_HOLD		= 5,
	};

	enum class RollMode
	{
		None			= 0,
		MCP				= 1,	// Displayed as "HDG SEL", "TRK SEL", "HDG" or "TRK" on FMA.
		HOLD			= 2,	// Displayed as "HDG HOLD" or "TRK HOLD" on FMA.
		WNG_LVL			= 3,
		LOC				= 4,
		LNAV			= 5,
	};

	enum class PitchMode
	{
		None			= 0,
		MCP_SPD			= 1,
		ALT_HOLD		= 2,	// Use alt_hold() instead of manually assigning to _pitch_mode.
		MCP_ALT			= 3,	// Displayed as "ALT" or "FLCH".
		VC				= 4,	// Vertical Control, displayed as "V/S" or "FPA" on FMA.
		VNAV_PTH		= 5,
		GS				= 6,
		FLARE			= 7,
	};

  public:
	// Ctor
	explicit
	AFCS (std::string const& instance = {});

  protected:
	// Module API
	void
	process (x2::Cycle const&) override;

  private:
	void
	button_press_ap();

	void
	button_press_at();

	void
	button_press_yd();

	/*
	 * Speed/thrust panel
	 *
	 * NOTE On each thrust mode change, pitch mode must be adjusted
	 * so that one of these control airspeed.
	 */

	void
	knob_speed_change (int delta);

	void
	button_press_xchg_ias_mach();

	void
	button_press_toga();

	void
	button_press_spd_hold();

	void
	button_press_spd_sel();

	/*
	 * Heading panel
	 */

	void
	knob_heading_change (int delta);

	void
	button_press_xchg_heading_step();

	void
	button_press_xchg_hdg_trk();

	void
	button_press_hdgtrk_sel();

	void
	button_press_hdgtrk_hold();

	void
	button_press_wng_lvl();

	void
	button_press_loc();

	/*
	 * Misc panel
	 *
	 * NOTE On each pitch mode change, thrust mode must be adjusted
	 * so that one of these control airspeed.
	 */

	void
	button_press_lnav();

	void
	button_press_vnav();

	void
	button_press_lvl_all();

	void
	button_press_to();

	void
	button_press_crz();

	void
	button_press_app();

	void
	button_press_ils();

	/*
	 * Altitude panel
	 */

	void
	knob_altitude_change (int delta);

	void
	button_press_xchg_altitude_step();

	void
	button_press_flch();

	void
	button_press_altitude_hold();

	void
	button_press_gs();

	/*
	 * Vertical speed panel
	 */

	void
	knob_vertical_change (int delta);

	void
	button_press_xchg_vs_fpa();

	void
	button_press_vertical_enable();

	void
	button_press_vertical_sel();

	void
	button_press_clb_con();

	/**
	 * Check input values and throws disconnect instruction if needed.
	 */
	void
	check_input();

	/**
	 * Check current measured values and possibly update state.
	 * Eg. change pitch mode to MCP ALT after FLCH if set altitude is acquired.
	 */
	void
	check_events();

	/**
	 * Compute and solve settings of Flight Director.
	 */
	void
	solve();

	/**
	 * Update LEDs and counters on MCP.
	 */
	void
	update_mcp();

	/**
	 * Update FMA messages.
	 */
	void
	update_efis();

	/**
	 * Update output cmd_* and *_ref properties.
	 */
	void
	update_output();

	/**
	 * Disengage A/P.
	 * From process() it's better to throw DisengageAP.
	 */
	void
	disengage_ap (const char* reason);

	/**
	 * Disengage A/T.
	 * From process() it's better to throw DisengageAT.
	 */
	void
	disengage_at (const char* reason);

	/**
	 * Enable SPD HOLD mode for thrust and assign current speed setting to thrust.
	 * If not possible to set the speed setting, signal error, but keep the mode.
	 */
	void
	spd_hold_with_thrust();

	/**
	 * Enable HDG|TRK HOLD mode for roll and assign current heading or track
	 * to cmd heading/track. If not possible to set the setting, signal error,
	 * but keep the mode.
	 */
	void
	heading_hold_with_roll();

	/**
	 * Enable ALT HOLD mode for pitch and assign current altitude setting to cmd.
	 * If not possible to set the altitude setting, signal error, but keep the mode.
	 */
	void
	alt_hold_with_pitch();

	/**
	 * Exchange two pitch modes, if one if them is active.
	 */
	void
	xchg_modes (PitchMode, PitchMode);

	/**
	 * Return current V/S rounded to _vs_rounding.
	 */
	Optional<si::Velocity>
	current_rounded_vs() const;

	/**
	 * Return current FPA rounded to _fpa_rounding.
	 */
	Optional<si::Angle>
	current_rounded_fpa() const;

	/**
	 * Return true if pitch is in any of VNAV modes.
	 */
	bool
	vnav_enabled() const;

	/**
	 * Return true if pitch controls airspeed.
	 */
	bool
	pitch_controls_airspeed() const;

	/**
	 * Makes pitch control airspeed.
	 */
	void
	transfer_airspeed_control_from_thrust_to_pitch();

	/**
	 * Makes thrust control airspeed.
	 */
	void
	transfer_airspeed_control_from_pitch_to_thrust();

	/**
	 * Create and save a button action for button press.
	 * Wrap button press callback and add call to solve() at the end.
	 */
	void
	make_button_action (x2::PropertyIn<bool>&, void (AFCS::* callback)());

	/**
	 * Create and save a knob action for knob movement.
	 * Wrap knob rotate callback and add call to solve() at the end.
	 */
	void
	make_knob_action (x2::PropertyIn<int64_t>&, void (AFCS::* callback)(int));

	/**
	 * Return string ID for a thrust mode.
	 */
	Optional<afcs_api::ThrustMode>
	translate_thrust_mode() const;

	/**
	 * Return string ID for a roll mode.
	 */
	Optional<afcs_api::RollMode>
	translate_roll_mode() const;

	/**
	 * Return string ID for a pitch mode.
	 */
	Optional<afcs_api::PitchMode>
	translate_pitch_mode() const;

	/**
	 * Cast internal Optional types.
	 */
	template<class Target, class Source>
		static Optional<Target>
		optional_cast (Optional<Source> const& source);

  private:
	bool									_ap_on				{ false };
	bool									_at_on				{ false };
	bool									_yd_on				{ false };
	ThrustMode								_thrust_mode		{ ThrustMode::None };
	RollMode								_roll_mode			{ RollMode::None };
	RollMode								_armed_roll_mode	{ RollMode::None };
	PitchMode								_pitch_mode			{ PitchMode::None };
	PitchMode								_armed_pitch_mode	{ PitchMode::None };
	SpeedControl							_speed_control		{ SpeedControl::KIAS };
	LateralControl							_lateral_control	{ LateralControl::Track };
	VerticalControl							_vertical_control	{ VerticalControl::VS };
	HeadingStep								_heading_step		{ HeadingStep::Deg1 };
	AltitudeStep							_altitude_step		{ AltitudeStep::Ft10 };
	// *_mcp are settings to be displayed on MCP:
	// TODO allow setting default values for _mcp_ias, _mcp_mach, _mcp_heading|track, _mcp_altitude
	si::Velocity							_mcp_ias			{ kSpeedRange.min() };
	double									_mcp_mach			{ 0.0 };
	si::Angle								_mcp_heading		{ 0_deg };
	si::Angle								_mcp_track			{ 0_deg };
	si::Length								_mcp_altitude		{ 1000_ft };
	Optional<si::Velocity>					_mcp_vs;
	Optional<si::Angle>						_mcp_fpa;
	std::set<Unique<x2::CallbackAction>>	_button_actions;
	std::set<Unique<x2::DeltaDecoder>>		_rotary_decoders;
};

#endif

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
#include <xefis/core/module.h>
#include <xefis/core/property.h>
#include <xefis/core/setting.h>
#include <xefis/utility/callback_actions.h>
#include <xefis/utility/delta_decoder.h>
#include <xefis/utility/range.h>

// Local:
#include "afcs_api.h"


class AFCS_IO: public xf::ModuleIO
{
  public:
	/*
	 * Settings
	 */

	xf::Setting<si::Velocity>		acq_delta_ias				{ this, "acq_delta_ias", 2_kt };
	xf::Setting<double>				acq_delta_mach				{ this, "acq_delta_mach", 0.0033 };
	xf::Setting<si::Angle>			acq_delta_heading			{ this, "acq_delta_heading", 2_deg };
	xf::Setting<si::Length>			acq_delta_altitude			{ this, "acq_delta_altitude", 100_ft };
	xf::Setting<si::Velocity>		vs_rounding					{ this, "vs_rounding", 100_fpm };
	xf::Setting<si::Angle>			fpa_rounding				{ this, "fpa_rounding", 0.1_deg };
	xf::Setting<std::string>		mcp_speed_format_kias		{ this, "mcp_speed_format_kias", "%d" };
	xf::Setting<std::string>		mcp_speed_format_mach		{ this, "mcp_speed_format_mach", "%.3f" };
	xf::Setting<std::string>		mcp_heading_format			{ this, "mcp_heading_format", "%03d" };
	xf::Setting<std::string>		mcp_altitude_format			{ this, "mcp_altitude_format", "%d" };
	xf::Setting<std::string>		mcp_vertical_format_vs		{ this, "mcp_vertical_format_vs", "%+d" };
	xf::Setting<std::string>		mcp_vertical_format_fpa		{ this, "mcp_vertical_format_fpa", "%.1f" };
	xf::Setting<si::Velocity>		default_ias					{ this, "default_ias" };
	xf::Setting<double>				default_mach				{ this, "default_mach" };

	/*
	 * Input - measurements
	 */

	xf::PropertyIn<si::Velocity>	measured_ias				{ this, "measurements/ias" };
	xf::PropertyIn<double>			measured_mach				{ this, "measurements/mach" };
	xf::PropertyIn<si::Angle>		measured_heading_magnetic	{ this, "measurements/heading/magnetic" };
	xf::PropertyIn<si::Angle>		measured_track_magnetic		{ this, "measurements/track/magnetic" };
	xf::PropertyIn<si::Length>		measured_altitude_amsl		{ this, "measurements/altitude/amsl" };
	xf::PropertyIn<si::Velocity>	measured_vs					{ this, "measurements/vertical-speed" };
	xf::PropertyIn<si::Angle>		measured_fpa				{ this, "measurements/flight-path-angle" };

	/*
	 * Input - airplane configuration
	 */

	xf::PropertyIn<si::Force>		thr_ref_for_toga			{ this, "configuration/thrust-reference-for-toga" };
	xf::PropertyIn<si::Force>		thr_ref_for_cont			{ this, "configuration/thrust-reference-for-max-cont" };
	xf::PropertyIn<si::Force>		thr_ref_for_cruise			{ this, "configuration/thrust-reference-for-cruise" };
	xf::PropertyIn<si::Force>		thr_ref_for_descent			{ this, "configuration/thrust-reference-for-descent" };
	xf::PropertyIn<si::Velocity>	spd_ref_for_climbout		{ this, "configuration/speed-reference-for-climbout" };
	xf::PropertyIn<si::Velocity>	spd_ref_for_cruise			{ this, "configuration/speed-reference-for-cruise" };
	xf::PropertyIn<si::Velocity>	spd_ref_for_approach		{ this, "configuration/speed-reference-for-approach" };

	/*
	 * Input - buttons
	 */

	xf::PropertyIn<bool>			button_ap					{ this, "buttons/ap" };
	xf::PropertyIn<bool>			button_at					{ this, "buttons/at" };
	xf::PropertyIn<bool>			button_yd					{ this, "buttons/yd" };
	xf::PropertyIn<bool>			button_xchg_ias_mach		{ this, "buttons/xchg-ias-mach" };
	xf::PropertyIn<bool>			button_toga					{ this, "buttons/toga" };
	xf::PropertyIn<bool>			button_spd_sel				{ this, "buttons/spd-sel" };
	xf::PropertyIn<bool>			button_spd_hold				{ this, "buttons/spd-hold" };
	xf::PropertyIn<bool>			button_xchg_heading_step	{ this, "buttons/xchg-heading-step" };
	xf::PropertyIn<bool>			button_xchg_hdg_trk			{ this, "buttons/xchg-hdg-trk" };
	xf::PropertyIn<bool>			button_hdgtrk_sel			{ this, "buttons/hdgtrk-sel" };
	xf::PropertyIn<bool>			button_hdgtrk_hold			{ this, "buttons/hdgtrk-hold" };
	xf::PropertyIn<bool>			button_wng_lvl				{ this, "buttons/wng-lvl" };
	xf::PropertyIn<bool>			button_loc					{ this, "buttons/loc" };
	xf::PropertyIn<bool>			button_lnav					{ this, "buttons/lnav" };
	xf::PropertyIn<bool>			button_vnav					{ this, "buttons/vnav" };
	xf::PropertyIn<bool>			button_lvl_all				{ this, "buttons/lvl-all" };
	xf::PropertyIn<bool>			button_to					{ this, "buttons/to" };
	xf::PropertyIn<bool>			button_crz					{ this, "buttons/crz" };
	xf::PropertyIn<bool>			button_app					{ this, "buttons/app" };
	xf::PropertyIn<bool>			button_ils					{ this, "buttons/ils" };
	xf::PropertyIn<bool>			button_xchg_altitude_step	{ this, "buttons/xchg-altitude-step" };
	xf::PropertyIn<bool>			button_flch					{ this, "buttons/flch" };
	xf::PropertyIn<bool>			button_altitude_hold		{ this, "buttons/altitude-hold" };
	xf::PropertyIn<bool>			button_gs					{ this, "buttons/gs" };
	xf::PropertyIn<bool>			button_xchg_vs_fpa			{ this, "buttons/xchg-vs-fpa" };
	xf::PropertyIn<bool>			button_vertical_enable		{ this, "buttons/vertical-enable" };
	xf::PropertyIn<bool>			button_vertical_sel			{ this, "buttons/vertical-sel" };
	xf::PropertyIn<bool>			button_clb_con				{ this, "buttons/clb-con" };

	/*
	 * Input - knobs
	 */

	xf::PropertyIn<int64_t>			knob_speed					{ this, "knobs/speed" };
	xf::PropertyIn<int64_t>			knob_heading				{ this, "knobs/heading" };
	xf::PropertyIn<int64_t>			knob_altitude				{ this, "knobs/altitude" };
	xf::PropertyIn<int64_t>			knob_vertical				{ this, "knobs/vertical" };

	/*
	 * Output - displays and LEDs
	 */

	xf::PropertyOut<double>			mcp_speed_display			{ this, "mcp/speed-display" };
	xf::PropertyOut<double>			mcp_heading_display			{ this, "mcp/heading-display" };
	xf::PropertyOut<double>			mcp_altitude_display		{ this, "mcp/altitude-display" };
	xf::PropertyOut<double>			mcp_vertical_display		{ this, "mcp/vertical-display" };
	xf::PropertyOut<std::string>	mcp_speed_format_out		{ this, "mcp/speed-format" };		// String format for speed display on MCP.
	xf::PropertyOut<std::string>	mcp_heading_format_out		{ this, "mcp/heading-format" };		// String format for heading display on MCP.
	xf::PropertyOut<std::string>	mcp_altitude_format_out		{ this, "mcp/altitude-format" };	// String format for altitude display on MCP.
	xf::PropertyOut<std::string>	mcp_vertical_format_out		{ this, "mcp/vertical-format" };	// String format for vertical speed display on MCP.
	xf::PropertyOut<bool>			mcp_led_ap					{ this, "mcp/ap-led" };
	xf::PropertyOut<bool>			mcp_led_at					{ this, "mcp/at-led" };
	xf::PropertyOut<bool>			mcp_led_yd					{ this, "mcp/yd-led" };

	/*
	 * Output - settings forwarded to Flight Director (might be different than MCP settings)
	 */

	xf::PropertyOut<int64_t>		cmd_thrust_mode				{ this, "cmd/thrust-mode" };
	xf::PropertyOut<int64_t>		cmd_roll_mode				{ this, "cmd/roll-mode" };
	xf::PropertyOut<int64_t>		cmd_pitch_mode				{ this, "cmd/pitch-mode" };
	xf::PropertyOut<si::Velocity>	cmd_ias						{ this, "cmd/ias" };
	xf::PropertyOut<double>			cmd_mach					{ this, "cmd/mach" };
	xf::PropertyOut<si::Angle>		cmd_heading_magnetic		{ this, "cmd/heading-magnetic" };
	xf::PropertyOut<si::Angle>		cmd_track_magnetic			{ this, "cmd/track-magnetic" };
	xf::PropertyOut<si::Length>		cmd_altitude				{ this, "cmd/altitude" };
	xf::PropertyOut<si::Velocity>	cmd_vs						{ this, "cmd/vs" };
	xf::PropertyOut<si::Angle>		cmd_fpa						{ this, "cmd/fpa" };

	/*
	 * Output - EFIS bugs
	 */

	xf::PropertyOut<si::Force>		thr_ref						{ this, "bugs/thr-ref" };
	xf::PropertyOut<si::Velocity>	spd_ref						{ this, "bugs/spd-ref" };
	xf::PropertyOut<bool>			cmd_use_trk					{ this, "bugs/use-trk" };

	/*
	 * Output - FMA strings
	 */

	xf::PropertyOut<std::string>	fma_hint					{ this, "fma/hint" };
	xf::PropertyOut<std::string>	fma_speed_hint				{ this, "fma/speed-hint" };
	xf::PropertyOut<std::string>	fma_roll_hint				{ this, "fma/roll-hint" };
	xf::PropertyOut<std::string>	fma_roll_armed_hint			{ this, "fma/roll-armed-hint" };
	xf::PropertyOut<std::string>	fma_pitch_hint				{ this, "fma/pitch-hint" };
	xf::PropertyOut<std::string>	fma_pitch_armed_hint		{ this, "fma/pitch-armed-hint" };
};


/**
 * Controls AFCS logic. Gets input from Mode Control Panel,
 * makes outputs for displays, LEDs, annunciators, also for commanded values (altitude, speed, etc).
 */
class AFCS: public xf::Module<AFCS_IO>
{
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
	AFCS (std::unique_ptr<AFCS_IO>, std::string_view const& instance = {});

  protected:
	// Module API
	void
	process (xf::Cycle const&) override;

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
	std::optional<si::Velocity>
	current_rounded_vs() const;

	/**
	 * Return current FPA rounded to _fpa_rounding.
	 */
	std::optional<si::Angle>
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
	make_button_action (xf::PropertyIn<bool>&, void (AFCS::* callback)());

	/**
	 * Create and save a knob action for knob movement.
	 * Wrap knob rotate callback and add call to solve() at the end.
	 */
	void
	make_knob_action (xf::PropertyIn<int64_t>&, void (AFCS::* callback)(int));

	/**
	 * Return string ID for a thrust mode.
	 */
	std::optional<afcs::ThrustMode>
	translate_thrust_mode() const;

	/**
	 * Return string ID for a roll mode.
	 */
	std::optional<afcs::RollMode>
	translate_roll_mode() const;

	/**
	 * Return string ID for a pitch mode.
	 */
	std::optional<afcs::PitchMode>
	translate_pitch_mode() const;

	/**
	 * Cast internal std::optional types.
	 */
	template<class Target, class Source>
		static std::optional<Target>
		optional_cast (std::optional<Source> const& source);

  private:
	bool											_ap_on				{ false };
	bool											_at_on				{ false };
	bool											_yd_on				{ false };
	ThrustMode										_thrust_mode		{ ThrustMode::None };
	RollMode										_roll_mode			{ RollMode::None };
	RollMode										_armed_roll_mode	{ RollMode::None };
	PitchMode										_pitch_mode			{ PitchMode::None };
	PitchMode										_armed_pitch_mode	{ PitchMode::None };
	SpeedControl									_speed_control		{ SpeedControl::KIAS };
	LateralControl									_lateral_control	{ LateralControl::Track };
	VerticalControl									_vertical_control	{ VerticalControl::VS };
	HeadingStep										_heading_step		{ HeadingStep::Deg1 };
	AltitudeStep									_altitude_step		{ AltitudeStep::Ft10 };
	// *_mcp are settings to be displayed on MCP:
	// TODO allow setting default values for _mcp_ias, _mcp_mach, _mcp_heading|track, _mcp_altitude
	si::Velocity									_mcp_ias			{ kSpeedRange.min() };
	double											_mcp_mach			{ 0.0 };
	si::Angle										_mcp_heading		{ 0_deg };
	si::Angle										_mcp_track			{ 0_deg };
	si::Length										_mcp_altitude		{ 1000_ft };
	std::optional<si::Velocity>						_mcp_vs;
	std::optional<si::Angle>						_mcp_fpa;
	std::set<std::unique_ptr<xf::CallbackAction>>	_button_actions;
	std::set<std::unique_ptr<xf::DeltaDecoder<>>>	_rotary_decoders;
};

#endif

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

// Local:
#include "afcs_api.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/setting.h>
#include <xefis/core/sockets/module_socket.h>
#include <xefis/support/sockets/socket_delta_decoder.h>
#include <xefis/support/sockets/socket_value_changed_action.h>

// Neutrino:
#include <neutrino/range.h>

// Standard:
#include <cstddef>
#include <set>
#include <string>


namespace si = neutrino::si;
using namespace neutrino::si::literals;


class AFCS_IO: public xf::Module
{
  public:
	/*
	 * Settings
	 */

	xf::Setting<si::Velocity>	acq_delta_ias				{ this, "acq_delta_ias", 2_kt };
	xf::Setting<double>			acq_delta_mach				{ this, "acq_delta_mach", 0.0033 };
	xf::Setting<si::Angle>		acq_delta_heading			{ this, "acq_delta_heading", 2_deg };
	xf::Setting<si::Length>		acq_delta_altitude			{ this, "acq_delta_altitude", 100_ft };
	xf::Setting<si::Velocity>	vs_rounding					{ this, "vs_rounding", 100_fpm };
	xf::Setting<si::Angle>		fpa_rounding				{ this, "fpa_rounding", 0.1_deg };
	xf::Setting<std::string>	mcp_speed_format_kias		{ this, "mcp_speed_format_kias", "%d" };
	xf::Setting<std::string>	mcp_speed_format_mach		{ this, "mcp_speed_format_mach", "%.3f" };
	xf::Setting<std::string>	mcp_heading_format			{ this, "mcp_heading_format", "%03d" };
	xf::Setting<std::string>	mcp_altitude_format			{ this, "mcp_altitude_format", "%d" };
	xf::Setting<std::string>	mcp_vertical_format_vs		{ this, "mcp_vertical_format_vs", "%+d" };
	xf::Setting<std::string>	mcp_vertical_format_fpa		{ this, "mcp_vertical_format_fpa", "%.1f" };
	xf::Setting<si::Velocity>	default_ias					{ this, "default_ias" };
	xf::Setting<double>			default_mach				{ this, "default_mach" };

	/*
	 * Input - measurements
	 */

	xf::ModuleIn<si::Velocity>	measured_ias				{ this, "measurements/ias" };
	xf::ModuleIn<double>		measured_mach				{ this, "measurements/mach" };
	xf::ModuleIn<si::Angle>		measured_heading_magnetic	{ this, "measurements/heading/magnetic" };
	xf::ModuleIn<si::Angle>		measured_track_magnetic		{ this, "measurements/track/magnetic" };
	xf::ModuleIn<si::Length>	measured_altitude_amsl		{ this, "measurements/altitude/amsl" };
	xf::ModuleIn<si::Velocity>	measured_vs					{ this, "measurements/vertical-speed" };
	xf::ModuleIn<si::Angle>		measured_fpa				{ this, "measurements/flight-path-angle" };

	/*
	 * Input - airplane configuration
	 */

	xf::ModuleIn<si::Force>		thr_ref_for_toga			{ this, "configuration/thrust-reference-for-toga" };
	xf::ModuleIn<si::Force>		thr_ref_for_cont			{ this, "configuration/thrust-reference-for-max-cont" };
	xf::ModuleIn<si::Force>		thr_ref_for_cruise			{ this, "configuration/thrust-reference-for-cruise" };
	xf::ModuleIn<si::Force>		thr_ref_for_descent			{ this, "configuration/thrust-reference-for-descent" };
	xf::ModuleIn<si::Velocity>	spd_ref_for_climbout		{ this, "configuration/speed-reference-for-climbout" };
	xf::ModuleIn<si::Velocity>	spd_ref_for_cruise			{ this, "configuration/speed-reference-for-cruise" };
	xf::ModuleIn<si::Velocity>	spd_ref_for_approach		{ this, "configuration/speed-reference-for-approach" };

	/*
	 * Input - buttons
	 */

	xf::ModuleIn<bool>			button_ap					{ this, "buttons/ap" };
	xf::ModuleIn<bool>			button_at					{ this, "buttons/at" };
	xf::ModuleIn<bool>			button_yd					{ this, "buttons/yd" };
	xf::ModuleIn<bool>			button_xchg_ias_mach		{ this, "buttons/xchg-ias-mach" };
	xf::ModuleIn<bool>			button_toga					{ this, "buttons/toga" };
	xf::ModuleIn<bool>			button_spd_sel				{ this, "buttons/spd-sel" };
	xf::ModuleIn<bool>			button_spd_hold				{ this, "buttons/spd-hold" };
	xf::ModuleIn<bool>			button_xchg_heading_step	{ this, "buttons/xchg-heading-step" };
	xf::ModuleIn<bool>			button_xchg_hdg_trk			{ this, "buttons/xchg-hdg-trk" };
	xf::ModuleIn<bool>			button_hdgtrk_sel			{ this, "buttons/hdgtrk-sel" };
	xf::ModuleIn<bool>			button_hdgtrk_hold			{ this, "buttons/hdgtrk-hold" };
	xf::ModuleIn<bool>			button_wng_lvl				{ this, "buttons/wng-lvl" };
	xf::ModuleIn<bool>			button_loc					{ this, "buttons/loc" };
	xf::ModuleIn<bool>			button_lnav					{ this, "buttons/lnav" };
	xf::ModuleIn<bool>			button_vnav					{ this, "buttons/vnav" };
	xf::ModuleIn<bool>			button_lvl_all				{ this, "buttons/lvl-all" };
	xf::ModuleIn<bool>			button_to					{ this, "buttons/to" };
	xf::ModuleIn<bool>			button_crz					{ this, "buttons/crz" };
	xf::ModuleIn<bool>			button_app					{ this, "buttons/app" };
	xf::ModuleIn<bool>			button_ils					{ this, "buttons/ils" };
	xf::ModuleIn<bool>			button_xchg_altitude_step	{ this, "buttons/xchg-altitude-step" };
	xf::ModuleIn<bool>			button_flch					{ this, "buttons/flch" };
	xf::ModuleIn<bool>			button_altitude_hold		{ this, "buttons/altitude-hold" };
	xf::ModuleIn<bool>			button_gs					{ this, "buttons/gs" };
	xf::ModuleIn<bool>			button_xchg_vs_fpa			{ this, "buttons/xchg-vs-fpa" };
	xf::ModuleIn<bool>			button_vertical_enable		{ this, "buttons/vertical-enable" };
	xf::ModuleIn<bool>			button_vertical_sel			{ this, "buttons/vertical-sel" };
	xf::ModuleIn<bool>			button_clb_con				{ this, "buttons/clb-con" };

	/*
	 * Input - knobs
	 */

	xf::ModuleIn<int64_t>		knob_speed					{ this, "knobs/speed" };
	xf::ModuleIn<int64_t>		knob_heading				{ this, "knobs/heading" };
	xf::ModuleIn<int64_t>		knob_altitude				{ this, "knobs/altitude" };
	xf::ModuleIn<int64_t>		knob_vertical				{ this, "knobs/vertical" };

	/*
	 * Output - displays and LEDs
	 */

	xf::ModuleOut<double>		mcp_speed_display			{ this, "mcp/speed-display" };
	xf::ModuleOut<double>		mcp_heading_display			{ this, "mcp/heading-display" };
	xf::ModuleOut<double>		mcp_altitude_display		{ this, "mcp/altitude-display" };
	xf::ModuleOut<double>		mcp_vertical_display		{ this, "mcp/vertical-display" };
	xf::ModuleOut<std::string>	mcp_speed_format_out		{ this, "mcp/speed-format" };		// String format for speed display on MCP.
	xf::ModuleOut<std::string>	mcp_heading_format_out		{ this, "mcp/heading-format" };		// String format for heading display on MCP.
	xf::ModuleOut<std::string>	mcp_altitude_format_out		{ this, "mcp/altitude-format" };	// String format for altitude display on MCP.
	xf::ModuleOut<std::string>	mcp_vertical_format_out		{ this, "mcp/vertical-format" };	// String format for vertical speed display on MCP.
	xf::ModuleOut<bool>			mcp_led_ap					{ this, "mcp/ap-led" };
	xf::ModuleOut<bool>			mcp_led_at					{ this, "mcp/at-led" };
	xf::ModuleOut<bool>			mcp_led_yd					{ this, "mcp/yd-led" };

	/*
	 * Output - settings forwarded to Flight Director (might be different than MCP settings)
	 */

	xf::ModuleOut<int64_t>		cmd_thrust_mode				{ this, "cmd/thrust-mode" };
	xf::ModuleOut<int64_t>		cmd_roll_mode				{ this, "cmd/roll-mode" };
	xf::ModuleOut<int64_t>		cmd_pitch_mode				{ this, "cmd/pitch-mode" };
	xf::ModuleOut<si::Velocity>	cmd_ias						{ this, "cmd/ias" };
	xf::ModuleOut<double>		cmd_mach					{ this, "cmd/mach" };
	xf::ModuleOut<si::Angle>	cmd_heading_magnetic		{ this, "cmd/heading-magnetic" };
	xf::ModuleOut<si::Angle>	cmd_track_magnetic			{ this, "cmd/track-magnetic" };
	xf::ModuleOut<si::Length>	cmd_altitude				{ this, "cmd/altitude" };
	xf::ModuleOut<si::Velocity>	cmd_vs						{ this, "cmd/vs" };
	xf::ModuleOut<si::Angle>	cmd_fpa						{ this, "cmd/fpa" };

	/*
	 * Output - EFIS bugs
	 */

	xf::ModuleOut<si::Force>	thr_ref						{ this, "bugs/thr-ref" };
	xf::ModuleOut<si::Velocity>	spd_ref						{ this, "bugs/spd-ref" };
	xf::ModuleOut<bool>			cmd_use_trk					{ this, "bugs/use-trk" };

	/*
	 * Output - FMA strings
	 */

	xf::ModuleOut<std::string>	fma_hint					{ this, "fma/hint" };
	xf::ModuleOut<std::string>	fma_speed_hint				{ this, "fma/speed-hint" };
	xf::ModuleOut<std::string>	fma_roll_hint				{ this, "fma/roll-hint" };
	xf::ModuleOut<std::string>	fma_roll_armed_hint			{ this, "fma/roll-armed-hint" };
	xf::ModuleOut<std::string>	fma_pitch_hint				{ this, "fma/pitch-hint" };
	xf::ModuleOut<std::string>	fma_pitch_armed_hint		{ this, "fma/pitch-armed-hint" };

  public:
	using xf::Module::Module;
};


/**
 * Controls AFCS logic. Gets input from Mode Control Panel,
 * makes outputs for displays, LEDs, annunciators, also for commanded values (altitude, speed, etc).
 */
class AFCS: public AFCS_IO
{
  private:
	static constexpr xf::Range<si::Velocity>	kSpeedRange		{ 10_kt, 300_kt };
	static constexpr xf::Range<double>			kMachRange		{ 0.000, 1.000 };
	static constexpr double						kMachStep		{ 0.001 };
	static constexpr xf::Range<si::Length>		kAltitudeRange	{ -5000_ft, 50'000_ft };
	static constexpr si::Velocity				kVSStep			{ 10_fpm };
	static constexpr xf::Range<si::Velocity>	kVSRange		{ -8'000_fpm, +8'000_fpm };
	static constexpr si::Angle					kFPAStep		{ 0.1_deg };
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
	AFCS (std::string_view const& instance = {});

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
	 * Update output cmd_* and *_ref sockets.
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
	make_button_action (xf::ModuleIn<bool>&, void (AFCS::* callback)());

	/**
	 * Create and save a knob action for knob movement.
	 * Wrap knob rotate callback and add call to solve() at the end.
	 */
	void
	make_knob_action (xf::ModuleIn<int64_t>&, void (AFCS::* callback)(int));

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
	AFCS_IO&											_io					{ *this };
	bool												_ap_on				{ false };
	bool												_at_on				{ false };
	bool												_yd_on				{ false };
	ThrustMode											_thrust_mode		{ ThrustMode::None };
	RollMode											_roll_mode			{ RollMode::None };
	RollMode											_armed_roll_mode	{ RollMode::None };
	PitchMode											_pitch_mode			{ PitchMode::None };
	PitchMode											_armed_pitch_mode	{ PitchMode::None };
	SpeedControl										_speed_control		{ SpeedControl::KIAS };
	LateralControl										_lateral_control	{ LateralControl::Track };
	VerticalControl										_vertical_control	{ VerticalControl::VS };
	HeadingStep											_heading_step		{ HeadingStep::Deg1 };
	AltitudeStep										_altitude_step		{ AltitudeStep::Ft10 };
	// *_mcp are settings to be displayed on MCP:
	// TODO allow setting default values for _mcp_ias, _mcp_mach, _mcp_heading|track, _mcp_altitude
	si::Velocity										_mcp_ias			{ kSpeedRange.min() };
	double												_mcp_mach			{ 0.0 };
	si::Angle											_mcp_heading		{ 0_deg };
	si::Angle											_mcp_track			{ 0_deg };
	si::Length											_mcp_altitude		{ 1000_ft };
	std::optional<si::Velocity>							_mcp_vs;
	std::optional<si::Angle>							_mcp_fpa;
	std::set<std::unique_ptr<xf::SocketAction>>			_socket_actions;
	std::set<std::unique_ptr<xf::SocketDeltaDecoder<>>>	_rotary_decoders;
};

#endif

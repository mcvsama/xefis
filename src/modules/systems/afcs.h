/* vim:ts=4
 *
 * Copyleft 2012…2014  Michał Gawron
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

// Qt:
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/property.h>
#include <xefis/core/property_observer.h>
#include <xefis/utility/actions.h>
#include <xefis/utility/delta_decoder.h>
#include <xefis/utility/range.h>


class AFCS: public xf::Module
{
	static constexpr xf::Range<Speed>	SpeedRange				= { 10_kt, 300_kt };
	static constexpr xf::Range<double>	MachRange				= { 0.000, 1.000 };
	static constexpr double				MachStep				= 0.001;
	static constexpr xf::Range<Length>	AltitudeRange			= { -5000_ft, 50'000_ft };
	static constexpr Speed				VSStep					= 10_fpm;
	static constexpr xf::Range<Speed>	VSRange					= { -8'000_fpm, +8'000_fpm };
	static constexpr Angle				FPAStep					= 0.1_deg;
	static constexpr xf::Range<Angle>	FPARange				= { -10_deg, +10_deg };

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
		sentinel		= 6,
	};

	enum class RollMode
	{
		None			= 0,
		MCP				= 1,	// Displayed as "HDG SEL", "TRK SEL", "HDG" or "TRK" on FMA.
		HOLD			= 2,	// Displayed as "HDG HOLD" or "TRK HOLD" on FMA.
		WNG_LVL			= 3,
		LOC				= 4,
		LNAV			= 5,
		sentinel		= 6,
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
		sentinel		= 8,
	};

  public:
	// Ctor
	AFCS (xf::ModuleManager*, QDomElement const& config);

  protected:
	void
	data_updated() override;

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
	 * From data_updated() it's better to throw DisengageAP.
	 */
	void
	disengage_ap (const char* reason);

	/**
	 * Disengage A/T.
	 * From data_updated() it's better to throw DisengageAT.
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
	Optional<Speed>
	current_rounded_vs() const;

	/**
	 * Return current FPA rounded to _fpa_rounding.
	 */
	Optional<Angle>
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
	make_button_action (xf::PropertyBoolean, void (AFCS::* callback)());

	/**
	 * Create and save a knob action for knob movement.
	 * Wrap knob rotate callback and add call to solve() at the end.
	 */
	void
	make_knob_action (xf::PropertyInteger, void (AFCS::* callback)(int));

	/**
	 * Return string ID for a thrust mode.
	 */
	Optional<std::string>
	stringify_thrust_mode() const;

	/**
	 * Return string ID for a roll mode.
	 */
	Optional<std::string>
	stringify_roll_mode() const;

	/**
	 * Return string ID for a pitch mode.
	 */
	Optional<std::string>
	stringify_pitch_mode() const;

  private:
	// Settings:
	Speed								_acq_delta_ias				= 2_kt;
	xf::PropertyFloat::Type				_acq_delta_mach				= 0.0033;
	Angle								_acq_delta_heading			= 2_deg;
	Length								_acq_delta_altitude			= 100_ft;
	Speed								_vs_rounding				= 100_fpm;
	Angle								_fpa_rounding				= 0.1_deg;
	std::string							_mcp_speed_format_kias		= "%d";
	std::string							_mcp_speed_format_mach		= "%.3f";
	std::string							_mcp_heading_format			= "%03d";
	std::string							_mcp_altitude_format		= "%d";
	std::string							_mcp_vertical_format_vs		= "%+d";
	std::string							_mcp_vertical_format_fpa	= "%.1f";
	// State:
	bool								_ap_on						= false;
	bool								_at_on						= false;
	bool								_yd_on						= false;
	ThrustMode							_thrust_mode				= ThrustMode::None;
	RollMode							_roll_mode					= RollMode::None;
	RollMode							_armed_roll_mode			= RollMode::None;
	PitchMode							_pitch_mode					= PitchMode::None;
	PitchMode							_armed_pitch_mode			= PitchMode::None;
	SpeedControl						_speed_control				= SpeedControl::KIAS;
	LateralControl						_lateral_control			= LateralControl::Track;
	VerticalControl						_vertical_control			= VerticalControl::VS;
	HeadingStep							_heading_step				= HeadingStep::Deg1;
	AltitudeStep						_altitude_step				= AltitudeStep::Ft10;
	// Settings to be displayed on MCP:
	Speed								_mcp_ias					= SpeedRange.min();
	xf::PropertyFloat::Type				_mcp_mach					= 0.0;
	Angle								_mcp_heading				= 0_deg;
	Angle								_mcp_track					= 0_deg;
	Length								_mcp_altitude				= 1000_ft;
	Optional<Speed>						_mcp_vs;
	Optional<Angle>						_mcp_fpa;
	// Measurements:
	xf::PropertySpeed					_measured_ias;
	xf::PropertyFloat					_measured_mach;
	xf::PropertyAngle					_measured_heading;
	xf::PropertyAngle					_measured_track;
	xf::PropertyLength					_measured_altitude_amsl;
	xf::PropertySpeed					_measured_vs;
	xf::PropertyAngle					_measured_fpa;
	// Airplane configuration:
	xf::PropertyForce					_thr_ref_for_toga;
	xf::PropertyForce					_thr_ref_for_cont;//TODO use it
	xf::PropertyForce					_thr_ref_for_cruise;
	xf::PropertyForce					_thr_ref_for_descent;
	xf::PropertySpeed					_spd_ref_for_climbout;
	xf::PropertySpeed					_spd_ref_for_cruise;
	xf::PropertySpeed					_spd_ref_for_approach;
	// Props for displays and LEDs:
	xf::PropertyFloat					_mcp_speed_display;
	xf::PropertyFloat					_mcp_heading_display;
	xf::PropertyFloat					_mcp_altitude_display;
	xf::PropertyFloat					_mcp_vertical_display;
	xf::PropertyString					_mcp_speed_format_out;		// String format for speed display on MCP.
	xf::PropertyString					_mcp_heading_format_out;	// String format for heading display on MCP.
	xf::PropertyString					_mcp_altitude_format_out;	// String format for altitude display on MCP.
	xf::PropertyString					_mcp_vertical_format_out;	// String format for vertical speed display on MCP.
	xf::PropertyBoolean					_mcp_led_ap;
	xf::PropertyBoolean					_mcp_led_at;
	xf::PropertyBoolean					_mcp_led_yd;
	// Settings forwarded fo FD (might be different than MCP settings):
	xf::PropertyString					_cmd_thrust_mode;
	xf::PropertyString					_cmd_roll_mode;
	xf::PropertyString					_cmd_pitch_mode;
	xf::PropertySpeed					_cmd_ias;
	xf::PropertyFloat					_cmd_mach;
	xf::PropertyAngle					_cmd_heading;
	xf::PropertyAngle					_cmd_track;
	xf::PropertyLength					_cmd_altitude;
	xf::PropertySpeed					_cmd_vs;
	xf::PropertyAngle					_cmd_fpa;
	// Speed/thrust bugs for EFIS:
	xf::PropertyForce					_thr_ref;
	xf::PropertySpeed					_spd_ref;
	// Output for FMA:
	xf::PropertyString					_fma_hint;
	xf::PropertyString					_fma_speed_hint;
	xf::PropertyString					_fma_roll_hint;
	xf::PropertyString					_fma_roll_armed_hint;
	xf::PropertyString					_fma_pitch_hint;
	xf::PropertyString					_fma_pitch_armed_hint;
	// Other:
	std::set<Unique<xf::Action>>		_button_actions;
	std::set<Unique<xf::DeltaDecoder>>	_rotary_decoders;
};

#endif

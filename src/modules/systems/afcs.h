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
	static constexpr xf::Range<Length>	AltitudeRange			= { 0_ft, 50000_ft };
	static constexpr Speed				VSpdStep				= 10_fpm;
	static constexpr xf::Range<Speed>	VSpdRange				= { -8000_fpm, +8000_fpm };
	static constexpr Angle				FPAStep					= 0.1_deg;
	static constexpr xf::Range<Angle>	FPARange				= { -10_deg, +10_deg };
	static constexpr Angle				HeadingHoldPitchLimit	= 30_deg;
	static constexpr Angle				AltitudeHoldRollLimit	= 30_deg;

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

	class InvalidState: public xf::Exception
	{
	  public:
		InvalidState():
			Exception ("invalid AFCS state")
		{ }
	};

	enum class FlightStage
	{
		Cruise,
		Approach,
	};

	enum class SpeedUnits
	{
		KIAS,
		Mach,
	};

	enum class LateralDirection
	{
		Heading,
		Track,
	};

	enum class VSPDUnits
	{
		VS,
		FPA,
	};

	enum class AltitudeStep
	{
		Ft10,
		Ft100,
	};

	enum class SpeedMode
	{
		None		= 0,
		TO_GA		= 1,
		THR			= 2,
		THR_REF		= 3,
		IDLE		= 4,
		SPD_REF		= 5,
		SPD_SEL		= 6,
		MCP_SPD		= 7,
		SPD_HOLD	= 8,
		sentinel	= 9,
	};

	enum class RollMode
	{
		None		= 0,
		HDG_SEL		= 1,
		HDG_HOLD	= 2,
		HDG			= 3,
		TRK_SEL		= 4,
		TRK_HOLD	= 5,
		TRK			= 6,
		LOC			= 7,
		WNG_LVL		= 8,
		LNAV		= 9,
		sentinel	= 10,
	};

	enum class PitchMode
	{
		None		= 0,
		FLCH_SPD	= 1,
		FLCH_VS		= 2,
		FLCH_FPA	= 3,
		VS			= 4,
		FPA			= 5,
		CLB_CON		= 6,
		VNAV_SPD	= 7,
		VNAV_PTH	= 8,
		ALT			= 9,
		ALT_HOLD	= 10,
		GS			= 11,
		sentinel	= 12,
	};

  public:
	// Ctor
	AFCS (xf::ModuleManager*, QDomElement const& config);

  protected:
	void
	data_updated() override;

  private:
	/*
	 * Button and knob callbacks.
	 */

	void
	button_press_ap();

	void
	button_press_at();

	void
	button_press_yd();

	void
	button_press_speed_ias_mach();

	void
	apply_pitch_changes_for_airspeed_via_at();

	void
	button_press_speed_sel();

	void
	button_press_speed_hold();

	void
	button_press_heading_hdg_trk();

	void
	button_press_heading_sel();

	void
	button_press_heading_hold();

	void
	button_press_vnav();

	void
	button_press_lnav();

	void
	button_press_app();

	void
	button_press_altitude_stepch();

	void
	button_press_altitude_flch();

	void
	button_press_altitude_hold();

	void
	button_press_vspd_vs_fpa();

	void
	button_press_vspd_sel();

	void
	button_press_vspd_clb_con();

	void
	knob_speed (int delta);

	void
	knob_heading (int delta);

	void
	knob_altitude (int delta);

	void
	knob_vspd (int delta);

	/**
	 * Compute and solve settings of Flight Director.
	 */
	void
	solve_mode();

	/**
	 * Update FMA messages.
	 */
	void
	update_efis();

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
	 * Set spd reference for given flight stage.
	 */
	void
	set_spd_ref_for_flight_stage (FlightStage stage);

	/**
	 * Return current V/S rounded to _vertical_speed_rounding.
	 */
	Speed
	current_rounded_vs() const;

	/**
	 * Return true if A/T operates in thrust mode.
	 */
	bool
	at_in_thrust_mode();

	/**
	 * Return true if A/T operates in speed mode.
	 */
	bool
	at_in_speed_mode();

	/**
	 * Return true if pitch mode is engaged in flight level change mode.
	 */
	bool
	flch_engaged();

	/**
	 * Create a Unique<ButtonAction> for button press.
	 * Wrap button press callback and add call to solve_mode() at the end.
	 */
	Unique<xf::ButtonAction>
	make_button_action (xf::PropertyBoolean, void (AFCS::* callback)());

	/**
	 * Create a knob action for knob movement.
	 * Wrap knob rotate callback and add call to solve_mode() at the end.
	 */
	Unique<xf::DeltaDecoder>
	make_knob_action (xf::PropertyInteger, void (AFCS::* callback)(int));

  private:
	// Settings:
	Speed							_altitude_hold_threshold_vs	= 100_fpm;
	Speed							_vertical_speed_rounding	= 100_fpm;
	std::string						_mcp_speed_format_kias		= "%d";
	std::string						_mcp_speed_format_mach		= "%.3f";
	std::string						_mcp_heading_format			= "%03d";
	std::string						_mcp_altitude_format		= "%d";
	std::string						_mcp_vspd_format_vs			= "%+d";
	std::string						_mcp_vspd_format_fpa		= "%.1f";
	// State:
	FlightStage						_flight_stage				= FlightStage::Cruise;
	SpeedUnits						_speed_units				= SpeedUnits::KIAS;
	LateralDirection				_lateral_direction			= LateralDirection::Track;
	VSPDUnits						_vspd_units					= VSPDUnits::VS;
	AltitudeStep					_altitude_step				= AltitudeStep::Ft10;
	Speed							_ias_counter				= SpeedRange.min();
	xf::PropertyFloat::Type			_mach_counter				= 0.0;
	Angle							_heading_counter			= 0_deg;
	Length							_altitude_counter			= 1000_ft;
	Speed							_vspd_counter				= 0_fpm;
	Angle							_fpa_counter				= 0_deg;
	SpeedMode						_speed_mode					= SpeedMode::None;
	RollMode						_roll_mode					= RollMode::None;
	PitchMode						_pitch_mode					= PitchMode::None;
	Speed							_hold_ias;
	xf::PropertyFloat::Type			_hold_mach;
	Angle							_hold_magnetic_heading_or_track;
	Length							_hold_altitude_amsl;
	Speed							_hold_vs;
	Angle							_hold_fpa;
	bool							_ap_on						= false;
	bool							_at_on						= false;
	bool							_yd_on						= false;
	// Measurements:
	xf::PropertySpeed				_measured_vs;
	xf::PropertyLength				_measured_altitude_amsl;
	xf::PropertyAngle				_measured_magnetic_heading;
	xf::PropertyAngle				_measured_magnetic_track;
	xf::PropertyAngle				_measured_vertical_track;
	// Config:
	xf::PropertyFrequency			_thr_ref_for_cruise;
	xf::PropertyFrequency			_thr_ref_for_climb;
	xf::PropertyFrequency			_thr_ref_for_descent;
	xf::PropertySpeed				_spd_ref_for_cruise;
	xf::PropertySpeed				_spd_ref_for_approach;
	// Buttons, encoders:
	xf::PropertyBoolean				_mcp_ap_button;
	Unique<xf::ButtonAction>		_mcp_ap_action;
	xf::PropertyBoolean				_mcp_ap_led;
	xf::PropertyBoolean				_mcp_at_button;
	Unique<xf::ButtonAction>		_mcp_at_action;
	xf::PropertyBoolean				_mcp_at_led;
	xf::PropertyBoolean				_mcp_yd_button;
	Unique<xf::ButtonAction>		_mcp_yd_action;
	xf::PropertyBoolean				_mcp_yd_led;
	xf::PropertyInteger				_mcp_speed_knob;
	Unique<xf::DeltaDecoder>		_mcp_speed_decoder;
	xf::PropertyInteger				_mcp_speed_display;
	xf::PropertyBoolean				_mcp_speed_ias_mach_button;
	Unique<xf::ButtonAction>		_mcp_speed_ias_mach_action;
	xf::PropertyBoolean				_mcp_speed_sel_button;
	Unique<xf::ButtonAction>		_mcp_speed_sel_action;
	xf::PropertyBoolean				_mcp_speed_sel_led;
	xf::PropertyBoolean				_mcp_speed_hold_button;
	Unique<xf::ButtonAction>		_mcp_speed_hold_action;
	xf::PropertyBoolean				_mcp_speed_hold_led;
	xf::PropertyInteger				_mcp_heading_knob;
	Unique<xf::DeltaDecoder>		_mcp_heading_decoder;
	xf::PropertyInteger				_mcp_heading_display;
	xf::PropertyBoolean				_mcp_heading_hdg_trk_button;
	Unique<xf::ButtonAction>		_mcp_heading_hdg_trk_action;
	xf::PropertyBoolean				_mcp_heading_sel_button;
	Unique<xf::ButtonAction>		_mcp_heading_sel_action;
	xf::PropertyBoolean				_mcp_heading_sel_led;
	xf::PropertyBoolean				_mcp_heading_hold_button;
	Unique<xf::ButtonAction>		_mcp_heading_hold_action;
	xf::PropertyBoolean				_mcp_heading_hold_led;
	xf::PropertyBoolean				_mcp_vnav_button;
	Unique<xf::ButtonAction>		_mcp_vnav_action;
	xf::PropertyBoolean				_mcp_vnav_led;
	xf::PropertyBoolean				_mcp_lnav_button;
	Unique<xf::ButtonAction>		_mcp_lnav_action;
	xf::PropertyBoolean				_mcp_lnav_led;
	xf::PropertyBoolean				_mcp_app_button;
	Unique<xf::ButtonAction>		_mcp_app_action;
	xf::PropertyBoolean				_mcp_app_led;
	xf::PropertyInteger				_mcp_altitude_knob;
	Unique<xf::DeltaDecoder>		_mcp_altitude_decoder;
	xf::PropertyInteger				_mcp_altitude_display;
	xf::PropertyBoolean				_mcp_altitude_stepch_button;
	Unique<xf::ButtonAction>		_mcp_altitude_stepch_action;
	xf::PropertyBoolean				_mcp_altitude_flch_button;
	Unique<xf::ButtonAction>		_mcp_altitude_flch_action;
	xf::PropertyBoolean				_mcp_altitude_flch_led;
	xf::PropertyBoolean				_mcp_altitude_hold_button;
	Unique<xf::ButtonAction>		_mcp_altitude_hold_action;
	xf::PropertyBoolean				_mcp_altitude_hold_led;
	xf::PropertyInteger				_mcp_vspd_knob;
	Unique<xf::DeltaDecoder>		_mcp_vspd_decoder;
	xf::PropertyFloat				_mcp_vspd_display;
	xf::PropertyBoolean				_mcp_vspd_vs_fpa_button;
	Unique<xf::ButtonAction>		_mcp_vspd_vs_fpa_action;
	xf::PropertyBoolean				_mcp_vspd_sel_button;
	Unique<xf::ButtonAction>		_mcp_vspd_sel_action;
	xf::PropertyBoolean				_mcp_vspd_sel_led;
	xf::PropertyBoolean				_mcp_vspd_clb_con_button;
	Unique<xf::ButtonAction>		_mcp_vspd_clb_con_action;
	xf::PropertyBoolean				_mcp_vspd_clb_con_led;
	// Output:
	xf::PropertyInteger				_cmd_roll_mode;
	xf::PropertyInteger				_cmd_pitch_mode;
	xf::PropertySpeed				_cmd_ias;
	xf::PropertyFloat				_cmd_mach;
	xf::PropertyAngle				_cmd_magnetic_heading_track;
	xf::PropertyLength				_cmd_altitude;
	xf::PropertySpeed				_cmd_vs;
	xf::PropertyAngle				_cmd_fpa;
	xf::PropertyFrequency			_thr_ref;
	xf::PropertySpeed				_spd_ref;
	xf::PropertyBoolean				_yaw_damper_enabled;
	xf::PropertyString				_flight_mode_hint;
	xf::PropertyString				_flight_mode_speed_hint;
	xf::PropertyString				_flight_mode_roll_hint;
	xf::PropertyString				_flight_mode_pitch_hint;
	xf::PropertyString				_output_mcp_speed_format;		// String format for speed display on MCP.
	xf::PropertyString				_output_mcp_heading_format;		// String format for heading display on MCP.
	xf::PropertyString				_output_mcp_altitude_format;	// String format for altitude display on MCP.
	xf::PropertyString				_output_mcp_vspd_format;		// String format for vertical speed display on MCP.
	// Other:
	std::vector<xf::Action*>		_button_actions;
	std::vector<xf::DeltaDecoder*>	_rotary_decoders;
};

#endif

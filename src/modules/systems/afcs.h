/* vim:ts=4
 *
 * Copyleft 2012…2013  Michał Gawron
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
#include <xefis/utility/rotary_encoder.h>
#include <xefis/utility/range.h>
#include <modules/systems/fly_by_wire.h>


class AutomatedFlightControlSystem: public Xefis::Module
{
	static constexpr Xefis::Range<Speed>	CmdSpeedRange			= { 10_kt, 300_kt };
	static constexpr Xefis::Range<Length>	CmdAltitudeRange		= { 0_ft, 50000_ft };
	static constexpr Speed					CmdVSpdStep				= 10_fpm;
	static constexpr Xefis::Range<Speed>	CmdVSpdRange			= { -8000_fpm, 8000_fpm };
	static constexpr Angle					HeadingHoldPitchLimit	= 30_deg;
	static constexpr Angle					AltitudeHoldRollLimit	= 30_deg;

	enum class CmdAltitudeStep
	{
		Ft10,
		Ft100,
	};

	// When updating this, update also speed_mode_string() method.
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
		HOLD		= 8,
		sentinel	= 9,
	};

	// When updating this, update also roll_mode_string() method.
	enum class RollMode
	{
		None		= 0,
		HDG_SEL		= 1,
		HDG			= 2,
		TRK_SEL		= 3,
		TRK			= 4,
		LOC			= 5,
		sentinel	= 6,
	};

	// When updating this, update also pitch_mode_string() method.
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
	AutomatedFlightControlSystem (Xefis::ModuleManager*, QDomElement const& config);

  protected:
	void
	data_updated() override;

  private:
	void
	prepare_afcs_main_panel();

	void
	process_afcs_main_panel();

	void
	prepare_speed_panel();

	void
	prepare_heading_panel();

	void
	prepare_nav_panel();

	void
	prepare_altitude_panel();

	void
	process_altitude_panel();

	void
	prepare_vspd_panel();

	/**
	 * Compute and solve settings of Flight Director.
	 */
	void
	solve_mode();

	/**
	 * Update FMA messages.
	 */
	void
	update_fma();

	static std::string const&
	speed_mode_string (SpeedMode);

	static std::string const&
	roll_mode_string (RollMode);

	static std::string const&
	pitch_mode_string (PitchMode);

	/**
	 * Return true if given button is fresh() and set to true.
	 */
	static bool
	pressed (Xefis::PropertyBoolean const&);

  private:
	bool								_ap_on						= false;
	bool								_at_on						= false;
	bool								_att_on						= false;
	Speed								_cmd_speed_counter			= CmdSpeedRange.min();
	Speed								_cmd_speed_selected			= CmdSpeedRange.min();
	Angle								_cmd_heading_counter		= 0_deg;
	Angle								_cmd_heading_selected		= 0_deg;
	Length								_cmd_altitude_counter		= 1000_ft;
	Length								_cmd_altitude_selected		= 1000_ft;
	Speed								_cmd_vspd_counter			= 0_fpm;
	Speed								_cmd_vspd_selected			= 0_fpm;
	CmdAltitudeStep						_cmd_altitude_step			= CmdAltitudeStep::Ft10;
	SpeedMode							_speed_mode					= SpeedMode::None;
	RollMode							_roll_mode					= RollMode::None;
	PitchMode							_pitch_mode					= PitchMode::None;
	// Buttons, encoders:
	Xefis::PropertyBoolean				_mcp_ap;
	Xefis::PropertyBoolean				_mcp_at;
	Xefis::PropertyBoolean				_mcp_prot;
	Xefis::PropertyBoolean				_mcp_tac;
	Xefis::PropertyBoolean				_mcp_att;
	Xefis::PropertyBoolean				_mcp_ct;
	Xefis::PropertyBoolean				_mcp_speed_a;
	Xefis::PropertyBoolean				_mcp_speed_b;
	Unique<Xefis::RotaryEncoder>		_mcp_speed_decoder;
	Xefis::PropertyBoolean				_mcp_speed_ias_mach;
	Xefis::PropertyBoolean				_mcp_speed_sel;
	Xefis::PropertyBoolean				_mcp_speed_hold;
	Xefis::PropertyBoolean				_mcp_heading_a;
	Xefis::PropertyBoolean				_mcp_heading_b;
	Unique<Xefis::RotaryEncoder>		_mcp_heading_decoder;
	Xefis::PropertyBoolean				_mcp_heading_hdg_trk;
	Xefis::PropertyBoolean				_mcp_heading_sel;
	Xefis::PropertyBoolean				_mcp_heading_hold;
	Xefis::PropertyBoolean				_mcp_vnav;
	Xefis::PropertyBoolean				_mcp_lnav;
	Xefis::PropertyBoolean				_mcp_app;
	Xefis::PropertyBoolean				_mcp_altitude_a;
	Xefis::PropertyBoolean				_mcp_altitude_b;
	Unique<Xefis::RotaryEncoder>		_mcp_altitude_decoder;
	Xefis::PropertyBoolean				_mcp_altitude_stepch;
	Xefis::PropertyBoolean				_mcp_altitude_flch;
	Xefis::PropertyBoolean				_mcp_altitude_hold;
	Xefis::PropertyBoolean				_mcp_vspd_a;
	Xefis::PropertyBoolean				_mcp_vspd_b;
	Unique<Xefis::RotaryEncoder>		_mcp_vspd_decoder;
	Xefis::PropertyBoolean				_mcp_vspd_vs_fpa;
	Xefis::PropertyBoolean				_mcp_vspd_sel;
	Xefis::PropertyBoolean				_mcp_vspd_clb_con;
	// LEDs, displays:
	Xefis::PropertyInteger				_mcp_speed_display;
	Xefis::PropertyInteger				_mcp_heading_display;
	Xefis::PropertyInteger				_mcp_altitude_display;
	Xefis::PropertyInteger				_mcp_vspd_display;
	Xefis::PropertyBoolean				_mcp_ap_led;
	Xefis::PropertyBoolean				_mcp_att_led;
	Xefis::PropertyBoolean				_mcp_speed_sel_led;
	Xefis::PropertyBoolean				_mcp_speed_hold_led;
	Xefis::PropertyBoolean				_mcp_heading_sel_led;
	Xefis::PropertyBoolean				_mcp_heading_hold_led;
	Xefis::PropertyBoolean				_mcp_vnav_led;
	Xefis::PropertyBoolean				_mcp_lnav_led;
	Xefis::PropertyBoolean				_mcp_app_led;
	Xefis::PropertyBoolean				_mcp_altitude_flch_led;
	Xefis::PropertyBoolean				_mcp_altitude_hold_led;
	Xefis::PropertyBoolean				_mcp_vspd_sel_led;
	Xefis::PropertyBoolean				_mcp_vspd_clb_con_led;
	// Output:
	Xefis::PropertyInteger				_fbw_mode;
	Xefis::PropertyInteger				_cmd_roll_mode;
	Xefis::PropertyInteger				_cmd_pitch_mode;
	Xefis::PropertySpeed				_cmd_ias;
	Xefis::PropertyAngle				_cmd_heading_track;
	Xefis::PropertyLength				_cmd_altitude;
	Xefis::PropertySpeed				_cmd_vspd;
	Xefis::PropertyAngle				_cmd_fpa;
	Xefis::PropertyString				_fma_control_hint;
	Xefis::PropertyString				_fma_speed_mode;
	Xefis::PropertyString				_fma_roll_mode;
	Xefis::PropertyString				_fma_pitch_mode;
	// Other:
	std::vector<Xefis::RotaryEncoder*>	_rotary_decoders;
};

#endif

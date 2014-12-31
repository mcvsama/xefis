/* vim:ts=4
 *
 * Copyleft 2012…2015  Michał Gawron
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
#include <QtXml/QDomElement>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLayout>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/numeric.h>

// Local:
#include "afcs.h"


XEFIS_REGISTER_MODULE_CLASS ("systems/afcs", AFCS);


constexpr xf::Range<Speed>	AFCS::SpeedRange;
constexpr xf::Range<double>	AFCS::MachRange;
constexpr double			AFCS::MachStep;
constexpr xf::Range<Length>	AFCS::AltitudeRange;
constexpr Speed				AFCS::VSStep;
constexpr xf::Range<Speed>	AFCS::VSRange;
constexpr Angle				AFCS::FPAStep;
constexpr xf::Range<Angle>	AFCS::FPARange;


AFCS::AFCS (xf::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	xf::PropertyBoolean button_ap;
	xf::PropertyBoolean button_at;
	xf::PropertyBoolean button_yd;
	xf::PropertyBoolean button_xchg_ias_mach;
	xf::PropertyBoolean button_toga;
	xf::PropertyBoolean button_spd_sel;
	xf::PropertyBoolean button_spd_hold;
	xf::PropertyBoolean button_xchg_heading_step;
	xf::PropertyBoolean button_xchg_hdg_trk;
	xf::PropertyBoolean button_hdgtrk_sel;
	xf::PropertyBoolean button_hdgtrk_hold;
	xf::PropertyBoolean button_wng_lvl;
	xf::PropertyBoolean button_loc;
	xf::PropertyBoolean button_lnav;
	xf::PropertyBoolean button_vnav;
	xf::PropertyBoolean button_lvl_all;
	xf::PropertyBoolean button_to;
	xf::PropertyBoolean button_crz;
	xf::PropertyBoolean button_app;
	xf::PropertyBoolean button_ils;
	xf::PropertyBoolean button_xchg_altitude_step;
	xf::PropertyBoolean button_flch;
	xf::PropertyBoolean button_altitude_hold;
	xf::PropertyBoolean button_gs;
	xf::PropertyBoolean button_xchg_vs_fpa;
	xf::PropertyBoolean button_vertical_enable;
	xf::PropertyBoolean button_vertical_sel;
	xf::PropertyBoolean button_clb_con;

	xf::PropertyInteger knob_speed;
	xf::PropertyInteger knob_heading;
	xf::PropertyInteger knob_altitude;
	xf::PropertyInteger knob_vertical;

	parse_settings (config, {
		{ "mcp.default.ias", _mcp_ias, true },
		{ "mcp.default.mach", _mcp_mach, true },
		{ "mcp.default.heading", _mcp_heading, true },
		{ "mcp.default.altitude", _mcp_altitude, true },
		{ "mcp.speed-format.kias", _mcp_speed_format_kias, false },
		{ "mcp.speed-format.mach", _mcp_speed_format_mach, false },
		{ "mcp.heading-format", _mcp_heading_format, false },
		{ "mcp.altitude-format", _mcp_altitude_format, false },
		{ "mcp.vertical-format.vs", _mcp_vertical_format_vs, false },
		{ "mcp.vertical-format.fpa", _mcp_vertical_format_fpa, false },
		{ "acq-delta.ias", _acq_delta_ias, false },
		{ "acq-delta.mach", _acq_delta_mach, false },
		{ "acq-delta.heading", _acq_delta_heading, false },
		{ "acq-delta.altitude", _acq_delta_altitude, false },
		{ "vs-rounding", _vs_rounding, false },
		{ "fpa-rounding", _fpa_rounding, false },
	});

	_mcp_track = _mcp_heading;

	parse_properties (config, {
		{ "input.ias", _measured_ias, true },
		{ "input.mach", _measured_mach, true },
		{ "input.heading", _measured_heading, true },
		{ "input.track", _measured_track, true },
		{ "input.altitude.amsl", _measured_altitude_amsl, true },
		{ "input.vs", _measured_vs, true },
		{ "input.fpa", _measured_fpa, true },
		{ "input.thr-ref.toga", _thr_ref_for_toga, true },
		{ "input.thr-ref.cont", _thr_ref_for_cont, true },
		{ "input.thr-ref.cruise", _thr_ref_for_cruise, true },
		{ "input.thr-ref.descent", _thr_ref_for_descent, true },
		{ "input.spd-ref.climbout", _spd_ref_for_climbout, true },
		{ "input.spd-ref.cruise", _spd_ref_for_cruise, true },
		{ "input.spd-ref.approach", _spd_ref_for_approach, true },
		{ "input.mcp.button.ap", button_ap, true },
		{ "input.mcp.button.at", button_at, true },
		{ "input.mcp.button.yd", button_yd, true },
		{ "input.mcp.button.xchg-ias-mach", button_xchg_ias_mach, true },
		{ "input.mcp.button.toga", button_toga, true },
		{ "input.mcp.button.spd-sel", button_spd_sel, true },
		{ "input.mcp.button.spd-hold", button_spd_hold, true },
		{ "input.mcp.button.xchg-heading-step", button_xchg_heading_step, true },
		{ "input.mcp.button.xchg-hdg-trk", button_xchg_hdg_trk, true },
		{ "input.mcp.button.hdgtrk-sel", button_hdgtrk_sel, true },
		{ "input.mcp.button.hdgtrk-hold", button_hdgtrk_hold, true },
		{ "input.mcp.button.wng-lvl", button_wng_lvl, true },
		{ "input.mcp.button.loc", button_loc, true },
		{ "input.mcp.button.lnav", button_lnav, true },
		{ "input.mcp.button.vnav", button_vnav, true },
		{ "input.mcp.button.lvl-all", button_lvl_all, true },
		{ "input.mcp.button.to", button_to, true },
		{ "input.mcp.button.crz", button_crz, true },
		{ "input.mcp.button.app", button_app, true },
		{ "input.mcp.button.ils", button_ils, true },
		{ "input.mcp.button.xchg-altitude-step", button_xchg_altitude_step, true },
		{ "input.mcp.button.flch", button_flch, true },
		{ "input.mcp.button.altitude-hold", button_altitude_hold, true },
		{ "input.mcp.button.gs", button_gs, true },
		{ "input.mcp.button.xchg-vs-fpa", button_xchg_vs_fpa, true },
		{ "input.mcp.button.vertical-enable", button_vertical_enable, true },
		{ "input.mcp.button.vertical-sel", button_vertical_sel, true },
		{ "input.mcp.button.clb-con", button_clb_con, true },
		{ "input.mcp.rotary-encoder.speed", knob_speed, true },
		{ "input.mcp.rotary-encoder.heading", knob_heading, true },
		{ "input.mcp.rotary-encoder.altitude", knob_altitude, true },
		{ "input.mcp.rotary-encoder.vertical", knob_vertical, true },
		{ "output.mcp.speed-display", _mcp_speed_display, true },
		{ "output.mcp.heading-display", _mcp_heading_display, true },
		{ "output.mcp.altitude-display", _mcp_altitude_display, true },
		{ "output.mcp.vertical-display", _mcp_vertical_display, true },
		{ "output.mcp.speed-format", _mcp_speed_format_out, true },
		{ "output.mcp.heading-format", _mcp_heading_format_out, true },
		{ "output.mcp.altitude-format", _mcp_altitude_format_out, true },
		{ "output.mcp.vertical-format", _mcp_vertical_format_out, true },
		{ "output.mcp.led-ap", _mcp_led_ap, true },
		{ "output.mcp.led-at", _mcp_led_at, true },
		{ "output.mcp.led-yd", _mcp_led_yd, true },
		{ "output.cmd.thrust-mode", _cmd_thrust_mode, true },
		{ "output.cmd.roll-mode", _cmd_roll_mode, true },
		{ "output.cmd.pitch-mode", _cmd_pitch_mode, true },
		{ "output.cmd.ias", _cmd_ias, true },
		{ "output.cmd.mach", _cmd_mach, true },
		{ "output.cmd.heading", _cmd_heading, true },
		{ "output.cmd.track", _cmd_track, true },
		{ "output.cmd.altitude", _cmd_altitude, true },
		{ "output.cmd.vs", _cmd_vs, true },
		{ "output.cmd.fpa", _cmd_fpa, true },
		{ "output.thr-ref", _thr_ref, true },
		{ "output.spd-ref", _spd_ref, true },
		{ "output.fma.hint", _fma_hint, true },
		{ "output.fma.speed-hint", _fma_speed_hint, true },
		{ "output.fma.roll-hint", _fma_roll_hint, true },
		{ "output.fma.roll-armed-hint", _fma_roll_armed_hint, true },
		{ "output.fma.pitch-hint", _fma_pitch_hint, true },
		{ "output.fma.pitch-armed-hint", _fma_pitch_armed_hint, true },
	});

	make_button_action (button_ap, &AFCS::button_press_ap);
	make_button_action (button_at, &AFCS::button_press_at);
	make_button_action (button_yd, &AFCS::button_press_yd);
	make_button_action (button_xchg_ias_mach, &AFCS::button_press_xchg_ias_mach);
	make_button_action (button_toga, &AFCS::button_press_toga);
	make_button_action (button_spd_sel, &AFCS::button_press_spd_sel);
	make_button_action (button_spd_hold, &AFCS::button_press_spd_hold);
	make_button_action (button_xchg_heading_step, &AFCS::button_press_xchg_heading_step);
	make_button_action (button_xchg_hdg_trk, &AFCS::button_press_xchg_hdg_trk);
	make_button_action (button_hdgtrk_sel, &AFCS::button_press_hdgtrk_sel);
	make_button_action (button_hdgtrk_hold, &AFCS::button_press_hdgtrk_hold);
	make_button_action (button_wng_lvl, &AFCS::button_press_wng_lvl);
	make_button_action (button_loc, &AFCS::button_press_loc);
	make_button_action (button_lnav, &AFCS::button_press_lnav);
	make_button_action (button_vnav, &AFCS::button_press_vnav);
	make_button_action (button_lvl_all, &AFCS::button_press_lvl_all);
	make_button_action (button_to, &AFCS::button_press_to);
	make_button_action (button_crz, &AFCS::button_press_crz);
	make_button_action (button_app, &AFCS::button_press_app);
	make_button_action (button_ils, &AFCS::button_press_ils);
	make_button_action (button_xchg_altitude_step, &AFCS::button_press_xchg_altitude_step);
	make_button_action (button_flch, &AFCS::button_press_flch);
	make_button_action (button_altitude_hold, &AFCS::button_press_altitude_hold);
	make_button_action (button_gs, &AFCS::button_press_gs);
	make_button_action (button_xchg_vs_fpa, &AFCS::button_press_xchg_vs_fpa);
	make_button_action (button_vertical_enable, &AFCS::button_press_vertical_enable);
	make_button_action (button_vertical_sel, &AFCS::button_press_vertical_sel);
	make_button_action (button_clb_con, &AFCS::button_press_clb_con);

	make_knob_action (knob_speed, &AFCS::knob_speed_change);
	make_knob_action (knob_heading, &AFCS::knob_heading_change);
	make_knob_action (knob_altitude, &AFCS::knob_altitude_change);
	make_knob_action (knob_vertical, &AFCS::knob_vertical_change);

	solve();
}


void
AFCS::data_updated()
{
	// TODO recheck
	try {
		for (auto& r: _rotary_decoders)
			r->data_updated();

		for (auto& a: _button_actions)
			a->data_updated();

		check_input();
		check_events();
	}
	catch (DisengageAP const& e)
	{
		disengage_ap (e.what());
	}
	catch (DisengageAT const& e)
	{
		disengage_at (e.what());
	}
	catch (Disengage const& e)
	{
		disengage_ap (e.what());
		disengage_at (e.what());
	}
}


void
AFCS::button_press_ap()
{
	_ap_on = !_ap_on;
	if (_ap_on)
		_yd_on = true;
}


void
AFCS::button_press_at()
{
	_at_on = !_at_on;
}


void
AFCS::button_press_yd()
{
	_yd_on = !_yd_on;
	if (!_yd_on)
		_ap_on = false;
}


void
AFCS::knob_speed_change (int delta)
{
	switch (_speed_control)
	{
		case SpeedControl::KIAS:
			_mcp_ias = xf::limit (_mcp_ias + 1_kt * delta, SpeedRange);
			break;

		case SpeedControl::Mach:
			_mcp_mach = xf::limit (_mcp_mach + MachStep * delta, MachRange);
			break;
	}
}


void
AFCS::button_press_xchg_ias_mach()
{
	switch (_speed_control)
	{
		case SpeedControl::KIAS:
			_speed_control = SpeedControl::Mach;
			if (_measured_mach.valid())
				_cmd_mach = *_measured_mach;
			break;

		case SpeedControl::Mach:
			_speed_control = SpeedControl::KIAS;
			if (_measured_ias.valid())
				_cmd_ias = *_measured_ias;
			break;
	}
}


void
AFCS::button_press_toga()
{
	// On?
	if (_thrust_mode != ThrustMode::TO_GA)
	{
		transfer_airspeed_control_from_thrust_to_pitch();
		_thrust_mode = ThrustMode::TO_GA;
		_thr_ref.copy (_thr_ref_for_toga);
	}
	// Off?
	else
	{
		transfer_airspeed_control_from_pitch_to_thrust();
		alt_hold_with_pitch();
	}
}


void
AFCS::button_press_spd_hold()
{
	spd_hold_with_thrust();
	if (pitch_controls_airspeed())
		alt_hold_with_pitch();
}


void
AFCS::button_press_spd_sel()
{
	_thrust_mode = ThrustMode::MCP_SPD;
	if (pitch_controls_airspeed())
		alt_hold_with_pitch();
}


void
AFCS::knob_heading_change (int delta)
{
	Angle step;
	switch (_heading_step)
	{
		case HeadingStep::Deg1:
			step = 1_deg;
			break;

		case HeadingStep::Deg10:
			step = 10_deg;
			break;
	}

	switch (_lateral_control)
	{
		case LateralControl::Heading:
			_mcp_heading = xf::floored_mod (_mcp_heading + step * delta, 360_deg);
			break;

		case LateralControl::Track:
			_mcp_track = xf::floored_mod (_mcp_track + step * delta, 360_deg);
			break;
	}
}


void
AFCS::button_press_xchg_heading_step()
{
	switch (_heading_step)
	{
		case HeadingStep::Deg1:
			_heading_step = HeadingStep::Deg10;
			break;

		case HeadingStep::Deg10:
			_heading_step = HeadingStep::Deg1;
			break;
	}
}


void
AFCS::button_press_xchg_hdg_trk()
{
	Optional<Angle> track_minus_heading;
	if (_measured_heading.valid() && _measured_track.valid())
		track_minus_heading = *_measured_track - *_measured_heading;

	switch (_lateral_control)
	{
		case LateralControl::Heading:
			_lateral_control = LateralControl::Track;
			if (track_minus_heading)
				_mcp_track = _mcp_heading + *track_minus_heading;
			break;

		case LateralControl::Track:
			_lateral_control = LateralControl::Heading;
			if (track_minus_heading)
				_mcp_heading = _mcp_track - *track_minus_heading;
			break;
	}
}


void
AFCS::button_press_hdgtrk_sel()
{
	_roll_mode = RollMode::MCP;
}


void
AFCS::button_press_hdgtrk_hold()
{
	heading_hold_with_roll();
}


void
AFCS::button_press_wng_lvl()
{
	_roll_mode = RollMode::WNG_LVL;
}


void
AFCS::button_press_loc()
{
	// On?
	if (_roll_mode != RollMode::LOC)
	{
		if (_armed_roll_mode == RollMode::LOC)
			_armed_roll_mode = RollMode::None;
		else
			_armed_roll_mode = RollMode::LOC;
	}
	// Off?
	else
		_roll_mode = RollMode::WNG_LVL;
}


void
AFCS::button_press_lnav()
{
	_roll_mode = RollMode::LNAV;

	// TODO think of best use case, when turning off:
	//	_roll_mode = HOLD, lateral_control = TRK?
	//	Also if VNAV was enabled it has to be disabled.
	//	And airspeed must be controlled by something.
}


void
AFCS::button_press_vnav()
{
	transfer_airspeed_control_from_pitch_to_thrust();
	_pitch_mode = PitchMode::VNAV_PTH;

	// TODO think of best use case, when turning off:
	//  transfer airspeed ctl to thrust?
	//  pitch mode to ALT_HOLD (call alt_hold_with_pitch())?
}


void
AFCS::button_press_lvl_all()
{
	transfer_airspeed_control_from_pitch_to_thrust();
	_roll_mode = RollMode::WNG_LVL;
	alt_hold_with_pitch();
}


void
AFCS::button_press_to()
{
	_thr_ref.copy (_thr_ref_for_toga);
	_spd_ref.copy (_spd_ref_for_climbout);
}


void
AFCS::button_press_crz()
{
	_thr_ref.copy (_thr_ref_for_cruise);
	_spd_ref.copy (_spd_ref_for_cruise);
}


void
AFCS::button_press_app()
{
	_thr_ref.copy (_thr_ref_for_descent);
	_spd_ref.copy (_spd_ref_for_approach);
}


void
AFCS::button_press_ils()
{
	_armed_roll_mode = RollMode::LOC;
	_armed_pitch_mode = PitchMode::GS;
}


void
AFCS::knob_altitude_change (int delta)
{
	Length altitude_step;
	switch (_altitude_step)
	{
		case AltitudeStep::Ft10:
			altitude_step = 10_ft;
			break;

		case AltitudeStep::Ft100:
			altitude_step = 100_ft;
			break;
	}
	_mcp_altitude = xf::limit (_mcp_altitude + altitude_step * delta, AltitudeRange);
}


void
AFCS::button_press_xchg_altitude_step()
{
	switch (_altitude_step)
	{
		case AltitudeStep::Ft10:
			_altitude_step = AltitudeStep::Ft100;
			break;

		case AltitudeStep::Ft100:
			_altitude_step = AltitudeStep::Ft10;
			break;
	}
}


void
AFCS::button_press_flch()
{
	transfer_airspeed_control_from_pitch_to_thrust();
	_pitch_mode = PitchMode::MCP_ALT;
}


void
AFCS::button_press_altitude_hold()
{
	transfer_airspeed_control_from_pitch_to_thrust();
	alt_hold_with_pitch();
}


void
AFCS::button_press_gs()
{
	if (_pitch_mode != PitchMode::GS)
	{
		if (_armed_pitch_mode != PitchMode::GS)
			_armed_pitch_mode = PitchMode::GS;
		else
			_armed_pitch_mode = PitchMode::None;
	}
	else
	{
		transfer_airspeed_control_from_pitch_to_thrust();
		alt_hold_with_pitch();
	}
}


void
AFCS::knob_vertical_change (int delta)
{
	auto perhaps_alt_hold = [&] {
		if (_pitch_mode == PitchMode::MCP_ALT ||
			_pitch_mode == PitchMode::VC)
		{
			transfer_airspeed_control_from_pitch_to_thrust();
			alt_hold_with_pitch();
		}
	};

	switch (_vertical_control)
	{
		case VerticalControl::VS:
			if (!_mcp_vs)
				_mcp_vs = 0_fpm;
			_mcp_vs = xf::limit (*_mcp_vs + VSStep * delta, VSRange);

			// Disengage on 0 crossing:
			if (xf::Range<Speed> ({ -0.5 * VSStep, 0.5 * VSStep }).includes (*_mcp_vs))
			{
				_mcp_vs.reset();
				perhaps_alt_hold();
			}
			break;

		case VerticalControl::FPA:
			if (!_mcp_fpa)
				_mcp_fpa = 0_deg;
			_mcp_fpa = xf::limit (*_mcp_fpa + FPAStep * delta, FPARange);

			// Disengage on 0 crossing:
			if (xf::Range<Angle> ({ -0.5 * FPAStep, +0.5 * FPAStep }).includes (*_mcp_fpa))
			{
				_mcp_fpa.reset();
				perhaps_alt_hold();
			}
			break;
	}
}


void
AFCS::button_press_xchg_vs_fpa()
{
	switch (_vertical_control)
	{
		case VerticalControl::VS:
			_vertical_control = VerticalControl::FPA;
			if (auto fpa = current_rounded_fpa())
				_mcp_fpa = *fpa;
			break;

		case VerticalControl::FPA:
			_vertical_control = VerticalControl::VS;
			if (auto vs = current_rounded_vs())
				_mcp_vs = *vs;
			break;
	}
}


void
AFCS::button_press_vertical_enable()
{
	bool vc_enabled = false;

	switch (_vertical_control)
	{
		case VerticalControl::VS:
			if (!_mcp_vs)
			{
				if (auto vs = current_rounded_vs())
					_mcp_vs = *vs;

				vc_enabled = true;
			}
			else
				_mcp_vs.reset();
			break;

		case VerticalControl::FPA:
			if (!_mcp_fpa)
			{
				if (auto fpa = current_rounded_fpa())
					_mcp_fpa = *fpa;

				vc_enabled = true;
			}
			else
				_mcp_fpa.reset();
			break;
	}

	if (vc_enabled)
	{
		if (_pitch_mode != PitchMode::MCP_ALT)
			_pitch_mode = PitchMode::VC;
	}
	else if (!vc_enabled)
	{
		if (_pitch_mode == PitchMode::VC)
			alt_hold_with_pitch();
	}
}


void
AFCS::button_press_vertical_sel()
{
	if ((_mcp_vs && *_mcp_vs > 0.5 * VSStep) ||
		(_mcp_fpa && *_mcp_fpa > 0.5 * FPAStep))
	{
		transfer_airspeed_control_from_pitch_to_thrust();
		_pitch_mode = PitchMode::VC;
	}
}


void
AFCS::button_press_clb_con()
{
	transfer_airspeed_control_from_thrust_to_pitch();
	_thrust_mode = ThrustMode::CONT;
	_thr_ref.copy (_thr_ref_for_cont);
	_spd_ref.copy (_spd_ref_for_climbout);
}


void
AFCS::check_input()
{
	if (!_measured_ias.valid() ||
		!_measured_mach.valid() ||
		!_measured_heading.valid() ||
		!_measured_track.valid() ||
		!_measured_altitude_amsl.valid() ||
		!_measured_vs.valid() ||
		!_measured_fpa.valid())
	{
		throw Disengage ("invalid sensor input");
	}
}


void
AFCS::check_events()
{
	// TODO
	// merge MCP_ALT_VC into MCP_ALT and have separate
	// submode telling how MCP_ALT should be mainained (const pitch, v/s or fpa).
	// And when alt is acquired, reset _mcp_vs|_mcp_fpa to nil.
}


void
AFCS::solve()
{
	update_mcp();
	update_efis();
	update_output();
}


void
AFCS::update_mcp()
{
	// LEDs:
	_mcp_led_ap.write (_ap_on);
	_mcp_led_at.write (_at_on);
	_mcp_led_yd.write (_yd_on);

	// Speed window:
	switch (_speed_control)
	{
		case SpeedControl::KIAS:
			_mcp_speed_format_out = _mcp_speed_format_kias;
			_mcp_speed_display.write (xf::symmetric_round (_mcp_ias.kt()));
			break;

		case SpeedControl::Mach:
			_mcp_speed_format_out = _mcp_speed_format_mach;
			_mcp_speed_display.write (_mcp_mach);
			break;
	}

	// Heading window:
	_mcp_heading_format_out = _mcp_heading_format;
	int lateral_angle = 0;
	switch (_lateral_control)
	{
		case LateralControl::Heading:
			lateral_angle = xf::symmetric_round (_mcp_heading.deg());
			break;

		case LateralControl::Track:
			lateral_angle = xf::symmetric_round (_mcp_track.deg());
			break;
	}
	if (lateral_angle == 0)
		lateral_angle = 360;
	_mcp_heading_display.write (lateral_angle);

	// Altitude window:
	_mcp_altitude_format_out = _mcp_altitude_format;
	_mcp_altitude_display.write (xf::symmetric_round (_mcp_altitude.ft()));

	// Vertical-control window:
	switch (_vertical_control)
	{
		case VerticalControl::VS:
			_mcp_vertical_format_out = _mcp_vertical_format_vs;
			if (_mcp_vs)
				_mcp_vertical_display.write (xf::symmetric_round (_mcp_vs->fpm()));
			else
				_mcp_vertical_display.set_nil();
			break;

		case VerticalControl::FPA:
			_mcp_vertical_format_out = _mcp_vertical_format_fpa;
			if (_mcp_fpa)
				_mcp_vertical_display.write (xf::symmetric_round (10.0 * _mcp_fpa->deg()) / 10.0);
			else
				_mcp_vertical_display.set_nil();
			break;
	}
}


void
AFCS::update_efis()
{
	switch (_thrust_mode)
	{
		case ThrustMode::None:
			_fma_speed_hint = "";
			break;

		case ThrustMode::TO_GA:
			_fma_speed_hint = "TO/GA";
			break;

		case ThrustMode::CONT:
			_fma_speed_hint = "CONT";
			break;

		case ThrustMode::IDLE:
			_fma_speed_hint = "IDLE";
			break;

		case ThrustMode::MCP_SPD:
			switch (_speed_control)
			{
				case SpeedControl::KIAS:
					if (_measured_ias.valid())
					{
						if (std::abs (*_measured_ias - _mcp_ias) < _acq_delta_ias)
							_fma_speed_hint = "MCP SPD";
						else
							_fma_speed_hint = "SPD";
					}
					break;

				case SpeedControl::Mach:
					if (_measured_mach.valid())
					{
						if (std::abs (*_measured_mach - _mcp_mach) < _acq_delta_mach)
							_fma_speed_hint = "MCP SPD";
						else
							_fma_speed_hint = "SPD";
					}
			}
			break;

		case ThrustMode::SPD_HOLD:
			_fma_speed_hint = "SPD HOLD";
			break;

		default:
			_fma_speed_hint = "X";
	}

	switch (_roll_mode)
	{
		case RollMode::None:
			_fma_roll_hint = "";
			break;

		case RollMode::MCP:
			switch (_lateral_control)
			{
				case LateralControl::Heading:
					if (_measured_heading.valid())
					{
						if (std::abs (*_measured_heading - _mcp_heading) < _acq_delta_heading)
							_fma_roll_hint = "HDG";
						else
							_fma_roll_hint = "HDG SEL";
					}
					break;

				case LateralControl::Track:
					if (_measured_track.valid())
					{
						if (std::abs (*_measured_track - _mcp_track) < _acq_delta_heading)
							_fma_roll_hint = "TRK";
						else
							_fma_roll_hint = "TRK SEL";
					}
					break;
			}
			break;

		case RollMode::HOLD:
			switch (_lateral_control)
			{
				case LateralControl::Heading:
					_fma_roll_hint = "HDG HOLD";
					break;

				case LateralControl::Track:
					_fma_roll_hint = "TRK HOLD";
					break;
			}
			break;

		case RollMode::WNG_LVL:
			_fma_roll_hint = "WNG LVL";
			break;

		case RollMode::LOC:
			_fma_roll_hint = "LOC";
			break;

		case RollMode::LNAV:
			_fma_roll_hint = "LNAV";
			break;

		default:
			_fma_roll_hint = "X";
			break;
	}

	switch (_armed_roll_mode)
	{
		case RollMode::None:
			_fma_roll_armed_hint = "";
			break;

		case RollMode::LOC:
			_fma_roll_armed_hint = "LOC";
			break;

		default:
			_fma_roll_armed_hint = "X";
	}

	switch (_pitch_mode)
	{
		case PitchMode::None:
			_fma_pitch_hint = "";
			break;

		case PitchMode::MCP_SPD:
			_fma_pitch_hint = "SPD";
			break;

		case PitchMode::ALT_HOLD:
			_fma_pitch_hint = "ALT HOLD";
			break;

		case PitchMode::MCP_ALT:
			if (_measured_altitude_amsl.valid())
			{
				if (std::abs (*_measured_altitude_amsl - *_cmd_altitude) <= _acq_delta_altitude)
					_fma_pitch_hint = "ALT";
				else
				{
					if (_cmd_vs.valid())
						_fma_pitch_hint = "FLCH V/S";
					else if (_cmd_fpa.valid())
						_fma_pitch_hint = "FLCH FPA";
					else
						_fma_pitch_hint = "FLCH";
				}
			}
			break;

		case PitchMode::VC:
			switch (_vertical_control)
			{
				case VerticalControl::VS:
					_fma_pitch_hint = "V/S";
					break;

				case VerticalControl::FPA:
					_fma_pitch_hint = "FPA";
					break;
			}
			break;

		case PitchMode::VNAV_PTH:
			_fma_pitch_hint = "VNAV PTH";
			break;

		case PitchMode::GS:
			_fma_pitch_hint = "G/S";
			break;

		case PitchMode::FLARE:
			_fma_pitch_hint = "FLARE";
			break;

		default:
			_fma_pitch_hint = "X";
			break;
	}

	switch (_armed_pitch_mode)
	{
		case PitchMode::None:
			_fma_pitch_armed_hint = "";
			break;

		case PitchMode::GS:
			_fma_pitch_armed_hint = "G/S";
			break;

		default:
			_fma_pitch_armed_hint = "X";
	}

	if (_ap_on)
		_fma_hint = "A/P";
	else
		_fma_hint = "F/D";
}


void
AFCS::update_output()
{
	// Modes:
	_cmd_thrust_mode = stringify_thrust_mode();
	_cmd_roll_mode = stringify_roll_mode();
	_cmd_pitch_mode = stringify_pitch_mode();

	// Settings:

	if (_thrust_mode != ThrustMode::SPD_HOLD)
	{
		_cmd_ias = _mcp_ias;
		_cmd_mach = _mcp_mach;
	}

	if (_roll_mode != RollMode::HOLD)
	{
		_cmd_heading = _mcp_heading;
		_cmd_track = _mcp_track;
	}

	if (_pitch_mode != PitchMode::ALT_HOLD)
		_cmd_altitude = _mcp_altitude;

	if (_mcp_vs)
		_cmd_vs = *_mcp_vs;
	else
		_cmd_vs.set_nil();

	if (_mcp_fpa)
		_cmd_fpa = *_mcp_fpa;
	else
		_cmd_fpa.set_nil();
}


void
AFCS::disengage_ap (const char* reason)
{
	std::clog << "AFCS A/P disengage: " << reason << std::endl;
	_ap_on = false;
	_roll_mode = RollMode::None;
	_pitch_mode = PitchMode::None;
	solve();
}


void
AFCS::disengage_at (const char* reason)
{
	std::clog << "AFCS A/T disengage: " << reason << std::endl;
	_at_on = false;
	_thrust_mode = ThrustMode::None;
	solve();
}


void
AFCS::spd_hold_with_thrust()
{
	_thrust_mode = ThrustMode::SPD_HOLD;

	switch (_speed_control)
	{
		case SpeedControl::KIAS:
			if (_measured_ias.valid())
				_cmd_ias = *_measured_ias;
			break;

		case SpeedControl::Mach:
			if (_measured_mach.valid())
				_cmd_mach = *_measured_mach;
			break;
	}
}


void
AFCS::heading_hold_with_roll()
{
	_roll_mode = RollMode::HOLD;

	switch (_lateral_control)
	{
		case LateralControl::Heading:
			if (_measured_heading.valid())
				_cmd_heading = *_measured_heading;
			break;

		case LateralControl::Track:
			if (_measured_track.valid())
				_cmd_track = *_measured_track;
			break;
	}
}


void
AFCS::alt_hold_with_pitch()
{
	_pitch_mode = PitchMode::ALT_HOLD;

	if (_measured_altitude_amsl.valid())
		_cmd_altitude = *_measured_altitude_amsl;
}


void
AFCS::xchg_modes (PitchMode a, PitchMode b)
{
	if (_pitch_mode == a)
		_pitch_mode = b;
	else if (_pitch_mode == b)
		_pitch_mode = a;
}


inline Optional<Speed>
AFCS::current_rounded_vs() const
{
	if (!_measured_vs.valid())
		return {};

	int vsr = _vs_rounding.fpm();
	return 1_fpm * std::round (_measured_vs->fpm() / vsr) * vsr;
}


inline Optional<Angle>
AFCS::current_rounded_fpa() const
{
	if (!_measured_fpa.valid())
		return {};

	int fpar = _fpa_rounding.deg();
	return 1_deg * std::round (_measured_fpa->deg() / fpar) * fpar;
}


inline bool
AFCS::vnav_enabled() const
{
	return _pitch_mode == PitchMode::VNAV_PTH;
}


inline bool
AFCS::pitch_controls_airspeed() const
{
	return _pitch_mode == PitchMode::MCP_SPD;
}


inline void
AFCS::transfer_airspeed_control_from_thrust_to_pitch()
{
	if (_thrust_mode == ThrustMode::MCP_SPD)
		_pitch_mode = PitchMode::MCP_SPD;
}


inline void
AFCS::transfer_airspeed_control_from_pitch_to_thrust()
{
	if (_pitch_mode == PitchMode::MCP_SPD)
		_thrust_mode = ThrustMode::MCP_SPD;
}


inline void
AFCS::make_button_action (xf::PropertyBoolean property, void (AFCS::* callback)())
{
	Unique<xf::ButtonAction> action = std::make_unique<xf::ButtonAction> (property, [this,callback] {
		try {
			(this->*callback)();
		}
		catch (...)
		{
			solve();
			throw;
		}
	});

	_button_actions.insert (std::move (action));
}


inline void
AFCS::make_knob_action (xf::PropertyInteger property, void (AFCS::* callback)(int))
{
	Unique<xf::DeltaDecoder> action = std::make_unique<xf::DeltaDecoder> (property, [this,callback](int delta) {
		try {
			(this->*callback) (delta);
		}
		catch (...)
		{
			solve();
			throw;
		}
	});

	// Initialize:
	action->call (0);
	_rotary_decoders.insert (std::move (action));
}


Optional<std::string>
AFCS::stringify_thrust_mode() const
{
	auto s = [&](const char* str) {
		return Optional<std::string> (str);
	};

	switch (_thrust_mode)
	{
		case ThrustMode::None:		return s ("none");
		case ThrustMode::TO_GA:		return s ("to/ga");
		case ThrustMode::CONT:		return s ("cont");
		case ThrustMode::IDLE:		return s ("idle");
		case ThrustMode::MCP_SPD:
			switch (_speed_control)
			{
				case SpeedControl::KIAS:	return s ("mcp-kias");
				case SpeedControl::Mach:	return s ("mcp-mach");
			}
			break;
		case ThrustMode::SPD_HOLD:
			switch (_speed_control)
			{
				case SpeedControl::KIAS:	return s ("kias-hold");
				case SpeedControl::Mach:	return s ("mach-hold");
			}
			break;
		case ThrustMode::sentinel:	return {};
	};

	return {};
}


Optional<std::string>
AFCS::stringify_roll_mode() const
{
	auto s = [&](const char* str) {
		return Optional<std::string> (str);
	};

	switch (_roll_mode)
	{
		case RollMode::None:		return s ("none");
		case RollMode::MCP:
			switch (_lateral_control)
			{
				case LateralControl::Heading:	return s ("mcp-hdg");
				case LateralControl::Track:		return s ("mcp-trk");
			}
			break;
		case RollMode::HOLD:
			switch (_lateral_control)
			{
				case LateralControl::Heading:	return s ("hdg-hold");
				case LateralControl::Track:		return s ("trk-hold");
			}
			break;
		case RollMode::WNG_LVL:		return s ("wng-lvl");
		case RollMode::LOC:			return s ("loc");
		case RollMode::LNAV:		return s ("lnav");
		case RollMode::sentinel:	return {};
	}

	return {};
}


Optional<std::string>
AFCS::stringify_pitch_mode() const
{
	auto s = [&](const char* str) {
		return Optional<std::string> (str);
	};

	switch (_pitch_mode)
	{
		case PitchMode::None:		return s ("none");
		case PitchMode::MCP_SPD:	return s ("mcp-spd");
		case PitchMode::ALT_HOLD:	return s ("alt-hold");
		case PitchMode::MCP_ALT:
			if (_cmd_vs.valid())
				return s ("mcp-alt-vs");
			else if (_cmd_fpa.valid())
				return s ("mcp-alt-fpa");
			else
				return s ("mcp-alt-pitch");
		case PitchMode::VC:
			switch (_vertical_control)
			{
				case VerticalControl::VS:	return s ("vs");
				case VerticalControl::FPA:	return s ("fpa");
			}
			break;
		case PitchMode::VNAV_PTH:	return s ("vnav-pth");
		case PitchMode::GS:			return s ("g/s");
		case PitchMode::FLARE:		return s ("flare");
		case PitchMode::sentinel:	return {};
	}

	return {};
}


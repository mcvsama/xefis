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

// Local:
#include "afcs.h"
#include "afcs_api.h"

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/qt/qdom.h>
#include <neutrino/qt/qstring.h>
#include <neutrino/numeric.h>

// Qt:
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLayout>

// Standard:
#include <cstddef>


AFCS::AFCS (xf::ProcessingLoop& loop, std::string_view const instance):
	AFCS_IO (loop, instance)
{
	make_button_action (_io.button_ap, &AFCS::button_press_ap);
	make_button_action (_io.button_at, &AFCS::button_press_at);
	make_button_action (_io.button_yd, &AFCS::button_press_yd);
	make_button_action (_io.button_xchg_ias_mach, &AFCS::button_press_xchg_ias_mach);
	make_button_action (_io.button_toga, &AFCS::button_press_toga);
	make_button_action (_io.button_spd_sel, &AFCS::button_press_spd_sel);
	make_button_action (_io.button_spd_hold, &AFCS::button_press_spd_hold);
	make_button_action (_io.button_xchg_heading_step, &AFCS::button_press_xchg_heading_step);
	make_button_action (_io.button_xchg_hdg_trk, &AFCS::button_press_xchg_hdg_trk);
	make_button_action (_io.button_hdgtrk_sel, &AFCS::button_press_hdgtrk_sel);
	make_button_action (_io.button_hdgtrk_hold, &AFCS::button_press_hdgtrk_hold);
	make_button_action (_io.button_wng_lvl, &AFCS::button_press_wng_lvl);
	make_button_action (_io.button_loc, &AFCS::button_press_loc);
	make_button_action (_io.button_lnav, &AFCS::button_press_lnav);
	make_button_action (_io.button_vnav, &AFCS::button_press_vnav);
	make_button_action (_io.button_lvl_all, &AFCS::button_press_lvl_all);
	make_button_action (_io.button_to, &AFCS::button_press_to);
	make_button_action (_io.button_crz, &AFCS::button_press_crz);
	make_button_action (_io.button_app, &AFCS::button_press_app);
	make_button_action (_io.button_ils, &AFCS::button_press_ils);
	make_button_action (_io.button_xchg_altitude_step, &AFCS::button_press_xchg_altitude_step);
	make_button_action (_io.button_flch, &AFCS::button_press_flch);
	make_button_action (_io.button_altitude_hold, &AFCS::button_press_altitude_hold);
	make_button_action (_io.button_gs, &AFCS::button_press_gs);
	make_button_action (_io.button_xchg_vs_fpa, &AFCS::button_press_xchg_vs_fpa);
	make_button_action (_io.button_vertical_enable, &AFCS::button_press_vertical_enable);
	make_button_action (_io.button_vertical_sel, &AFCS::button_press_vertical_sel);
	make_button_action (_io.button_clb_con, &AFCS::button_press_clb_con);

	make_knob_action (_io.knob_speed, &AFCS::knob_speed_change);
	make_knob_action (_io.knob_heading, &AFCS::knob_heading_change);
	make_knob_action (_io.knob_altitude, &AFCS::knob_altitude_change);
	make_knob_action (_io.knob_vertical, &AFCS::knob_vertical_change);

	solve();
}


void
AFCS::process (xf::Cycle const&)
{
	// TODO recheck
	try {
		for (auto& r: _rotary_decoders)
			r->process();

		for (auto& a: _socket_actions)
			a->process();

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
			_mcp_ias = neutrino::clamp (_mcp_ias + 1_kt * delta, kSpeedRange);
			break;

		case SpeedControl::Mach:
			_mcp_mach = neutrino::clamp (_mcp_mach + kMachStep * delta, kMachRange);
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
			if (_io.measured_mach)
				_io.cmd_mach = *_io.measured_mach;
			break;

		case SpeedControl::Mach:
			_speed_control = SpeedControl::KIAS;
			if (_io.measured_ias)
				_io.cmd_ias = *_io.measured_ias;
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
		_io.thr_ref = _io.thr_ref_for_toga;
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
	si::Angle step;
	switch (_heading_step)
	{
		case HeadingStep::Deg1:
			step = 1_deg;
			break;

		case HeadingStep::Deg10:
			step = 10_deg;
			break;
	}

	_mcp_heading = xf::floored_mod (_mcp_heading + step * delta, 360_deg);
	_mcp_track = xf::floored_mod (_mcp_track + step * delta, 360_deg);
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
	std::optional<si::Angle> track_minus_heading;
	if (_io.measured_heading_magnetic && _io.measured_track_magnetic)
		track_minus_heading = *_io.measured_track_magnetic - *_io.measured_heading_magnetic;

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
	_io.thr_ref = _io.thr_ref_for_toga;
	_io.spd_ref = _io.spd_ref_for_climbout;
}


void
AFCS::button_press_crz()
{
	_io.thr_ref = _io.thr_ref_for_cruise;
	_io.spd_ref = _io.spd_ref_for_cruise;
}


void
AFCS::button_press_app()
{
	_io.thr_ref = _io.thr_ref_for_descent;
	_io.spd_ref = _io.spd_ref_for_approach;
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
	si::Length altitude_step;
	switch (_altitude_step)
	{
		case AltitudeStep::Ft10:
			altitude_step = 10_ft;
			break;

		case AltitudeStep::Ft100:
			altitude_step = 100_ft;
			break;
	}
	_mcp_altitude = neutrino::clamp (_mcp_altitude + altitude_step * delta, kAltitudeRange);
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
			_mcp_vs = neutrino::clamp (*_mcp_vs + kVSStep * delta, kVSRange);

			// Disengage on 0 crossing:
			if (xf::Range (-0.5 * kVSStep, 0.5 * kVSStep).includes (*_mcp_vs))
			{
				_mcp_vs.reset();
				perhaps_alt_hold();
			}
			break;

		case VerticalControl::FPA:
			if (!_mcp_fpa)
				_mcp_fpa = 0_deg;
			_mcp_fpa = neutrino::clamp (*_mcp_fpa + kFPAStep * delta, kFPARange);

			// Disengage on 0 crossing:
			if (xf::Range (-0.5 * kFPAStep, +0.5 * kFPAStep).includes (*_mcp_fpa))
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
	if ((_mcp_vs && *_mcp_vs > 0.5 * kVSStep) ||
		(_mcp_fpa && *_mcp_fpa > 0.5 * kFPAStep))
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
	_io.thr_ref = _io.thr_ref_for_cont;
	_io.spd_ref = _io.spd_ref_for_climbout;
}


void
AFCS::check_input()
{
	std::array<xf::BasicModuleSocket*, 7> checked_props = { {
		&_io.measured_ias,
		&_io.measured_mach,
		&_io.measured_heading_magnetic,
		&_io.measured_track_magnetic,
		&_io.measured_altitude_amsl,
		&_io.measured_vs,
		&_io.measured_fpa,
	} };

	if (std::any_of (checked_props.begin(), checked_props.end(), [](xf::BasicSocket* p) { return !p->valid(); }))
	{
		QStringList failed_props;
		for (auto const& p: checked_props)
			if (!p->valid())
				failed_props.push_back (neutrino::to_qstring (p->path().string()));
		throw Disengage ("invalid sensor input on props: " + failed_props.join (", ").toStdString());
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
	_io.mcp_led_ap = _ap_on;
	_io.mcp_led_at = _at_on;
	_io.mcp_led_yd = _yd_on;

	// Speed window:
	switch (_speed_control)
	{
		case SpeedControl::KIAS:
			_io.mcp_speed_format_out = *_io.mcp_speed_format_kias;
			_io.mcp_speed_display = xf::symmetric_round (_mcp_ias.in<si::Knot>());
			break;

		case SpeedControl::Mach:
			_io.mcp_speed_format_out = *_io.mcp_speed_format_mach;
			_io.mcp_speed_display = _mcp_mach;
			break;
	}

	// Heading window:
	_io.mcp_heading_format_out = *_io.mcp_heading_format;
	int lateral_angle = 0;

	switch (_lateral_control)
	{
		case LateralControl::Heading:
			lateral_angle = xf::symmetric_round (_mcp_heading.in<si::Degree>());
			break;

		case LateralControl::Track:
			lateral_angle = xf::symmetric_round (_mcp_track.in<si::Degree>());
			break;
	}

	if (lateral_angle == 0)
		lateral_angle = 360;

	_io.mcp_heading_display = lateral_angle;

	// Altitude window:
	_io.mcp_altitude_format_out = *_io.mcp_altitude_format;
	_io.mcp_altitude_display = xf::symmetric_round (_mcp_altitude.in<si::Foot>());

	// Vertical-control window:
	switch (_vertical_control)
	{
		case VerticalControl::VS:
			_io.mcp_vertical_format_out = *_io.mcp_vertical_format_vs;

			if (_mcp_vs)
				_io.mcp_vertical_display = xf::symmetric_round (_mcp_vs->in<si::FootPerMinute>());
			else
				_io.mcp_vertical_display = xf::nil;
			break;

		case VerticalControl::FPA:
			_io.mcp_vertical_format_out = *_io.mcp_vertical_format_fpa;

			if (_mcp_fpa)
				_io.mcp_vertical_display = xf::symmetric_round (10.0 * _mcp_fpa->in<si::Degree>()) / 10.0;
			else
				_io.mcp_vertical_display = xf::nil;
			break;
	}
}


void
AFCS::update_efis()
{
	using std::abs;

	switch (_thrust_mode)
	{
		case ThrustMode::None:
			_io.fma_speed_hint = "";
			break;

		case ThrustMode::TO_GA:
			_io.fma_speed_hint = "TO/GA";
			break;

		case ThrustMode::CONT:
			_io.fma_speed_hint = "CONT";
			break;

		case ThrustMode::IDLE:
			_io.fma_speed_hint = "IDLE";
			break;

		case ThrustMode::MCP_SPD:
			switch (_speed_control)
			{
				case SpeedControl::KIAS:
					if (_io.measured_ias)
					{
						if (abs (*_io.measured_ias - _mcp_ias) < *_io.acq_delta_ias)
							_io.fma_speed_hint = "MCP SPD";
						else
							_io.fma_speed_hint = "SPD";
					}
					break;

				case SpeedControl::Mach:
					if (_io.measured_mach)
					{
						if (abs (*_io.measured_mach - _mcp_mach) < *_io.acq_delta_mach)
							_io.fma_speed_hint = "MCP SPD";
						else
							_io.fma_speed_hint = "SPD";
					}
			}
			break;

		case ThrustMode::SPD_HOLD:
			_io.fma_speed_hint = "SPD HOLD";
			break;

		default:
			_io.fma_speed_hint = "X";
	}

	switch (_roll_mode)
	{
		case RollMode::None:
			_io.fma_roll_hint = "";
			break;

		case RollMode::MCP:
			switch (_lateral_control)
			{
				case LateralControl::Heading:
					if (_io.measured_heading_magnetic)
					{
						if (abs (*_io.measured_heading_magnetic - _mcp_heading) < *_io.acq_delta_heading)
							_io.fma_roll_hint = "HDG";
						else
							_io.fma_roll_hint = "HDG SEL";
					}
					break;

				case LateralControl::Track:
					if (_io.measured_track_magnetic)
					{
						if (abs (*_io.measured_track_magnetic - _mcp_track) < *_io.acq_delta_heading)
							_io.fma_roll_hint = "TRK";
						else
							_io.fma_roll_hint = "TRK SEL";
					}
					break;
			}
			break;

		case RollMode::HOLD:
			switch (_lateral_control)
			{
				case LateralControl::Heading:
					_io.fma_roll_hint = "HDG HOLD";
					break;

				case LateralControl::Track:
					_io.fma_roll_hint = "TRK HOLD";
					break;
			}
			break;

		case RollMode::WNG_LVL:
			_io.fma_roll_hint = "WNG LVL";
			break;

		case RollMode::LOC:
			_io.fma_roll_hint = "LOC";
			break;

		case RollMode::LNAV:
			_io.fma_roll_hint = "LNAV";
			break;

		default:
			_io.fma_roll_hint = "X";
			break;
	}

	switch (_armed_roll_mode)
	{
		case RollMode::None:
			_io.fma_roll_armed_hint = "";
			break;

		case RollMode::LOC:
			_io.fma_roll_armed_hint = "LOC";
			break;

		default:
			_io.fma_roll_armed_hint = "X";
	}

	switch (_pitch_mode)
	{
		case PitchMode::None:
			_io.fma_pitch_hint = "";
			break;

		case PitchMode::MCP_SPD:
			_io.fma_pitch_hint = "SPD";
			break;

		case PitchMode::ALT_HOLD:
			_io.fma_pitch_hint = "ALT HOLD";
			break;

		case PitchMode::MCP_ALT:
			if (_io.measured_altitude_amsl)
			{
				if (abs (*_io.measured_altitude_amsl - *_io.cmd_altitude) <= *_io.acq_delta_altitude)
					_io.fma_pitch_hint = "ALT";
				else
				{
					if (_io.cmd_vs)
						_io.fma_pitch_hint = "FLCH V/S";
					else if (_io.cmd_fpa)
						_io.fma_pitch_hint = "FLCH FPA";
					else
						_io.fma_pitch_hint = "FLCH";
				}
			}
			break;

		case PitchMode::VC:
			switch (_vertical_control)
			{
				case VerticalControl::VS:
					_io.fma_pitch_hint = "V/S";
					break;

				case VerticalControl::FPA:
					_io.fma_pitch_hint = "FPA";
					break;
			}
			break;

		case PitchMode::VNAV_PTH:
			_io.fma_pitch_hint = "VNAV PTH";
			break;

		case PitchMode::GS:
			_io.fma_pitch_hint = "G/S";
			break;

		case PitchMode::FLARE:
			_io.fma_pitch_hint = "FLARE";
			break;

		default:
			_io.fma_pitch_hint = "X";
			break;
	}

	switch (_armed_pitch_mode)
	{
		case PitchMode::None:
			_io.fma_pitch_armed_hint = "";
			break;

		case PitchMode::GS:
			_io.fma_pitch_armed_hint = "G/S";
			break;

		default:
			_io.fma_pitch_armed_hint = "X";
	}

	if (_ap_on)
		_io.fma_hint = "A/P";
	else
		_io.fma_hint = "F/D";
}


void
AFCS::update_output()
{
	// Modes:
	_io.cmd_thrust_mode = optional_cast<int64_t> (translate_thrust_mode());
	_io.cmd_roll_mode = optional_cast<int64_t> (translate_roll_mode());
	_io.cmd_pitch_mode = optional_cast<int64_t> (translate_pitch_mode());

	// Settings:

	if (_thrust_mode != ThrustMode::SPD_HOLD)
	{
		_io.cmd_ias = _mcp_ias;
		_io.cmd_mach = _mcp_mach;
	}

	if (_roll_mode != RollMode::HOLD)
	{
		_io.cmd_heading_magnetic = _mcp_heading;
		_io.cmd_track_magnetic = _mcp_track;

		_io.cmd_use_trk = (_lateral_control == LateralControl::Track);
	}

	if (_pitch_mode != PitchMode::ALT_HOLD)
		_io.cmd_altitude = _mcp_altitude;

	_io.cmd_vs = _mcp_vs;
	_io.cmd_fpa = _mcp_fpa;
}


void
AFCS::disengage_ap (const char* reason)
{
	if (_ap_on)
	{
		std::clog << "AFCS A/P disengage: " << reason << std::endl;
		_ap_on = false;
		_roll_mode = RollMode::None;
		_pitch_mode = PitchMode::None;
		solve();
	}
}


void
AFCS::disengage_at (const char* reason)
{
	if (_at_on)
	{
		std::clog << "AFCS A/T disengage: " << reason << std::endl;
		_at_on = false;
		_thrust_mode = ThrustMode::None;
		solve();
	}
}


void
AFCS::spd_hold_with_thrust()
{
	_thrust_mode = ThrustMode::SPD_HOLD;

	switch (_speed_control)
	{
		case SpeedControl::KIAS:
			if (_io.measured_ias)
				_io.cmd_ias = *_io.measured_ias;
			break;

		case SpeedControl::Mach:
			if (_io.measured_mach)
				_io.cmd_mach = *_io.measured_mach;
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
			if (_io.measured_heading_magnetic)
				_io.cmd_heading_magnetic = *_io.measured_heading_magnetic;
			break;

		case LateralControl::Track:
			if (_io.measured_track_magnetic)
				_io.cmd_track_magnetic = *_io.measured_track_magnetic;
			break;
	}
}


void
AFCS::alt_hold_with_pitch()
{
	_pitch_mode = PitchMode::ALT_HOLD;

	if (_io.measured_altitude_amsl)
		_io.cmd_altitude = *_io.measured_altitude_amsl;
}


void
AFCS::xchg_modes (PitchMode a, PitchMode b)
{
	if (_pitch_mode == a)
		_pitch_mode = b;
	else if (_pitch_mode == b)
		_pitch_mode = a;
}


inline std::optional<si::Velocity>
AFCS::current_rounded_vs() const
{
	if (_io.measured_vs)
		return std::round (*_io.measured_vs / *_io.vs_rounding) * *_io.vs_rounding;

	return std::nullopt;
}


inline std::optional<si::Angle>
AFCS::current_rounded_fpa() const
{
	if (_io.measured_fpa)
		return std::round (*_io.measured_fpa / *_io.fpa_rounding) * *_io.fpa_rounding;

	return std::nullopt;
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


void
AFCS::make_button_action (xf::ModuleIn<bool>& socket, void (AFCS::* callback)())
{
	auto action = std::make_unique<xf::SocketValueChangedAction<bool>> (socket, [this,callback] (auto const& value) {
		if (value.value_or (true))
		{
			try {
				(this->*callback)();
				solve();
			}
			catch (...)
			{
				solve();
				throw;
			}
		}
	});

	_socket_actions.insert (std::move (action));
}


void
AFCS::make_knob_action (xf::ModuleIn<int64_t>& socket, void (AFCS::* callback)(int))
{
	auto action = std::make_unique<xf::SocketDeltaDecoder<>> (socket, [this, callback] (auto delta) {
		try {
			if (delta)
				(this->*callback) (*delta);

			solve();
		}
		catch (...)
		{
			solve();
			throw;
		}
	});

	// Initialize:
	action->call_action (0);
	_rotary_decoders.insert (std::move (action));
}


std::optional<afcs::ThrustMode>
AFCS::translate_thrust_mode() const
{
	switch (_thrust_mode)
	{
		case ThrustMode::None:		return afcs::ThrustMode::None;
		case ThrustMode::TO_GA:		return afcs::ThrustMode::TO_GA;
		case ThrustMode::CONT:		return afcs::ThrustMode::Continuous;
		case ThrustMode::IDLE:		return afcs::ThrustMode::Idle;
		case ThrustMode::MCP_SPD:
		case ThrustMode::SPD_HOLD:
			switch (_speed_control)
			{
				case SpeedControl::KIAS:	return afcs::ThrustMode::KIAS;
				case SpeedControl::Mach:	return afcs::ThrustMode::Mach;
			}
			break;
	};

	return { };
}


std::optional<afcs::RollMode>
AFCS::translate_roll_mode() const
{
	switch (_roll_mode)
	{
		case RollMode::None:		return afcs::RollMode::None;
		case RollMode::MCP:
		case RollMode::HOLD:
			switch (_lateral_control)
			{
				case LateralControl::Heading:	return afcs::RollMode::Heading;
				case LateralControl::Track:		return afcs::RollMode::Track;
			}
			break;
		case RollMode::WNG_LVL:		return afcs::RollMode::WingsLevel;
		case RollMode::LOC:			return afcs::RollMode::Localizer;
		case RollMode::LNAV:		return afcs::RollMode::LNAV;
	}

	return { };
}


std::optional<afcs::PitchMode>
AFCS::translate_pitch_mode() const
{
	switch (_pitch_mode)
	{
		case PitchMode::None:		return afcs::PitchMode::None;
		case PitchMode::MCP_SPD:
			switch (_speed_control)
			{
				case SpeedControl::KIAS:	return afcs::PitchMode::KIAS;
				case SpeedControl::Mach:	return afcs::PitchMode::Mach;
			}
			break;
		case PitchMode::ALT_HOLD:	return afcs::PitchMode::Altitude;
		case PitchMode::MCP_ALT:
			if (_io.cmd_vs)
				return afcs::PitchMode::VS;
			else if (_io.cmd_fpa)
				return afcs::PitchMode::FPA;
			else
				return afcs::PitchMode::Altitude;
		case PitchMode::VC:
			switch (_vertical_control)
			{
				case VerticalControl::VS:	return afcs::PitchMode::VS;
				case VerticalControl::FPA:	return afcs::PitchMode::FPA;
			}
			break;
		case PitchMode::VNAV_PTH:	return afcs::PitchMode::VNAVPath;
		case PitchMode::GS:			return afcs::PitchMode::GS;
		case PitchMode::FLARE:		return afcs::PitchMode::Flare;
	}

	return { };
}


template<class Target, class Source>
	std::optional<Target>
	AFCS::optional_cast (std::optional<Source> const& source)
	{
		if (!source)
			return { };
		else
			return static_cast<Target> (*source);
	}


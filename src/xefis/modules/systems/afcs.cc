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

// Standard:
#include <cstddef>

// Qt:
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLayout>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/numeric.h>

// Local:
#include "afcs.h"
#include "afcs_api.h"


constexpr xf::Range<si::Velocity>	AFCS::kSpeedRange;
constexpr xf::Range<double>			AFCS::kMachRange;
constexpr double					AFCS::kMachStep;
constexpr xf::Range<si::Length>		AFCS::kAltitudeRange;
constexpr si::Velocity				AFCS::kVSStep;
constexpr xf::Range<si::Velocity>	AFCS::kVSRange;
constexpr si::Angle					AFCS::kFPAStep;
constexpr xf::Range<si::Angle>		AFCS::kFPARange;


AFCS::AFCS (std::string const& instance):
	Module (instance)
{
	make_button_action (input_button_ap, &AFCS::button_press_ap);
	make_button_action (input_button_at, &AFCS::button_press_at);
	make_button_action (input_button_yd, &AFCS::button_press_yd);
	make_button_action (input_button_xchg_ias_mach, &AFCS::button_press_xchg_ias_mach);
	make_button_action (input_button_toga, &AFCS::button_press_toga);
	make_button_action (input_button_spd_sel, &AFCS::button_press_spd_sel);
	make_button_action (input_button_spd_hold, &AFCS::button_press_spd_hold);
	make_button_action (input_button_xchg_heading_step, &AFCS::button_press_xchg_heading_step);
	make_button_action (input_button_xchg_hdg_trk, &AFCS::button_press_xchg_hdg_trk);
	make_button_action (input_button_hdgtrk_sel, &AFCS::button_press_hdgtrk_sel);
	make_button_action (input_button_hdgtrk_hold, &AFCS::button_press_hdgtrk_hold);
	make_button_action (input_button_wng_lvl, &AFCS::button_press_wng_lvl);
	make_button_action (input_button_loc, &AFCS::button_press_loc);
	make_button_action (input_button_lnav, &AFCS::button_press_lnav);
	make_button_action (input_button_vnav, &AFCS::button_press_vnav);
	make_button_action (input_button_lvl_all, &AFCS::button_press_lvl_all);
	make_button_action (input_button_to, &AFCS::button_press_to);
	make_button_action (input_button_crz, &AFCS::button_press_crz);
	make_button_action (input_button_app, &AFCS::button_press_app);
	make_button_action (input_button_ils, &AFCS::button_press_ils);
	make_button_action (input_button_xchg_altitude_step, &AFCS::button_press_xchg_altitude_step);
	make_button_action (input_button_flch, &AFCS::button_press_flch);
	make_button_action (input_button_altitude_hold, &AFCS::button_press_altitude_hold);
	make_button_action (input_button_gs, &AFCS::button_press_gs);
	make_button_action (input_button_xchg_vs_fpa, &AFCS::button_press_xchg_vs_fpa);
	make_button_action (input_button_vertical_enable, &AFCS::button_press_vertical_enable);
	make_button_action (input_button_vertical_sel, &AFCS::button_press_vertical_sel);
	make_button_action (input_button_clb_con, &AFCS::button_press_clb_con);

	make_knob_action (input_knob_speed, &AFCS::knob_speed_change);
	make_knob_action (input_knob_heading, &AFCS::knob_heading_change);
	make_knob_action (input_knob_altitude, &AFCS::knob_altitude_change);
	make_knob_action (input_knob_vertical, &AFCS::knob_vertical_change);

	solve();
}


void
AFCS::process (x2::Cycle const&)
{
	// TODO recheck
	try {
		for (auto& r: _rotary_decoders)
			(*r)();

		for (auto& a: _button_actions)
			(*a)();

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
			_mcp_ias = xf::clamped (_mcp_ias + 1_kt * delta, kSpeedRange);
			break;

		case SpeedControl::Mach:
			_mcp_mach = xf::clamped (_mcp_mach + kMachStep * delta, kMachRange);
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
			if (input_measured_mach)
				output_cmd_mach = *input_measured_mach;
			break;

		case SpeedControl::Mach:
			_speed_control = SpeedControl::KIAS;
			if (input_measured_ias)
				output_cmd_ias = *input_measured_ias;
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
		output_thr_ref = input_thr_ref_for_toga;
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
	Optional<si::Angle> track_minus_heading;
	if (input_measured_heading_magnetic && input_measured_track_magnetic)
		track_minus_heading = *input_measured_track_magnetic - *input_measured_heading_magnetic;

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
	output_thr_ref = input_thr_ref_for_toga;
	output_spd_ref = input_spd_ref_for_climbout;
}


void
AFCS::button_press_crz()
{
	output_thr_ref = input_thr_ref_for_cruise;
	output_spd_ref = input_spd_ref_for_cruise;
}


void
AFCS::button_press_app()
{
	output_thr_ref = input_thr_ref_for_descent;
	output_spd_ref = input_spd_ref_for_approach;
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
	_mcp_altitude = xf::clamped (_mcp_altitude + altitude_step * delta, kAltitudeRange);
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
			_mcp_vs = xf::clamped (*_mcp_vs + kVSStep * delta, kVSRange);

			// Disengage on 0 crossing:
			if (xf::Range<si::Velocity> ({ -0.5 * kVSStep, 0.5 * kVSStep }).includes (*_mcp_vs))
			{
				_mcp_vs.reset();
				perhaps_alt_hold();
			}
			break;

		case VerticalControl::FPA:
			if (!_mcp_fpa)
				_mcp_fpa = 0_deg;
			_mcp_fpa = xf::clamped (*_mcp_fpa + kFPAStep * delta, kFPARange);

			// Disengage on 0 crossing:
			if (xf::Range<si::Angle> ({ -0.5 * kFPAStep, +0.5 * kFPAStep }).includes (*_mcp_fpa))
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
	output_thr_ref = input_thr_ref_for_cont;
	output_spd_ref = input_spd_ref_for_climbout;
}


void
AFCS::check_input()
{
	std::array<x2::BasicProperty*, 7> checked_props = { {
		&input_measured_ias,
		&input_measured_mach,
		&input_measured_heading_magnetic,
		&input_measured_track_magnetic,
		&input_measured_altitude_amsl,
		&input_measured_vs,
		&input_measured_fpa,
	} };

	if (std::any_of (checked_props.begin(), checked_props.end(), [](x2::BasicProperty* p) { return !p->valid(); }))
	{
		QStringList failed_props;
		for (auto const& p: checked_props)
			if (!p->valid())
				failed_props.push_back (QString::fromStdString (p->path().string()));
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
	output_mcp_led_ap = _ap_on;
	output_mcp_led_at = _at_on;
	output_mcp_led_yd = _yd_on;

	// Speed window:
	switch (_speed_control)
	{
		case SpeedControl::KIAS:
			output_mcp_speed_format_out = *setting_mcp_speed_format_kias;
			output_mcp_speed_display = xf::symmetric_round (_mcp_ias.quantity<Knot>());
			break;

		case SpeedControl::Mach:
			output_mcp_speed_format_out = *setting_mcp_speed_format_mach;
			output_mcp_speed_display = _mcp_mach;
			break;
	}

	// Heading window:
	output_mcp_heading_format_out = *setting_mcp_heading_format;
	int lateral_angle = 0;
	switch (_lateral_control)
	{
		case LateralControl::Heading:
			lateral_angle = xf::symmetric_round (_mcp_heading.quantity<Degree>());
			break;

		case LateralControl::Track:
			lateral_angle = xf::symmetric_round (_mcp_track.quantity<Degree>());
			break;
	}
	if (lateral_angle == 0)
		lateral_angle = 360;
	output_mcp_heading_display = lateral_angle;

	// Altitude window:
	output_mcp_altitude_format_out = *setting_mcp_altitude_format;
	output_mcp_altitude_display = xf::symmetric_round (_mcp_altitude.quantity<Foot>());

	// Vertical-control window:
	switch (_vertical_control)
	{
		case VerticalControl::VS:
			output_mcp_vertical_format_out = *setting_mcp_vertical_format_vs;
			if (_mcp_vs)
				output_mcp_vertical_display = xf::symmetric_round (_mcp_vs->quantity<FootPerMinute>());
			else
				output_mcp_vertical_display.set_nil();
			break;

		case VerticalControl::FPA:
			output_mcp_vertical_format_out = *setting_mcp_vertical_format_fpa;
			if (_mcp_fpa)
				output_mcp_vertical_display = xf::symmetric_round (10.0 * _mcp_fpa->quantity<Degree>()) / 10.0;
			else
				output_mcp_vertical_display.set_nil();
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
			output_fma_speed_hint = "";
			break;

		case ThrustMode::TO_GA:
			output_fma_speed_hint = "TO/GA";
			break;

		case ThrustMode::CONT:
			output_fma_speed_hint = "CONT";
			break;

		case ThrustMode::IDLE:
			output_fma_speed_hint = "IDLE";
			break;

		case ThrustMode::MCP_SPD:
			switch (_speed_control)
			{
				case SpeedControl::KIAS:
					if (input_measured_ias)
					{
						if (abs (*input_measured_ias - _mcp_ias) < *setting_acq_delta_ias)
							output_fma_speed_hint = "MCP SPD";
						else
							output_fma_speed_hint = "SPD";
					}
					break;

				case SpeedControl::Mach:
					if (input_measured_mach)
					{
						if (abs (*input_measured_mach - _mcp_mach) < *setting_acq_delta_mach)
							output_fma_speed_hint = "MCP SPD";
						else
							output_fma_speed_hint = "SPD";
					}
			}
			break;

		case ThrustMode::SPD_HOLD:
			output_fma_speed_hint = "SPD HOLD";
			break;

		default:
			output_fma_speed_hint = "X";
	}

	switch (_roll_mode)
	{
		case RollMode::None:
			output_fma_roll_hint = "";
			break;

		case RollMode::MCP:
			switch (_lateral_control)
			{
				case LateralControl::Heading:
					if (input_measured_heading_magnetic)
					{
						if (abs (*input_measured_heading_magnetic - _mcp_heading) < *setting_acq_delta_heading)
							output_fma_roll_hint = "HDG";
						else
							output_fma_roll_hint = "HDG SEL";
					}
					break;

				case LateralControl::Track:
					if (input_measured_track_magnetic)
					{
						if (abs (*input_measured_track_magnetic - _mcp_track) < *setting_acq_delta_heading)
							output_fma_roll_hint = "TRK";
						else
							output_fma_roll_hint = "TRK SEL";
					}
					break;
			}
			break;

		case RollMode::HOLD:
			switch (_lateral_control)
			{
				case LateralControl::Heading:
					output_fma_roll_hint = "HDG HOLD";
					break;

				case LateralControl::Track:
					output_fma_roll_hint = "TRK HOLD";
					break;
			}
			break;

		case RollMode::WNG_LVL:
			output_fma_roll_hint = "WNG LVL";
			break;

		case RollMode::LOC:
			output_fma_roll_hint = "LOC";
			break;

		case RollMode::LNAV:
			output_fma_roll_hint = "LNAV";
			break;

		default:
			output_fma_roll_hint = "X";
			break;
	}

	switch (_armed_roll_mode)
	{
		case RollMode::None:
			output_fma_roll_armed_hint = "";
			break;

		case RollMode::LOC:
			output_fma_roll_armed_hint = "LOC";
			break;

		default:
			output_fma_roll_armed_hint = "X";
	}

	switch (_pitch_mode)
	{
		case PitchMode::None:
			output_fma_pitch_hint = "";
			break;

		case PitchMode::MCP_SPD:
			output_fma_pitch_hint = "SPD";
			break;

		case PitchMode::ALT_HOLD:
			output_fma_pitch_hint = "ALT HOLD";
			break;

		case PitchMode::MCP_ALT:
			if (input_measured_altitude_amsl)
			{
				if (abs (*input_measured_altitude_amsl - *output_cmd_altitude) <= *setting_acq_delta_altitude)
					output_fma_pitch_hint = "ALT";
				else
				{
					if (output_cmd_vs)
						output_fma_pitch_hint = "FLCH V/S";
					else if (output_cmd_fpa)
						output_fma_pitch_hint = "FLCH FPA";
					else
						output_fma_pitch_hint = "FLCH";
				}
			}
			break;

		case PitchMode::VC:
			switch (_vertical_control)
			{
				case VerticalControl::VS:
					output_fma_pitch_hint = "V/S";
					break;

				case VerticalControl::FPA:
					output_fma_pitch_hint = "FPA";
					break;
			}
			break;

		case PitchMode::VNAV_PTH:
			output_fma_pitch_hint = "VNAV PTH";
			break;

		case PitchMode::GS:
			output_fma_pitch_hint = "G/S";
			break;

		case PitchMode::FLARE:
			output_fma_pitch_hint = "FLARE";
			break;

		default:
			output_fma_pitch_hint = "X";
			break;
	}

	switch (_armed_pitch_mode)
	{
		case PitchMode::None:
			output_fma_pitch_armed_hint = "";
			break;

		case PitchMode::GS:
			output_fma_pitch_armed_hint = "G/S";
			break;

		default:
			output_fma_pitch_armed_hint = "X";
	}

	if (_ap_on)
		output_fma_hint = "A/P";
	else
		output_fma_hint = "F/D";
}


void
AFCS::update_output()
{
	// Modes:
	output_cmd_thrust_mode = optional_cast<int64_t> (translate_thrust_mode());
	output_cmd_roll_mode = optional_cast<int64_t> (translate_roll_mode());
	output_cmd_pitch_mode = optional_cast<int64_t> (translate_pitch_mode());

	// Settings:

	if (_thrust_mode != ThrustMode::SPD_HOLD)
	{
		output_cmd_ias = _mcp_ias;
		output_cmd_mach = _mcp_mach;
	}

	if (_roll_mode != RollMode::HOLD)
	{
		output_cmd_heading_magnetic = _mcp_heading;
		output_cmd_track_magnetic = _mcp_track;

		output_cmd_use_trk = (_lateral_control == LateralControl::Track);
	}

	if (_pitch_mode != PitchMode::ALT_HOLD)
		output_cmd_altitude = _mcp_altitude;

	output_cmd_vs = _mcp_vs;
	output_cmd_fpa = _mcp_fpa;
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
			if (input_measured_ias)
				output_cmd_ias = *input_measured_ias;
			break;

		case SpeedControl::Mach:
			if (input_measured_mach)
				output_cmd_mach = *input_measured_mach;
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
			if (input_measured_heading_magnetic)
				output_cmd_heading_magnetic = *input_measured_heading_magnetic;
			break;

		case LateralControl::Track:
			if (input_measured_track_magnetic)
				output_cmd_track_magnetic = *input_measured_track_magnetic;
			break;
	}
}


void
AFCS::alt_hold_with_pitch()
{
	_pitch_mode = PitchMode::ALT_HOLD;

	if (input_measured_altitude_amsl)
		output_cmd_altitude = *input_measured_altitude_amsl;
}


void
AFCS::xchg_modes (PitchMode a, PitchMode b)
{
	if (_pitch_mode == a)
		_pitch_mode = b;
	else if (_pitch_mode == b)
		_pitch_mode = a;
}


inline Optional<si::Velocity>
AFCS::current_rounded_vs() const
{
	if (input_measured_vs)
		return std::round (*input_measured_vs / *setting_vs_rounding) * *setting_vs_rounding;

	return { };
}


inline Optional<si::Angle>
AFCS::current_rounded_fpa() const
{
	if (input_measured_fpa)
		return std::round (*input_measured_fpa / *setting_fpa_rounding) * *setting_fpa_rounding;

	return { };
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
AFCS::make_button_action (x2::PropertyIn<bool>& property, void (AFCS::* callback)())
{
	auto action = std::make_unique<x2::PropChangedToAction<bool>> (property, true, [this,callback] {
		try {
			(this->*callback)();
			solve();
		}
		catch (...)
		{
			solve();
			throw;
		}
	});

	_button_actions.insert (std::move (action));
}


void
AFCS::make_knob_action (x2::PropertyIn<int64_t>& property, void (AFCS::* callback)(int))
{
	auto action = std::make_unique<x2::DeltaDecoder> (property, [this,callback](int delta) {
		try {
			(this->*callback) (delta);
			solve();
		}
		catch (...)
		{
			solve();
			throw;
		}
	});

	// Initialize:
	action->force_callback (0);
	_rotary_decoders.insert (std::move (action));
}


Optional<afcs_api::ThrustMode>
AFCS::translate_thrust_mode() const
{
	switch (_thrust_mode)
	{
		case ThrustMode::None:		return afcs_api::ThrustMode::None;
		case ThrustMode::TO_GA:		return afcs_api::ThrustMode::TO_GA;
		case ThrustMode::CONT:		return afcs_api::ThrustMode::Continuous;
		case ThrustMode::IDLE:		return afcs_api::ThrustMode::Idle;
		case ThrustMode::MCP_SPD:
		case ThrustMode::SPD_HOLD:
			switch (_speed_control)
			{
				case SpeedControl::KIAS:	return afcs_api::ThrustMode::KIAS;
				case SpeedControl::Mach:	return afcs_api::ThrustMode::Mach;
			}
			break;
	};

	return { };
}


Optional<afcs_api::RollMode>
AFCS::translate_roll_mode() const
{
	switch (_roll_mode)
	{
		case RollMode::None:		return afcs_api::RollMode::None;
		case RollMode::MCP:
		case RollMode::HOLD:
			switch (_lateral_control)
			{
				case LateralControl::Heading:	return afcs_api::RollMode::Heading;
				case LateralControl::Track:		return afcs_api::RollMode::Track;
			}
			break;
		case RollMode::WNG_LVL:		return afcs_api::RollMode::WingsLevel;
		case RollMode::LOC:			return afcs_api::RollMode::Localizer;
		case RollMode::LNAV:		return afcs_api::RollMode::LNAV;
	}

	return { };
}


Optional<afcs_api::PitchMode>
AFCS::translate_pitch_mode() const
{
	switch (_pitch_mode)
	{
		case PitchMode::None:		return afcs_api::PitchMode::None;
		case PitchMode::MCP_SPD:
			switch (_speed_control)
			{
				case SpeedControl::KIAS:	return afcs_api::PitchMode::KIAS;
				case SpeedControl::Mach:	return afcs_api::PitchMode::Mach;
			}
			break;
		case PitchMode::ALT_HOLD:	return afcs_api::PitchMode::Altitude;
		case PitchMode::MCP_ALT:
			if (output_cmd_vs)
				return afcs_api::PitchMode::VS;
			else if (output_cmd_fpa)
				return afcs_api::PitchMode::FPA;
			else
				return afcs_api::PitchMode::Altitude;
		case PitchMode::VC:
			switch (_vertical_control)
			{
				case VerticalControl::VS:	return afcs_api::PitchMode::VS;
				case VerticalControl::FPA:	return afcs_api::PitchMode::FPA;
			}
			break;
		case PitchMode::VNAV_PTH:	return afcs_api::PitchMode::VNAVPath;
		case PitchMode::GS:			return afcs_api::PitchMode::GS;
		case PitchMode::FLARE:		return afcs_api::PitchMode::Flare;
	}

	return { };
}


template<class Target, class Source>
	Optional<Target>
	AFCS::optional_cast (Optional<Source> const& source)
	{
		if (!source)
			return { };
		else
			return static_cast<Target> (*source);
	}


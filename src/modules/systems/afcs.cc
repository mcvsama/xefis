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


constexpr Angle				AFCS::HeadingHoldPitchLimit;
constexpr Angle				AFCS::AltitudeHoldRollLimit;
constexpr xf::Range<Speed>	AFCS::SpeedRange;
constexpr xf::Range<Length>	AFCS::AltitudeRange;
constexpr Speed				AFCS::VSpdStep;
constexpr xf::Range<Speed>	AFCS::VSpdRange;
constexpr Angle				AFCS::FPAStep;
constexpr xf::Range<Angle>	AFCS::FPARange;




AFCS::AFCS (xf::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	parse_settings (config, {
		{ "default.speed", _ias_counter, true },
		{ "default.heading", _heading_counter, true },
		{ "default.altitude", _altitude_counter, true },
		{ "altitude-hold-threshold-vs", _altitude_hold_threshold_vs, true },
	});

	parse_properties (config, {
		{ "button.ap", _mcp_ap_button, true },
		{ "button.at", _mcp_at_button, true },
		{ "button.yd", _mcp_yd_button, true },
		{ "button.speed.ias-mach", _mcp_speed_ias_mach_button, true },
		{ "button.speed.sel", _mcp_speed_sel_button, true },
		{ "button.speed.hold", _mcp_speed_hold_button, true },
		{ "button.heading.hdg-trk", _mcp_heading_hdg_trk_button, true },
		{ "button.heading.sel", _mcp_heading_sel_button, true },
		{ "button.heading.hold", _mcp_heading_hold_button, true },
		{ "button.vnav", _mcp_vnav_button, true },
		{ "button.lnav", _mcp_lnav_button, true },
		{ "button.app", _mcp_app_button, true },
		{ "button.altitude.stepch", _mcp_altitude_stepch_button, true },
		{ "button.altitude.flch", _mcp_altitude_flch_button, true },
		{ "button.altitude.hold", _mcp_altitude_hold_button, true },
		{ "button.vertical-speed.vs-fpa", _mcp_vspd_vs_fpa_button, true },
		{ "button.vertical-speed.sel", _mcp_vspd_sel_button, true },
		{ "button.vertical-speed.clb-con", _mcp_vspd_clb_con_button, true },
		{ "rotary-encoder.speed", _mcp_speed_knob, true },
		{ "rotary-encoder.heading", _mcp_heading_knob, true },
		{ "rotary-encoder.altitude", _mcp_altitude_knob, true },
		{ "rotary-encoder.vertical-speed", _mcp_vspd_knob, true },
		{ "display.speed", _mcp_speed_display, true },
		{ "display.heading", _mcp_heading_display, true },
		{ "display.altitude", _mcp_altitude_display, true },
		{ "display.vertical-speed", _mcp_vspd_display, true },
		{ "led.ap", _mcp_ap_led, true },
		{ "led.at", _mcp_at_led, true },
		{ "led.yd", _mcp_yd_led, true },
		{ "led.speed.sel", _mcp_speed_sel_led, true },
		{ "led.speed.hold", _mcp_speed_hold_led, true },
		{ "led.heading.sel", _mcp_heading_sel_led, true },
		{ "led.heading.hold", _mcp_heading_hold_led, true },
		{ "led.vnav", _mcp_vnav_led, true },
		{ "led.lnav", _mcp_lnav_led, true },
		{ "led.app", _mcp_app_led, true },
		{ "led.altitude.flch", _mcp_altitude_flch_led, true },
		{ "led.altitude.hold", _mcp_altitude_hold_led, true },
		{ "led.vertical-speed.sel", _mcp_vspd_sel_led, true },
		{ "led.vertical-speed.clb-con", _mcp_vspd_clb_con_led, true },
		{ "input.vertical-speed", _measured_vs, true },
		{ "input.altitude-amsl", _measured_altitude_amsl, true },
		{ "input.heading.magnetic", _measured_magnetic_heading, true },
		{ "input.track.lateral.magnetic", _measured_magnetic_track, true },
		{ "input.track.vertical", _measured_vertical_track, true },
		{ "input.thr-ref-for-cruise", _thr_ref_for_cruise, true },
		{ "input.thr-ref-for-climb", _thr_ref_for_climb, true },
		{ "input.thr-ref-for-descent", _thr_ref_for_descent, true },
		{ "input.spd-ref-for-cruise", _spd_ref_for_cruise, true },
		{ "input.spd-ref-for-approach", _spd_ref_for_approach, true },
		{ "output.roll-mode", _cmd_roll_mode, true },
		{ "output.pitch-mode", _cmd_pitch_mode, true },
		{ "output.ias", _cmd_ias, true },
		{ "output.mach", _cmd_mach, true },
		{ "output.magnetic-heading-track", _cmd_magnetic_heading_track, true },
		{ "output.altitude", _cmd_altitude, true },
		{ "output.vertical-speed", _cmd_vs, true },
		{ "output.fpa", _cmd_fpa, true },
		{ "output.thr-ref", _thr_ref, true },
		{ "output.spd-ref", _spd_ref, true },
		{ "output.flight-mode-hint", _flight_mode_hint, true },
		{ "output.flight-mode-speed-hint", _flight_mode_speed_hint, true },
		{ "output.flight-mode-roll-hint", _flight_mode_roll_hint, true },
		{ "output.flight-mode-pitch-hint", _flight_mode_pitch_hint, true },
		{ "output.yaw-damper-enabled", _yaw_damper_enabled, true },
	});

	_mcp_ap_action = make_button_action (_mcp_ap_button, &AFCS::button_press_ap);
	_mcp_at_action = make_button_action (_mcp_at_button, &AFCS::button_press_at);
	_mcp_yd_action = make_button_action (_mcp_yd_button, &AFCS::button_press_yd);
	_mcp_speed_ias_mach_action = make_button_action (_mcp_speed_ias_mach_button, &AFCS::button_press_speed_ias_mach);
	_mcp_speed_sel_action = make_button_action (_mcp_speed_sel_button, &AFCS::button_press_speed_sel);
	_mcp_speed_hold_action = make_button_action (_mcp_speed_hold_button, &AFCS::button_press_speed_hold);
	_mcp_heading_hdg_trk_action = make_button_action (_mcp_heading_hdg_trk_button, &AFCS::button_press_heading_hdg_trk);
	_mcp_heading_sel_action = make_button_action (_mcp_heading_sel_button, &AFCS::button_press_heading_sel);
	_mcp_heading_hold_action = make_button_action (_mcp_heading_hold_button, &AFCS::button_press_heading_hold);
	_mcp_vnav_action = make_button_action (_mcp_vnav_button, &AFCS::button_press_vnav);
	_mcp_lnav_action = make_button_action (_mcp_lnav_button, &AFCS::button_press_lnav);
	_mcp_app_action = make_button_action (_mcp_app_button, &AFCS::button_press_app);
	_mcp_altitude_stepch_action = make_button_action (_mcp_altitude_stepch_button, &AFCS::button_press_altitude_stepch);
	_mcp_altitude_flch_action = make_button_action (_mcp_altitude_flch_button, &AFCS::button_press_altitude_flch);
	_mcp_altitude_hold_action = make_button_action (_mcp_altitude_hold_button, &AFCS::button_press_altitude_hold);
	_mcp_vspd_vs_fpa_action = make_button_action (_mcp_vspd_vs_fpa_button, &AFCS::button_press_vspd_vs_fpa);
	_mcp_vspd_sel_action = make_button_action (_mcp_vspd_sel_button, &AFCS::button_press_vspd_sel);
	_mcp_vspd_clb_con_action = make_button_action (_mcp_vspd_clb_con_button, &AFCS::button_press_vspd_clb_con);

	_button_actions = {
		&*_mcp_ap_action,
		&*_mcp_at_action,
		&*_mcp_yd_action,
		&*_mcp_speed_ias_mach_action,
		&*_mcp_speed_sel_action,
		&*_mcp_speed_hold_action,
		&*_mcp_heading_hdg_trk_action,
		&*_mcp_heading_sel_action,
		&*_mcp_heading_hold_action,
		&*_mcp_vnav_action,
		&*_mcp_lnav_action,
		&*_mcp_app_action,
		&*_mcp_altitude_stepch_action,
		&*_mcp_altitude_flch_action,
		&*_mcp_altitude_hold_action,
		&*_mcp_vspd_vs_fpa_action,
		&*_mcp_vspd_sel_action,
		&*_mcp_vspd_clb_con_action,
	};

	_mcp_speed_decoder = make_knob_action (_mcp_speed_knob, &AFCS::knob_speed);
	_mcp_heading_decoder = make_knob_action (_mcp_heading_knob, &AFCS::knob_heading);
	_mcp_altitude_decoder = make_knob_action (_mcp_altitude_knob, &AFCS::knob_altitude);
	_mcp_vspd_decoder = make_knob_action (_mcp_vspd_knob, &AFCS::knob_vspd);

	_rotary_decoders = {
		_mcp_speed_decoder.get(),
		_mcp_heading_decoder.get(),
		_mcp_altitude_decoder.get(),
		_mcp_vspd_decoder.get(),
	};

	for (auto* decoder: _rotary_decoders)
		decoder->call (0);

	solve_mode();
}


void
AFCS::data_updated()
{
	// TODO If any observed input is nil, disengage!

	try {
		for (auto r: _rotary_decoders)
			r->data_updated();

		for (auto a: _button_actions)
			a->data_updated();
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
	catch (InvalidState const&)
	{
		std::clog << "AFCS was in invalid state" << std::endl;
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
AFCS::button_press_speed_ias_mach()
{
	switch (_speed_units)
	{
		case SpeedUnits::KIAS:
			// TODO update cmd?
			_speed_units = SpeedUnits::Mach;
			break;

		case SpeedUnits::Mach:
			// TODO update mach?
			_speed_units = SpeedUnits::KIAS;
			break;
	}
}


void
AFCS::apply_pitch_changes_for_airspeed_via_at()
{
	auto ensure_valid_vs = [&] {
		if (!_measured_vs.valid())
			throw DisengageAP ("invalid measured v/s");
	};

	auto ensure_valid_alt = [&] {
		if (!_measured_altitude_amsl.valid())
			throw DisengageAP ("invalid measured altitude amsl");
	};

	switch (_pitch_mode)
	{
		case PitchMode::FLCH_SPD:
			ensure_valid_vs();

			if (std::abs (*_measured_vs) <= _altitude_hold_threshold_vs)
			{
				ensure_valid_alt();

				_pitch_mode = PitchMode::ALT_HOLD;
				_hold_altitude_amsl = *_measured_altitude_amsl;
			}
			else
			{
				_pitch_mode = PitchMode::FLCH_VS;
				_vspd_counter = current_rounded_vs();
			}
			break;

		case PitchMode::CLB_CON:
			ensure_valid_vs();

			if (std::abs (*_measured_vs) <= _altitude_hold_threshold_vs)
			{
				ensure_valid_alt();

				_pitch_mode = PitchMode::ALT_HOLD;
				_hold_altitude_amsl = *_measured_altitude_amsl;
			}
			else
			{
				_pitch_mode = PitchMode::VS;
				_vspd_counter = current_rounded_vs();
			}
			break;

		case PitchMode::VNAV_SPD:
			ensure_valid_alt();

			_pitch_mode = PitchMode::ALT_HOLD;
			_hold_altitude_amsl = *_measured_altitude_amsl;
			break;

		default:
			// No changes
			break;
	}
}


void
AFCS::button_press_speed_sel()
{
	apply_pitch_changes_for_airspeed_via_at();
	_speed_mode = SpeedMode::SPD_SEL;
}


void
AFCS::button_press_speed_hold()
{
	apply_pitch_changes_for_airspeed_via_at();
	_speed_mode = SpeedMode::HOLD;
}


void
AFCS::button_press_heading_hdg_trk()
{
	if (!_measured_magnetic_heading.valid())
		throw DisengageAP ("invalid measured magnetic heading");
	if (!_measured_magnetic_track.valid())
		throw DisengageAP ("invlid measured magnetic track");

	Angle track_minus_heading = *_measured_magnetic_track - *_measured_magnetic_heading;

	switch (_lateral_direction)
	{
		case LateralDirection::Heading:
			_lateral_direction = LateralDirection::Track;
			// Convert heading to track:
			_heading_counter += track_minus_heading;
			break;

		case LateralDirection::Track:
			_lateral_direction = LateralDirection::Heading;
			// Convert track to heading:
			_heading_counter -= track_minus_heading;
			break;
	}
}


void
AFCS::button_press_heading_sel()
{
	if (_roll_mode == RollMode::HDG_SEL ||
		_roll_mode == RollMode::HDG ||
		_roll_mode == RollMode::TRK_SEL ||
		_roll_mode == RollMode::TRK)
	{
		_roll_mode = RollMode::WNG_LVL;
	}
	else
	{
		switch (_lateral_direction)
		{
			case LateralDirection::Heading:
				_roll_mode = RollMode::HDG_SEL;
				break;

			case LateralDirection::Track:
				_roll_mode = RollMode::TRK_SEL;
				break;
		}
	}
}


void
AFCS::button_press_heading_hold()
{
	switch (_lateral_direction)
	{
		case LateralDirection::Heading:
			if (!_measured_magnetic_heading.valid())
				throw DisengageAP ("invalid measured magnetic heading");

			_roll_mode = RollMode::HDG_HOLD;
			_hold_magnetic_heading_or_track = *_measured_magnetic_heading;
			break;

		case LateralDirection::Track:
			if (!_measured_magnetic_track.valid())
				throw DisengageAP ("invlid measured magnetic track");

			_roll_mode = RollMode::TRK_HOLD;
			_hold_magnetic_heading_or_track = *_measured_magnetic_track;
			break;
	}
}


void
AFCS::button_press_vnav()
{
	// TODO
}


void
AFCS::button_press_lnav()
{
	if (_roll_mode != RollMode::LNAV)
		_roll_mode = RollMode::LNAV;
	else
	{
		switch (_lateral_direction)
		{
			case LateralDirection::Heading:
				if (!_measured_magnetic_heading.valid())
					throw DisengageAP ("invalid measured magnetic heading");

				_roll_mode = RollMode::HDG_HOLD;
				_hold_magnetic_heading_or_track = *_measured_magnetic_heading;
				break;

			case LateralDirection::Track:
				if (!_measured_magnetic_heading.valid())
					throw DisengageAP ("invalid measured magnetic heading");

				_roll_mode = RollMode::TRK_HOLD;
				_hold_magnetic_heading_or_track = *_measured_magnetic_heading;
				break;
		}
	}
}


void
AFCS::button_press_app()
{
	// TODO
}


void
AFCS::button_press_altitude_stepch()
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
AFCS::button_press_altitude_flch()
{
	if (_pitch_mode != PitchMode::FLCH_SPD)
	{
		if (!_measured_altitude_amsl.valid())
			throw DisengageAP ("invalid measured altitude amsl");

		if (_altitude_counter > *_measured_altitude_amsl)
		{
			if (!_thr_ref_for_climb.valid())
				throw DisengageAP ("invalid thr ref for climb");

			_thr_ref = *_thr_ref_for_climb;
			_speed_mode = SpeedMode::THR_REF;
		}
		else
		{
			if (!_thr_ref_for_descent.valid())
				throw DisengageAP ("invalid thr ref for descent");

			_thr_ref = *_thr_ref_for_descent;
			_speed_mode = SpeedMode::IDLE;
		}
		_pitch_mode = PitchMode::FLCH_SPD;
	}
	else
	{
		if (!_measured_altitude_amsl.valid())
			throw DisengageAP ("invalid measured altitude amsl");

		_thr_ref = _thr_ref_for_cruise;
		_speed_mode = SpeedMode::THR_REF;
		_pitch_mode = PitchMode::ALT_HOLD;
		_hold_altitude_amsl = *_measured_altitude_amsl;
	}
}


void
AFCS::button_press_altitude_hold()
{
	if (!_measured_altitude_amsl.valid())
		throw DisengageAP ("invalid measured altitude amsl");
	if (!_thr_ref_for_cruise.valid())
		throw DisengageAP ("invalid thr ref for cruise");
	if (!_spd_ref_for_cruise.valid())
		throw DisengageAP ("invalid spd ref for cruise");

	_pitch_mode = PitchMode::ALT_HOLD;
	_hold_altitude_amsl = *_measured_altitude_amsl;
	_thr_ref = _thr_ref_for_cruise;
	_spd_ref = _spd_ref_for_cruise;

	switch (_speed_mode)
	{
		case SpeedMode::TO_GA:
		case SpeedMode::THR:
		case SpeedMode::IDLE:
			_speed_mode = SpeedMode::SPD_REF;
			break;

		default:
			break;
	}
}


void
AFCS::button_press_vspd_vs_fpa()
{
	// TODO change FLCH_VS <-> FLCH_FPA
	// TODO change VS <-> FPA
	switch (_pitch_units)
	{
		case PitchUnits::VS:
			if (!_measured_vertical_track.valid())
				throw DisengageAP ("invalid measured vertical track");

			_cmd_fpa = *_measured_vertical_track;
			_pitch_units = PitchUnits::FPA;
			break;

		case PitchUnits::FPA:
			if (!_measured_vs.valid())
				throw DisengageAP ("invalid measured v/s");

			_cmd_vs = current_rounded_vs();
			_pitch_units = PitchUnits::VS;
			break;
	}
}


void
AFCS::button_press_vspd_sel()
{
	// Pitch will control V/S, so A/T has to control airspeed:
	if (at_in_thrust_mode())
	{
		set_spd_ref_for_flight_stage (_flight_stage);
		_speed_mode = SpeedMode::SPD_REF;
	}

	if (flch_engaged())
	{
		switch (_pitch_units)
		{
			case PitchUnits::VS:
				_pitch_mode = PitchMode::FLCH_VS;
				break;

			case PitchUnits::FPA:
				_pitch_mode = PitchMode::FLCH_FPA;
				break;
		}
	}
	else
	{
		switch (_pitch_units)
		{
			case PitchUnits::VS:
				_pitch_mode = PitchMode::VS;
				break;

			case PitchUnits::FPA:
				_pitch_mode = PitchMode::FPA;
				break;
		}
	}
}


void
AFCS::button_press_vspd_clb_con()
{
	// TODO
}


void
AFCS::knob_speed (int delta)
{
	switch (_speed_units)
	{
		case SpeedUnits::KIAS:
			_ias_counter = xf::limit (_ias_counter + 1_kt * delta, SpeedRange);
			break;

		case SpeedUnits::Mach:
			_mach_counter = 0.0;// TODO
			// TODO +- mach unit
			break;
	}
}


void
AFCS::knob_heading (int delta)
{
	_heading_counter = xf::floored_mod (_heading_counter + 1_deg * delta, 360_deg);
}


void
AFCS::knob_altitude (int delta)
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
	_altitude_counter = xf::limit (_altitude_counter + altitude_step * delta, AltitudeRange);
}


void
AFCS::knob_vspd (int delta)
{
	switch (_pitch_units)
	{
		case PitchUnits::VS:
			_vspd_counter = xf::limit (_vspd_counter + VSpdStep * delta, VSpdRange);

			// Disengage on 0 crossing:
			if (xf::Range<Speed> ({ -1_fpm, +1_fpm }).includes (_vspd_counter))
			{
				_vspd_counter = 0_fpm;

				if (_pitch_mode == PitchMode::FLCH_VS ||
					_pitch_mode == PitchMode::VS ||
					_pitch_mode == PitchMode::FLCH_FPA ||
					_pitch_mode == PitchMode::FPA)
				{
					_pitch_mode = PitchMode::ALT_HOLD;
					_hold_altitude_amsl = *_measured_altitude_amsl;
				}
			}
			break;

		case PitchUnits::FPA:
			_fpa_counter = xf::limit (_fpa_counter + FPAStep * delta, FPARange);

			// Disengage on 0 crossing:
			if (xf::Range<Angle> ({ -0.1_deg, +0.1_deg }).includes (_fpa_counter))
			{
				_fpa_counter = 0_deg;

				if (_pitch_mode == PitchMode::FLCH_VS ||
					_pitch_mode == PitchMode::VS ||
					_pitch_mode == PitchMode::FLCH_FPA ||
					_pitch_mode == PitchMode::FPA)
				{
					_pitch_mode = PitchMode::ALT_HOLD;
					_hold_altitude_amsl = *_measured_altitude_amsl;
				}
			}
			break;
	}
}


void
AFCS::solve_mode()
{
	// Displays (counters):
	_mcp_ap_led.write (_ap_on);
	_mcp_at_led.write (_at_on);
	_mcp_yd_led.write (_yd_on);
	_mcp_speed_display.write (xf::symmetric_round (_ias_counter.kt()));
	int heading = xf::symmetric_round (_heading_counter.deg());
	if (heading == 0)
		heading = 360;
	_mcp_heading_display.write (heading);
	_mcp_altitude_display.write (xf::symmetric_round (_altitude_counter.ft()));

	switch (_pitch_units)
	{
		case PitchUnits::VS:
			_mcp_vspd_display.write (xf::symmetric_round (_vspd_counter.fpm()));
			break;

		case PitchUnits::FPA:
			_mcp_vspd_display.write (xf::symmetric_round (10.0 * _fpa_counter.deg()) / 10.0);
			break;
	}

	// Speed:
	_mcp_speed_sel_led.write (_speed_mode == SpeedMode::SPD_SEL);
	_mcp_speed_hold_led.write (_speed_mode == SpeedMode::HOLD);

	// Control A/T and FD modules:
	_cmd_ias.write (_ias_counter);
	_cmd_magnetic_heading_track.write (_heading_counter);
	_cmd_altitude.write (_altitude_counter);
	// TODO only if accplicable
	_cmd_vs.write (_vspd_counter);
	_cmd_fpa.write (_fpa_counter);

	// Other modules:
	_yaw_damper_enabled = _yd_on;
	// TODO

	update_efis();
}


void
AFCS::update_efis()
{
	switch (_speed_mode)
	{
		case SpeedMode::None:
			_flight_mode_speed_hint = "";
			break;

		case SpeedMode::TO_GA:
			_flight_mode_speed_hint = "TO/GA";
			break;

		case SpeedMode::THR:
			_flight_mode_speed_hint = "THR";
			break;

		case SpeedMode::THR_REF:
			_flight_mode_speed_hint = "THR REF";
			break;

		case SpeedMode::IDLE:
			_flight_mode_speed_hint = "IDLE";
			break;

		case SpeedMode::SPD_REF:
			_flight_mode_speed_hint = "SPD REF";
			break;

		case SpeedMode::SPD_SEL:
			_flight_mode_speed_hint = "SPD SEL";
			break;

		case SpeedMode::MCP_SPD:
			_flight_mode_speed_hint = "MCP SPD";
			break;

		case SpeedMode::HOLD:
			_flight_mode_speed_hint = "HOLD";
			break;

		default:
			_flight_mode_speed_hint = "?";
	}

	switch (_roll_mode)
	{
		case RollMode::None:
			_flight_mode_roll_hint = "";
			break;

		case RollMode::HDG_SEL:
			_flight_mode_roll_hint = "HDG SEL";
			break;

		case RollMode::HDG_HOLD:
			_flight_mode_roll_hint = "HDG HOLD";
			break;

		case RollMode::HDG:
			_flight_mode_roll_hint = "HDG";
			break;

		case RollMode::TRK_SEL:
			_flight_mode_roll_hint = "TRK SEL";
			break;

		case RollMode::TRK_HOLD:
			_flight_mode_roll_hint = "TRK HOLD";
			break;

		case RollMode::TRK:
			_flight_mode_roll_hint = "TRK";
			break;

		case RollMode::LOC:
			_flight_mode_roll_hint = "LOC";
			break;

		case RollMode::WNG_LVL:
			_flight_mode_roll_hint = "WNG LVL";
			break;

		case RollMode::LNAV:
			_flight_mode_roll_hint = "LNAV";
			break;

		default:
			_flight_mode_roll_hint = "?";
			break;
	}

	switch (_pitch_mode)
	{
		case PitchMode::None:
			_flight_mode_pitch_hint = "";
			break;

		case PitchMode::FLCH_SPD:
			_flight_mode_pitch_hint = "FLCH SPD";
			break;

		case PitchMode::FLCH_VS:
			_flight_mode_pitch_hint = "FLCH V/S";
			break;

		case PitchMode::FLCH_FPA:
			_flight_mode_pitch_hint = "FLCH FPA";
			break;

		case PitchMode::VS:
			_flight_mode_pitch_hint = "V/S";
			break;

		case PitchMode::FPA:
			_flight_mode_pitch_hint = "FPA";
			break;

		case PitchMode::CLB_CON:
			_flight_mode_pitch_hint = "CLB/CON";
			break;

		case PitchMode::VNAV_SPD:
			_flight_mode_pitch_hint = "VNAV SPD";
			break;

		case PitchMode::VNAV_PTH:
			_flight_mode_pitch_hint = "VNAV PTH";
			break;

		case PitchMode::ALT:
			_flight_mode_pitch_hint = "ALT";
			break;

		case PitchMode::ALT_HOLD:
			_flight_mode_pitch_hint = "ALT HOLD";
			break;

		case PitchMode::GS:
			_flight_mode_pitch_hint = "G/S";
			break;

		default:
			_flight_mode_pitch_hint = "?";
			break;
	}

	// TODO if engaged, _flight_mode_hint = "FLT DIR" or sth.
	// TODO if error-mode (unknown mode), _flight_mode_hint = "XXX";
}


void
AFCS::disengage_ap (const char* reason)
{
	std::clog << "AFCS A/P disengage: " << reason << std::endl;
	_ap_on = false;
	solve_mode();
}


void
AFCS::disengage_at (const char* reason)
{
	std::clog << "AFCS A/T disengage: " << reason << std::endl;
	_at_on = false;
	solve_mode();
}


void
AFCS::set_spd_ref_for_flight_stage (FlightStage stage)
{
	switch (stage)
	{
		case FlightStage::Cruise:
			_spd_ref = _spd_ref_for_cruise;
			break;

		case FlightStage::Approach:
			_spd_ref = _spd_ref_for_approach;
			break;
	}
}


inline Speed
AFCS::current_rounded_vs() const
{
	if (!_measured_vs.valid())
		throw DisengageAP ("invalid measured v/s");

	int vsr = _vertical_speed_rounding.fpm();
	return 1_fpm * std::round (_measured_vs->fpm() / vsr) * vsr;
}


inline bool
AFCS::at_in_thrust_mode()
{
	switch (_speed_mode)
	{
		case SpeedMode::None:
		case SpeedMode::TO_GA:
		case SpeedMode::THR:
		case SpeedMode::THR_REF:
		case SpeedMode::IDLE:
			return true;

		case SpeedMode::SPD_REF:
		case SpeedMode::SPD_SEL:
		case SpeedMode::MCP_SPD:
		case SpeedMode::HOLD:
			return false;

		case SpeedMode::sentinel:
			_speed_mode = SpeedMode::None;
			throw InvalidState();
	}
}


inline bool
AFCS::at_in_speed_mode()
{
	return !at_in_thrust_mode();
}


inline bool
AFCS::flch_engaged()
{
	switch (_pitch_mode)
	{
		case PitchMode::FLCH_SPD:
		case PitchMode::FLCH_VS:
		case PitchMode::FLCH_FPA:
			return true;

		case PitchMode::None:
		case PitchMode::VS:
		case PitchMode::FPA:
		case PitchMode::CLB_CON:
		case PitchMode::VNAV_SPD:
		case PitchMode::VNAV_PTH:
		case PitchMode::ALT:
		case PitchMode::ALT_HOLD:
		case PitchMode::GS:
			return false;

		case PitchMode::sentinel:
			_pitch_mode = PitchMode::None;
			throw InvalidState();
	}
}


inline Unique<xf::ButtonAction>
AFCS::make_button_action (xf::PropertyBoolean property, void (AFCS::* callback)())
{
	return std::make_unique<xf::ButtonAction> (property, [this,callback] {
		(this->*callback)();
		solve_mode();
	});
}


inline Unique<xf::DeltaDecoder>
AFCS::make_knob_action (xf::PropertyInteger property, void (AFCS::* callback)(int))
{
	return std::make_unique<xf::DeltaDecoder> (property, [this,callback](int delta) {
		(this->*callback) (delta);
		solve_mode();
	});
}


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


XEFIS_REGISTER_MODULE_CLASS ("systems/afcs", AutomatedFlightControlSystem);


constexpr Angle					AutomatedFlightControlSystem::HeadingHoldPitchLimit;
constexpr Angle					AutomatedFlightControlSystem::AltitudeHoldRollLimit;
constexpr Xefis::Range<Speed>	AutomatedFlightControlSystem::CmdSpeedRange;
constexpr Xefis::Range<Length>	AutomatedFlightControlSystem::CmdAltitudeRange;
constexpr Speed					AutomatedFlightControlSystem::CmdVSpdStep;
constexpr Xefis::Range<Speed>	AutomatedFlightControlSystem::CmdVSpdRange;


AutomatedFlightControlSystem::AutomatedFlightControlSystem (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	parse_settings (config, {
		{ "default.speed", _cmd_speed_counter, true },
		{ "default.heading", _cmd_heading_counter, true },
		{ "default.altitude", _cmd_altitude_counter, true },
	});

	parse_properties (config, {
		{ "button.ap", _mcp_ap, true },
		{ "button.at", _mcp_at, true },
		{ "button.prot", _mcp_prot, true },
		{ "button.tac", _mcp_tac, true },
		{ "button.att", _mcp_att, true },
		{ "button.ct", _mcp_ct, true },
		{ "button.speed.ias-mach", _mcp_speed_ias_mach, true },
		{ "button.speed.sel", _mcp_speed_sel, true },
		{ "button.speed.hold", _mcp_speed_hold, true },
		{ "button.heading.hdg-trk", _mcp_heading_hdg_trk, true },
		{ "button.heading.sel", _mcp_heading_sel, true },
		{ "button.heading.hold", _mcp_heading_hold, true },
		{ "button.vnav", _mcp_vnav, true },
		{ "button.lnav", _mcp_lnav, true },
		{ "button.app", _mcp_app, true },
		{ "button.altitude.stepch", _mcp_altitude_stepch, true },
		{ "button.altitude.flch", _mcp_altitude_flch, true },
		{ "button.altitude.hold", _mcp_altitude_hold, true },
		{ "button.vertical-speed.vs-fpa", _mcp_vspd_vs_fpa, true },
		{ "button.vertical-speed.sel", _mcp_vspd_sel, true },
		{ "button.vertical-speed.clb-con", _mcp_vspd_clb_con, true },
		{ "rotary-encoder.speed.a", _mcp_speed_a, true },
		{ "rotary-encoder.speed.b", _mcp_speed_b, true },
		{ "rotary-encoder.heading.a", _mcp_heading_a, true },
		{ "rotary-encoder.heading.b", _mcp_heading_b, true },
		{ "rotary-encoder.altitude.a", _mcp_altitude_a, true },
		{ "rotary-encoder.altitude.b", _mcp_altitude_b, true },
		{ "rotary-encoder.vertical-speed.a", _mcp_vspd_a, true },
		{ "rotary-encoder.vertical-speed.b", _mcp_vspd_b, true },
		{ "display.speed", _mcp_speed_display, true },
		{ "display.heading", _mcp_heading_display, true },
		{ "display.altitude", _mcp_altitude_display, true },
		{ "display.vertical-speed", _mcp_vspd_display, true },
		{ "led.ap", _mcp_ap_led, true },
		{ "led.at", _mcp_at_led, true },
		{ "led.att", _mcp_att_led, true },
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
		{ "fbw.attitude-mode", _fbw_attitude_mode, true },
		{ "fbw.throttle-mode", _fbw_throttle_mode, true },
		{ "cmd.roll-mode", _cmd_roll_mode, true },
		{ "cmd.pitch-mode", _cmd_pitch_mode, true },
		{ "cmd.ias", _cmd_ias, true },
		{ "cmd.magnetic-heading-track", _cmd_heading_track, true },
		{ "cmd.altitude", _cmd_altitude, true },
		{ "cmd.vertical-speed", _cmd_vspd, true },
		{ "cmd.fpa", _cmd_fpa, true },
		{ "fma.control-hint", _fma_control_hint, true },
		{ "fma.speed-mode", _fma_speed_mode, true },
		{ "fma.roll-mode", _fma_roll_mode, true },
		{ "fma.pitch-mode", _fma_pitch_mode, true },
	});

	prepare_afcs_main_panel();
	prepare_speed_panel();
	prepare_heading_panel();
	prepare_nav_panel();
	prepare_altitude_panel();
	prepare_vspd_panel();

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
AutomatedFlightControlSystem::data_updated()
{
	for (Xefis::RotaryEncoder* r: _rotary_decoders)
		r->data_updated();

	process_afcs_main_panel();
	process_altitude_panel();
}


void
AutomatedFlightControlSystem::prepare_afcs_main_panel()
{
//	TODO _mcp_ap;
//	TODO _mcp_at;
//	TODO _mcp_prot;
//	TODO _mcp_tac;
//	TODO _mcp_att;
//	TODO _mcp_ct;
}


void
AutomatedFlightControlSystem::process_afcs_main_panel()
{
	if (pressed (_mcp_ap))
	{
		_ap_on = !_ap_on;
		solve_mode();
	}

	if (pressed (_mcp_at))
	{
		_at_on = !_at_on;
		solve_mode();
	}

	if (pressed (_mcp_att))
	{
		_att_on = !_att_on;
		solve_mode();
	}
}


void
AutomatedFlightControlSystem::prepare_speed_panel()
{
	_mcp_speed_decoder = std::make_unique<Xefis::RotaryEncoder> (_mcp_speed_a, _mcp_speed_b, [this](int delta) {
		_cmd_speed_counter = Xefis::limit (_cmd_speed_counter + 1_kt * delta, CmdSpeedRange);
		solve_mode();
	});

//	TODO _mcp_speed_ias_mach;
//	TODO _mcp_speed_sel
//	TODO _mcp_speed_hold
//		check is speed is not nil or EICAS warning
//		check if speed inside safe range or EICAS warning
}


void
AutomatedFlightControlSystem::prepare_heading_panel()
{
	_mcp_heading_decoder = std::make_unique<Xefis::RotaryEncoder> (_mcp_heading_a, _mcp_heading_b, [this](int delta) {
		_cmd_heading_counter = Xefis::floored_mod (_cmd_heading_counter + 1_deg * delta, 360_deg);
		solve_mode();
	});

//	TODO _mcp_heading_hdg_trk;
//	TODO _mcp_heading_sel
//	TODO _mcp_heading_hold
//		check if pitch is not nil or EICAS warning
//		check if orientation inside safe limit (HeadingHoldPitchLimit) or EICAS warning
}


void
AutomatedFlightControlSystem::prepare_nav_panel()
{
//	TODO _mcp_vnav
//	TODO _mcp_lnav
//	TODO _mcp_app
}


void
AutomatedFlightControlSystem::prepare_altitude_panel()
{
	_mcp_altitude_decoder = std::make_unique<Xefis::RotaryEncoder> (_mcp_altitude_a, _mcp_altitude_b, [this](int delta) {
		Length altitude_step;
		switch (_cmd_altitude_step)
		{
			case CmdAltitudeStep::Ft10:		altitude_step = 10_ft; break;
			case CmdAltitudeStep::Ft100:	altitude_step = 100_ft; break;
		}
		_cmd_altitude_counter = Xefis::limit (_cmd_altitude_counter + altitude_step * delta, CmdAltitudeRange);
		solve_mode();
	});

//	TODO _mcp_altitude_flch
//	TODO _mcp_altitude_hold
//		check if roll is not nil or EICAS warning
//		check if roll inside safe limit (AltitudeHoldRollLimit) or EICAS warning
}


void
AutomatedFlightControlSystem::process_altitude_panel()
{
	// Altitude Step Change:
	if (pressed (_mcp_altitude_stepch))
	{
		switch (_cmd_altitude_step)
		{
			case CmdAltitudeStep::Ft100:	_cmd_altitude_step = CmdAltitudeStep::Ft10; break;
			case CmdAltitudeStep::Ft10:		_cmd_altitude_step = CmdAltitudeStep::Ft100; break;
		}
	}

// TODO _mcp_altitude_flch
// TODO _mcp_altitude_hold
}


void
AutomatedFlightControlSystem::prepare_vspd_panel()
{
	_mcp_vspd_decoder = std::make_unique<Xefis::RotaryEncoder> (_mcp_vspd_a, _mcp_vspd_b, [this](int delta) {
		_cmd_vspd_counter = Xefis::limit (_cmd_vspd_counter + CmdVSpdStep * delta, CmdVSpdRange);
		solve_mode();
	});

//	TODO _mcp_vspd_vs_fpa
//	TODO _mcp_vspd_sel
//	TODO _mcp_vspd_clb_con
}


void
AutomatedFlightControlSystem::solve_mode()
{
	_mcp_ap_led.write (_ap_on);
	_mcp_at_led.write (_at_on);
	_mcp_att_led.write (_att_on);
	_mcp_speed_display.write (Xefis::symmetric_round (_cmd_speed_counter.kt()));
	int heading = Xefis::symmetric_round (_cmd_heading_counter.deg());
	if (heading == 0)
		heading = 360;
	_mcp_heading_display.write (heading);
	_mcp_altitude_display.write (Xefis::symmetric_round (_cmd_altitude_counter.ft()));
	_mcp_vspd_display.write (Xefis::symmetric_round (_cmd_vspd_counter.fpm()));

	// Control FBW module:
	FlyByWire::AttitudeMode fbw_attitude_mode = FlyByWire::AttitudeMode::Manual;
	if (_ap_on)
		fbw_attitude_mode = FlyByWire::AttitudeMode::FlightDirector;
	else if (_att_on)
		fbw_attitude_mode = FlyByWire::AttitudeMode::Stabilized;
	_fbw_attitude_mode.write (static_cast<decltype (_fbw_attitude_mode)::Type> (fbw_attitude_mode));

	FlyByWire::ThrottleMode fbw_throttle_mode = FlyByWire::ThrottleMode::Manual;
	if (_at_on)
		fbw_throttle_mode = FlyByWire::ThrottleMode::Autothrottle;
	_fbw_throttle_mode.write (static_cast<decltype (_fbw_throttle_mode)::Type> (fbw_throttle_mode));

	// Control A/T and FD modules:
	_cmd_ias.write (_cmd_speed_counter);
	_cmd_heading_track.write (_cmd_heading_counter);
	_cmd_altitude.write (_cmd_altitude_counter);
	// TODO only if accplicable
	_cmd_vspd.write (_cmd_vspd_counter);
	_cmd_fpa.write (0.0_deg);//TODO

	update_fma();
	signal_data_updated();
}


void
AutomatedFlightControlSystem::update_fma()
{
	switch (static_cast<FlyByWire::AttitudeMode> (*_fbw_attitude_mode))
	{
		case FlyByWire::AttitudeMode::Manual:
			_fma_control_hint.write ("");
			break;

		case FlyByWire::AttitudeMode::Stabilized:
			_fma_control_hint.write ("ATT");
			break;

		case FlyByWire::AttitudeMode::FlightDirector:
			_fma_control_hint.write ("FLT DIR");
			break;

		default:
			_fma_control_hint.write ("---");
			break;
	}

	_fma_speed_mode.write (speed_mode_string (_speed_mode));
	_fma_roll_mode.write (roll_mode_string (_roll_mode));
	_fma_pitch_mode.write (pitch_mode_string (_pitch_mode));

	signal_data_updated();
}


std::string const&
AutomatedFlightControlSystem::speed_mode_string (SpeedMode speed_mode)
{
	static std::array<std::string, 9> names = {
		"",
		"TO/GA",
		"THR",
		"THR REF",
		"IDLE",
		"SPD REF",
		"SPD SEL",
		"MCP SPD",
		"HOLD",
	};

	return names[static_cast<std::size_t> (speed_mode)];
}


std::string const&
AutomatedFlightControlSystem::roll_mode_string (RollMode roll_mode)
{
	static std::array<std::string, 6> names = {
		"",
		"HDG SEL",
		"HDG",
		"TRK SEL",
		"TRK",
		"LOC",
	};

	return names[static_cast<std::size_t> (roll_mode)];
}


std::string const&
AutomatedFlightControlSystem::pitch_mode_string (PitchMode pitch_mode)
{
	static std::array<std::string, 12> names = {
		"",
		"FLCH SPD",
		"FLCH V/S",
		"FLCH FPA",
		"V/S",
		"FPA",
		"CLB/CON",
		"VNAV SPD",
		"VNAV PTH",
		"ALT",
		"ALT HOLD",
		"G/S",
	};

	return names[static_cast<std::size_t> (pitch_mode)];
}


bool
AutomatedFlightControlSystem::pressed (Xefis::PropertyBoolean const& property)
{
	return property.fresh() && property.read (false);
}


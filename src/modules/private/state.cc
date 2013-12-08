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
#include <memory>

// Qt:
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/config/exception.h>
#include <xefis/utility/numeric.h>

// Local:
#include "state.h"


XEFIS_REGISTER_MODULE_CLASS ("private/state", State);


constexpr Length					State::MinimumsBaroStep;
constexpr Length					State::MinimumsRadioStep;
constexpr Xefis::Range<Length>		State::MinimumsBaroRange;
constexpr Xefis::Range<Length>		State::MinimumsRadioRange;
constexpr Pressure					State::QNHhPaStep;
constexpr Pressure					State::QNHinHgStep;
constexpr Xefis::Range<Pressure>	State::QNHRange;
constexpr Xefis::Range<Speed>		State::CmdSpeedRange;
constexpr Xefis::Range<Length>		State::CmdAltitudeRange;
constexpr Speed						State::CmdVSpdStep;
constexpr Xefis::Range<Speed>		State::CmdVSpdRange;
constexpr Angle						State::HeadingHoldPitchLimit;
constexpr Angle						State::AltitudeHoldRollLimit;


template<class P>
	void
	State::Observable<P>::process()
	{
		typename PropertyType::Type v = *_property;

		if (v != _prev_value)
		{
			_prev_value = v;
			if (_callback)
				_callback (_property);
		}
	}


State::Switch::Switch (std::string const& momentary_path, std::string const& latching_path):
	Observable (momentary_path),
	_latching_prop (latching_path)
{
	set_callback ([this](Xefis::PropertyBoolean& momentary_prop) {
		if (*momentary_prop)
			_latching_prop.write (!*_latching_prop);
	});
}


State::State (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	std::string mcp_root = "/panels/mcp";

	_saitek_a.set_path ("/input/main/button/2");
	_saitek_b.set_path ("/input/main/button/3");
	_saitek_c.set_path ("/input/main/button/4");
	_saitek_d.set_path ("/input/main/button/6");
	_saitek_shift.set_path ("/input/main/button/5");
	_saitek_t1.set_path ("/input/main/button/8");
	_saitek_t2.set_path ("/input/main/button/9");
	_saitek_t3.set_path ("/input/main/button/10");
	_saitek_t4.set_path ("/input/main/button/11");
	_saitek_t5.set_path ("/input/main/button/12");
	_saitek_t6.set_path ("/input/main/button/13");
	_saitek_mode_1.set_path ("/input/main/button/27");
	_saitek_mode_2.set_path ("/input/main/button/28");
	_saitek_mode_3.set_path ("/input/main/button/29");
	_saitek_mfd_startstop.set_path ("/input/main/button/32");
	_saitek_mfd_reset.set_path ("/input/main/button/33");
	_saitek_function_up.set_path ("/input/main/button/34");
	_saitek_function_down.set_path ("/input/main/button/35");
	_saitek_function_press.set_path ("/input/main/button/31");
	_saitek_mfd_select_up.set_path ("/input/main/button/36");
	_saitek_mfd_select_down.set_path ("/input/main/button/37");
	_saitek_mfd_select_press.set_path ("/input/main/button/38");
	_mcp_mins_a.set_path (mcp_root + "/mins-a");
	_mcp_mins_b.set_path (mcp_root + "/mins-b");
	_mcp_mins_mode.set_path (mcp_root + "/mins-mode");
	_mcp_appr.set_path (mcp_root + "/appr");
	_mcp_fd.set_path (mcp_root + "/fd");
	_mcp_qnh_a.set_path (mcp_root + "/qnh-a");
	_mcp_qnh_b.set_path (mcp_root + "/qnh-b");
	_mcp_qnh_hpa.set_path (mcp_root + "/qnh-hpa");
	_mcp_std.set_path (mcp_root + "/std");
	_mcp_stick.set_path (mcp_root + "/stick");
	_mcp_metric.set_path (mcp_root + "/metric");
	_mcp_fpv.set_path (mcp_root + "/fpv");
	_mcp_range_a.set_path (mcp_root + "/range-a");
	_mcp_range_b.set_path (mcp_root + "/range-b");
	_mcp_range_ctr.set_path (mcp_root + "/range-ctr");
	_mcp_hdg_trk.set_path (mcp_root + "/hdg-trk");
	_mcp_mag_tru.set_path (mcp_root + "/mag-tru");
	_mcp_course_a.set_path (mcp_root + "/course-a");
	_mcp_course_b.set_path (mcp_root + "/course-b");
	_mcp_course_hide.set_path (mcp_root + "/course-hide");
	_mcp_ap.set_path (mcp_root + "/ap");
	_mcp_at.set_path (mcp_root + "/at");
	_mcp_prot.set_path (mcp_root + "/prot");
	_mcp_tac.set_path (mcp_root + "/tac");
	_mcp_att.set_path (mcp_root + "/att");
	_mcp_ct.set_path (mcp_root + "/ct");
	_mcp_speed_a.set_path (mcp_root + "/speed-a");
	_mcp_speed_b.set_path (mcp_root + "/speed-b");
	_mcp_speed_ias_mach.set_path (mcp_root + "/speed-ias-mach");
	_mcp_speed_sel.set_path (mcp_root + "/speed-sel");
	_mcp_speed_hold.set_path (mcp_root + "/speed-hold");
	_mcp_heading_a.set_path (mcp_root + "/heading-a");
	_mcp_heading_b.set_path (mcp_root + "/heading-b");
	_mcp_heading_hdg_trk.set_path (mcp_root + "/heading-ias-mach");
	_mcp_heading_sel.set_path (mcp_root + "/heading-sel");
	_mcp_heading_hold.set_path (mcp_root + "/heading-hold");
	_mcp_vnav.set_path (mcp_root + "/vnav");
	_mcp_lnav.set_path (mcp_root + "/lnav");
	_mcp_app.set_path (mcp_root + "/app");
	_mcp_altitude_a.set_path (mcp_root + "/altitude-a");
	_mcp_altitude_b.set_path (mcp_root + "/altitude-b");
	_mcp_altitude_stepch.set_path (mcp_root + "/altitude-stepch");
	_mcp_altitude_flch.set_path (mcp_root + "/altitude-flch");
	_mcp_altitude_hold.set_path (mcp_root + "/altitude-hold");
	_mcp_vspd_a.set_path (mcp_root + "/vspd-a");
	_mcp_vspd_b.set_path (mcp_root + "/vspd-b");
	_mcp_vspd_vs_fpa.set_path (mcp_root + "/vspd-vs-fpa");
	_mcp_vspd_sel.set_path (mcp_root + "/vspd-sel");
	_mcp_vspd_clb_con.set_path (mcp_root + "/vspd-clb-con");
	_mcp_gpws_flap_inh.set_path (mcp_root + "/gpws-flap-inh");
	_mcp_gpws_gear_inh.set_path (mcp_root + "/gpws-gear-inh");
	_mcp_gpws_terr_inh.set_path (mcp_root + "/gpws-terr-inh");
	_mcp_lights_strobe.set_path (mcp_root + "/lights-strobe");
	_mcp_lights_pos.set_path (mcp_root + "/lights-pos");
	_mcp_lights_ldg.set_path (mcp_root + "/lights-ldg");
	_mcp_eicas_cancel.set_path (mcp_root + "/eicas-cancel");
	_mcp_eicas_recall.set_path (mcp_root + "/eicas-recall");
	_mcp_eicas_ok.set_path (mcp_root + "/eicas-ok");
	_mcp_show_nd.set_path (mcp_root + "/show-nd");
	_mcp_show_eicas.set_path (mcp_root + "/show-eicas");
	_mcp_show_chklst.set_path (mcp_root + "/show-chklst");

	_mcp_course_display.set_path ("/panels/mcp/display/course");
	_mcp_speed_display.set_path ("/panels/mcp/display/speed");
	_mcp_heading_display.set_path ("/panels/mcp/display/heading");
	_mcp_altitude_display.set_path ("/panels/mcp/display/altitude");
	_mcp_vspd_display.set_path ("/panels/mcp/display/vertical-speed");
	_mcp_speed_sel_led.set_path ("/panels/mcp/led/speed-sel");
	_mcp_speed_hold_led.set_path ("/panels/mcp/led/speed-hold");
	_mcp_heading_sel_led.set_path ("/panels/mcp/led/heading-sel");
	_mcp_heading_hold_led.set_path ("/panels/mcp/led/heading-hold");
	_mcp_lnav_led.set_path ("/panels/mcp/led/lnav");
	_mcp_vnav_led.set_path ("/panels/mcp/led/vnav");
	_mcp_altitude_flch_led.set_path ("/panels/mcp/led/altitude-flch");
	_mcp_altitude_hold_led.set_path ("/panels/mcp/led/altitude-hold");

	_orientation_pitch.set_path ("/systems/nc/orientation/pitch");
	_orientation_roll.set_path ("/systems/nc/orientation/roll");
	_orientation_magnetic_heading.set_path ("/systems/nc/orientation/heading/magnetic");
	_orientation_magnetic_track.set_path ("/systems/nc/track/lateral/magnetic");
	_speed_ias.set_path ("/systems/adc/speed/ias");
	_altitude_amsl.set_path ("/systems/adc/altitude/amsl");

	_setting_efis_fpv_visible.set_path ("/settings/efis/fpv-visible");
	_setting_efis_fpv_visible.set_default (false);
	_setting_efis_show_metric.set_path ("/settings/efis/show-metric");
	_setting_efis_show_metric.set_default (false);
	_setting_efis_fd_visible.set_path ("/settings/efis/flight-director-visible");
	_setting_efis_fd_visible.set_default (false);
	_setting_efis_appr_visible.set_path ("/settings/efis/approach-reference-visible");
	_setting_efis_appr_visible.set_default (false);
	_setting_efis_stick_visible.set_path ("/settings/efis/stick-visible");
	_setting_efis_stick_visible.set_default (false);
	_setting_efis_mfd_mode.set_path ("/settings/efis/mfd-mode");
	_setting_efis_mfd_mode.set_default (static_cast<int> (MFDMode::EICAS));
	_setting_pressure_qnh.set_path ("/settings/pressure/qnh");
	_setting_pressure_qnh.set_default (29.92_inHg);
	_setting_pressure_display_hpa.set_path ("/settings/pressure/display-hpa");
	_setting_pressure_display_hpa.set_default (true);
	_setting_pressure_use_std.set_path ("/settings/pressure/use-std");
	_setting_pressure_use_std.set_default (false);
	_setting_minimums_amsl.set_path ("/settings/minimums/amsl");
	_setting_minimums_setting.set_path ("/settings/minimums/setting");
	_setting_minimums_type.set_path ("/settings/minimums/type");
	_setting_cmd_speed.set_path ("/settings/flight-director/cmd/ias");
	_setting_cmd_heading.set_path ("/settings/flight-director/cmd/magnetic-heading-track");
	_setting_cmd_altitude.set_path ("/settings/flight-director/cmd/altitude");
	_setting_cmd_vspd.set_path ("/settings/flight-director/cmd/vertical-speed");
	_setting_hsi_display_true_heading.set_path ("/settings/hsi/display-true-heading");
	_setting_hsi_display_true_heading.set_default (false);
	_setting_hsi_center_on_track.set_path ("/settings/hsi/center-on-track");
	_setting_hsi_center_on_track.set_default (true);
	_setting_hsi_display_mode_mfd.set_path ("/settings/hsi/display-mode/mfd");
	_setting_hsi_display_mode_mfd.set_default (0);
	_setting_hsi_range.set_path ("/settings/hsi/range");
	_setting_hsi_range.set_default (1_nm);
	_setting_lights_strobe.set_path ("/settings/lights/strobe");
	_setting_lights_strobe.set_default (false);
	_setting_lights_position.set_path ("/settings/lights/position");
	_setting_lights_position.set_default (false);
	_setting_lights_landing.set_path ("/settings/lights/landing");
	_setting_lights_landing.set_default (false);
	_setting_fma_speed.set_path ("/settings/fma/speed-hint");
	_setting_fma_lateral.set_path ("/settings/fma/lateral-hint");
	_setting_fma_vertical.set_path ("/settings/fma/vertical-hint");

	prepare_efis_settings();
	prepare_afcs_main_panel();
	prepare_speed_panel();
	prepare_heading_panel();
	prepare_nav_panel();
	prepare_altitude_panel();
	prepare_vspd_panel();
	prepare_lights_gpws_mfd_panels();

	_observables = {
		&_saitek_t1,
		&_saitek_t2,
		&_saitek_t3,
		&_saitek_t4,
		&_saitek_t5,
		&_saitek_t6,
		&_saitek_shift,
		&_saitek_a,
		&_saitek_b,
		&_saitek_c,
		&_saitek_d,
		&_saitek_mode_1,
		&_saitek_mode_2,
		&_saitek_mode_3,
		&_saitek_mfd_startstop,
		&_saitek_mfd_reset,
		&_saitek_function_up,
		&_saitek_function_down,
		&_saitek_function_press,
		&_saitek_mfd_select_up,
		&_saitek_mfd_select_down,
		&_saitek_mfd_select_press,
		&_mcp_mins_mode,
		&_mcp_appr,
		&_mcp_fd,
		&_mcp_qnh_hpa,
		&_mcp_std,
		&_mcp_stick,
		&_mcp_metric,
		&_mcp_fpv,
		&_mcp_range_ctr,
		&_mcp_hdg_trk,
		&_mcp_mag_tru,
		&_mcp_course_hide,
		&_mcp_ap,
		&_mcp_at,
		&_mcp_prot,
		&_mcp_tac,
		&_mcp_att,
		&_mcp_ct,
		&_mcp_speed_ias_mach,
		&_mcp_speed_sel,
		&_mcp_speed_hold,
		&_mcp_heading_hdg_trk,
		&_mcp_heading_sel,
		&_mcp_heading_hold,
		&_mcp_vnav,
		&_mcp_lnav,
		&_mcp_app,
		&_mcp_altitude_stepch,
		&_mcp_altitude_flch,
		&_mcp_altitude_hold,
		&_mcp_vspd_vs_fpa,
		&_mcp_vspd_sel,
		&_mcp_vspd_clb_con,
		&_mcp_gpws_flap_inh,
		&_mcp_gpws_gear_inh,
		&_mcp_gpws_terr_inh,
		&_mcp_lights_strobe,
		&_mcp_lights_pos,
		&_mcp_lights_ldg,
		&_mcp_eicas_cancel,
		&_mcp_eicas_recall,
		&_mcp_eicas_ok,
		&_mcp_show_nd,
		&_mcp_show_eicas,
		&_mcp_show_chklst,
	};

	_rotary_decoders = {
		_mcp_mins_decoder.get(),
		_mcp_qnh_decoder.get(),
		_mcp_range_decoder.get(),
		_mcp_course_decoder.get(),
		_mcp_speed_decoder.get(),
		_mcp_heading_decoder.get(),
		_mcp_altitude_decoder.get(),
		_mcp_vspd_decoder.get(),
	};

	_mcp_mins_decoder->call (0);
	_mcp_qnh_decoder->call (0);
	_mcp_range_decoder->call (0);
	_mcp_course_decoder->call (0);
	_mcp_speed_decoder->call (0);
	_mcp_heading_decoder->call (0);
	_mcp_altitude_decoder->call (0);
	_mcp_vspd_decoder->call (0);

	solve_minimums();
	solve_pressure();
	solve_fd();

	_periodic_timer = new QTimer (this);
	_periodic_timer->setSingleShot (false);
	_periodic_timer->setInterval (100);
	QObject::connect (_periodic_timer, SIGNAL (timeout()), this, SLOT (periodic_fd_check()));
	_periodic_timer->start();
}


void
State::data_updated()
{
	for (ObservableBase* o: _observables)
		o->process();
	for (Xefis::RotaryEncoder* r: _rotary_decoders)
		r->data_updated();
	// TODO observe autoflight modules if they're not disengaged
}


void
State::periodic_fd_check()
{
	// TODO eg
	//if (_lateral_mode == Select && current-heading == counter.heading (with margin 5 deg))
	//{
	//	_lateral_mode = Hold;
	//}
	solve_fd();
}


void
State::prepare_efis_settings()
{
	_mcp_mins_decoder = std::make_unique<Xefis::RotaryEncoder> (_mcp_mins_a, _mcp_mins_b, [this](int delta) {
		switch (_minimums_type)
		{
			case MinimumsType::Baro:
				_minimums_setting_baro += MinimumsBaroStep * delta;
				break;

			case MinimumsType::Radio:
				_minimums_setting_radio += MinimumsRadioStep * delta;
				break;
		}

		solve_minimums();
		signal_data_updated();
	});

	make_switch (_mcp_mins_mode, [this]() {
		switch (_minimums_type)
		{
			case MinimumsType::Baro:
				_minimums_type = MinimumsType::Radio;
				break;

			case MinimumsType::Radio:
				_minimums_type = MinimumsType::Baro;
				break;
		}

		solve_minimums();
	});

	make_toggle (_mcp_appr, _setting_efis_appr_visible);
	make_toggle (_mcp_fd, _setting_efis_fd_visible);

	_mcp_qnh_decoder = std::make_unique<Xefis::RotaryEncoder> (_mcp_qnh_a, _mcp_qnh_b, [this](int delta) {
		if (*_setting_pressure_display_hpa)
			_qnh_setting += QNHhPaStep * delta;
		else
			_qnh_setting += QNHinHgStep * delta;

		solve_pressure();
		signal_data_updated();
	});

	make_toggle (_mcp_qnh_hpa, _setting_pressure_display_hpa);
	make_toggle (_mcp_std, _setting_pressure_use_std);
	make_toggle (_mcp_stick, _setting_efis_stick_visible);
	make_toggle (_mcp_metric, _setting_efis_show_metric);
	make_toggle (_mcp_fpv, _setting_efis_fpv_visible);

	_mcp_range_decoder = std::make_unique<Xefis::RotaryEncoder> (_mcp_range_a, _mcp_range_b, [this](int delta) {
		int range_nm = Xefis::symmetric_round ((*_setting_hsi_range).nm());
		delta = -delta;

		if (range_nm < 2)
			range_nm += 1 * delta;
		else if (range_nm < 20)
			range_nm += 2 * delta;
		else if (range_nm < 60)
			range_nm += 10 * delta;
		else if (range_nm < 100)
			range_nm += 20 * delta;
		else
			range_nm += 50 * delta;

		range_nm = Xefis::limit (range_nm, 1, 50);
		_setting_hsi_range.write (1_nm * range_nm);

		signal_data_updated();
	});

	make_switch (_mcp_range_ctr, [this]() {
		_setting_hsi_display_mode_mfd.write ((_setting_hsi_display_mode_mfd.read (0) + 1) % 2);
	});

	make_toggle (_mcp_hdg_trk, _setting_hsi_center_on_track);
	make_toggle (_mcp_mag_tru, _setting_hsi_display_true_heading);

	_mcp_course_decoder = std::make_unique<Xefis::RotaryEncoder> (_mcp_course_a, _mcp_course_b, [this](int delta) {
		_course = Xefis::floored_mod (_course + 1_deg * delta, 360_deg);
		signal_data_updated();
	});
}


void
State::prepare_afcs_main_panel()
{
//	_mcp_course_hide;
//	_mcp_ap;
//	_mcp_at;
//	_mcp_prot;
//	_mcp_tac;
//	_mcp_att;
//	_mcp_ct;
}


void
State::prepare_speed_panel()
{
	_mcp_speed_decoder = std::make_unique<Xefis::RotaryEncoder> (_mcp_speed_a, _mcp_speed_b, [this](int delta) {
		_cmd_speed_counter = Xefis::limit (_cmd_speed_counter + 1_kt * delta, CmdSpeedRange);
		solve_fd();
	});

//	_mcp_speed_ias_mach;

	make_switch (_mcp_speed_sel, [this]() {
		if (_speed_mode == SpeedMode::Select)
			_speed_mode = SpeedMode::None;
		else
			_speed_mode = SpeedMode::Select;
		solve_fd();
	});

	make_switch (_mcp_speed_hold, [this]() {
		if (_speed_mode == SpeedMode::Hold)
			_speed_mode = SpeedMode::None;
		else
		{
			if (_speed_ias.is_nil())
			{
				// TODO EICAS message
				return;
			}

			if (!CmdSpeedRange.includes (*_speed_ias))
			{
				// TODO EICAS message
				return;
			}

			_speed_mode = SpeedMode::Hold;
		}

		solve_fd();
	});
}


void
State::prepare_heading_panel()
{
	_mcp_heading_decoder = std::make_unique<Xefis::RotaryEncoder> (_mcp_heading_a, _mcp_heading_b, [this](int delta) {
		_cmd_heading_counter = Xefis::floored_mod (_cmd_heading_counter + 1_deg * delta, 360_deg);
		solve_fd();
	});

//	_mcp_heading_hdg_trk;

	make_switch (_mcp_heading_sel, [this]() {
		if (_lateral_mode == LateralMode::Select)
			_lateral_mode = LateralMode::None;
		else
			_lateral_mode = LateralMode::Select;
		solve_fd();
	});

	// TODO auto disengage subpilot if eg. track or heading is unavailable

	make_switch (_mcp_heading_hold, [this]() {
		if (_lateral_mode == LateralMode::Hold)
			_lateral_mode = LateralMode::None;
		else
		{
			if (_orientation_pitch.is_nil())
			{
				// TODO EICAS message
				return;
			}

			// User presses HOLD: set current heading on the counter,
			// but only if abs(pitch) < HeadingHoldPitchLimit.
			if (std::abs (_orientation_pitch.read().deg()) > HeadingHoldPitchLimit.deg())
			{
				// TODO EICAS message
				return;
			}

			_lateral_mode = LateralMode::Hold;
		}
		// TODO prevent possibility of setting track if track is unavailable (nil)
		solve_fd();
	});
}


void
State::prepare_nav_panel()
{
	make_switch (_mcp_vnav, [this]() {
		if (_vertical_mode == VerticalMode::VNAV)
			_vertical_mode = VerticalMode::None;
		else
			_vertical_mode = VerticalMode::VNAV;
		solve_fd();
	});

	make_switch (_mcp_lnav, [this]() {
		if (_lateral_mode == LateralMode::LNAV)
			_lateral_mode = LateralMode::None;
		else
			_lateral_mode = LateralMode::LNAV;
		solve_fd();
	});

//	_mcp_app
}


void
State::prepare_altitude_panel()
{
	_mcp_altitude_decoder = std::make_unique<Xefis::RotaryEncoder> (_mcp_altitude_a, _mcp_altitude_b, [this](int delta) {
		Length altitude_step;
		switch (_cmd_altitude_step)
		{
			case CmdAltitudeStep::Ft10:		altitude_step = 10_ft; break;
			case CmdAltitudeStep::Ft100:	altitude_step = 100_ft; break;
		}
		_cmd_altitude_counter = Xefis::limit (_cmd_altitude_counter + altitude_step * delta, CmdAltitudeRange);
		solve_fd();
	});

	make_switch (_mcp_altitude_stepch, [this]() {
		switch (_cmd_altitude_step)
		{
			case CmdAltitudeStep::Ft100:	_cmd_altitude_step = CmdAltitudeStep::Ft10; break;
			case CmdAltitudeStep::Ft10:		_cmd_altitude_step = CmdAltitudeStep::Ft100; break;
		}
	});

	make_switch (_mcp_altitude_flch, [this]() {
		if (_vertical_mode == VerticalMode::Select)
			_vertical_mode = VerticalMode::None;
		else
			_vertical_mode = VerticalMode::Select;
		solve_fd();
	});

	make_switch (_mcp_altitude_hold, [this]() {
		if (_vertical_mode == VerticalMode::Hold)
			_vertical_mode = VerticalMode::None;
		else
		{
			if (_orientation_roll.is_nil())
			{
				// TODO EICAS message
				return;
			}

			if (std::abs (_orientation_roll.read().deg()) > AltitudeHoldRollLimit.deg())
			{
				// TODO EICAS message
				return;
			}

			_vertical_mode = VerticalMode::Hold;
		}

		solve_fd();
	});
}


void
State::prepare_vspd_panel()
{
	_mcp_vspd_decoder = std::make_unique<Xefis::RotaryEncoder> (_mcp_vspd_a, _mcp_vspd_b, [this](int delta) {
		_cmd_vspd_counter = Xefis::limit (_cmd_vspd_counter + CmdVSpdStep * delta, CmdVSpdRange);
		solve_fd();
	});

//	_mcp_vspd_vs_fpa
//	_mcp_vspd_sel
//	_mcp_vspd_clb_con
}


void
State::prepare_lights_gpws_mfd_panels()
{
//	_mcp_gpws_flap_inh
//	_mcp_gpws_gear_inh
//	_mcp_gpws_terr_inh

	make_toggle (_mcp_lights_strobe, _setting_lights_strobe);
	make_toggle (_mcp_lights_pos, _setting_lights_position);
	make_toggle (_mcp_lights_ldg, _setting_lights_landing);

//	_mcp_eicas_cancel;
//	_mcp_eicas_recall;
//	_mcp_eicas_ok;

	make_int_writer (_mcp_show_eicas, _setting_efis_mfd_mode, static_cast<int> (MFDMode::EICAS));
	make_int_writer (_mcp_show_nd, _setting_efis_mfd_mode, static_cast<int> (MFDMode::ND));
	make_int_writer (_mcp_show_chklst, _setting_efis_mfd_mode, static_cast<int> (MFDMode::CHKLST));
}


void
State::solve_minimums()
{
	_minimums_setting_baro = Xefis::limit (_minimums_setting_baro, MinimumsBaroRange);
	_minimums_setting_radio = Xefis::limit (_minimums_setting_radio, MinimumsRadioRange);

	switch (_minimums_type)
	{
		case MinimumsType::Baro:
			_setting_minimums_type.write ("BARO");
			_setting_minimums_setting.write (_minimums_setting_baro);
			_setting_minimums_amsl.write (0_ft); // TODO ldg alt
			break;

		case MinimumsType::Radio:
			_setting_minimums_type.write ("RADIO");
			_setting_minimums_setting.write (_minimums_setting_radio);
			_setting_minimums_amsl.write (0_ft); // TODO ldg alt
			break;
	}
}


void
State::solve_pressure()
{
	_qnh_setting = Xefis::limit (_qnh_setting, QNHRange);
	_setting_pressure_qnh.write (_qnh_setting);
	signal_data_updated();
}


void
State::solve_fd()
{
	// TODO FMA

	bool led_speed_sel = false;
	bool led_speed_hold = false;
	bool led_lnav = false;
	bool led_heading_sel = false;
	bool led_heading_hold = false;
	bool led_vnav = false;
	bool led_altitude_flch = false;
	bool led_altitude_hold = false;

	switch (_speed_mode)
	{
		case SpeedMode::None:
			_setting_fma_speed.write ("");
			// TODO
			break;

		case SpeedMode::Select:
			_setting_fma_speed.write ("SPD");
			led_speed_sel = true;
			break;

		case SpeedMode::Hold:
			_setting_fma_speed.write ("SPD HOLD");
			led_speed_hold = true;
			break;
	}

	switch (_lateral_mode)
	{
		case LateralMode::None:
			_setting_fma_lateral.write ("");
			// TODO disable lateral mode in FD module
			break;

		case LateralMode::LNAV:
			_setting_fma_lateral.write ("LNAV");
			led_lnav = true;
			// TODO
			break;

		case LateralMode::Select:
			_setting_fma_lateral.write ("HDG SEL");
			led_heading_sel = true;
			// TODO
			break;

		case LateralMode::Hold:
			_setting_fma_lateral.write ("HDG HOLD");
			led_heading_hold = true;
			break;
	}

	switch (_vertical_mode)
	{
		case VerticalMode::None:
			_setting_fma_vertical.write ("");
			// TODO
			break;

		case VerticalMode::VNAV:
			_setting_fma_vertical.write ("VNAV");
			led_vnav = true;
			// TODO
			break;

		case VerticalMode::Select:
			_setting_fma_vertical.write ("ALT");
			led_altitude_flch = true;
			break;

		case VerticalMode::Hold:
			_setting_fma_vertical.write ("ALT HOLD");
			led_altitude_hold = true;
			break;
	}

	if (_speed_mode == SpeedMode::Hold)
		_cmd_speed_selected = _cmd_speed_counter;
	if (_lateral_mode == LateralMode::Hold)
		_cmd_heading_selected = _cmd_heading_counter;
	if (_vertical_mode == VerticalMode::Hold)
		_cmd_altitude_selected = _cmd_altitude_counter;
	// TODO CLB CON

	_mcp_speed_sel_led.write (led_speed_sel);
	_mcp_speed_hold_led.write (led_speed_hold);
	_mcp_lnav_led.write (led_lnav);
	_mcp_heading_sel_led.write (led_heading_sel);
	_mcp_heading_hold_led.write (led_heading_hold);
	_mcp_vnav_led.write (led_vnav);
	_mcp_altitude_flch_led.write (led_altitude_flch);
	_mcp_altitude_hold_led.write (led_altitude_hold);
	_mcp_course_display.write (Xefis::symmetric_round (_course.deg()));
	_mcp_speed_display.write (Xefis::symmetric_round (_cmd_speed_counter.kt()));
	_mcp_heading_display.write (Xefis::symmetric_round (_cmd_heading_counter.deg()));
	_mcp_altitude_display.write (Xefis::symmetric_round (_cmd_altitude_counter.ft()));
	_mcp_vspd_display.write (Xefis::symmetric_round (_cmd_vspd_counter.fpm()));

	_setting_cmd_speed.write (_cmd_speed_selected);
	_setting_cmd_heading.write (_cmd_heading_selected);
	_setting_cmd_altitude.write (_cmd_altitude_selected);
	_setting_cmd_vspd.write (_cmd_vspd_selected);

	signal_data_updated();
}


void
State::make_switch (Observable<Xefis::PropertyBoolean>& bool_observable, std::function<void()> callback)
{
	bool_observable.set_callback ([callback,this](Xefis::PropertyBoolean& prop) {
		if (*prop)
		{
			callback();
			signal_data_updated();
		}
	});
}


void
State::make_toggle (Observable<Xefis::PropertyBoolean>& bool_observable, Xefis::PropertyBoolean& target_switch)
{
	bool_observable.set_callback ([&](Xefis::PropertyBoolean& prop) {
		if (*prop)
		{
			target_switch.write (!*target_switch);
			signal_data_updated();
		}
	});
}


void
State::make_int_writer (Observable<Xefis::PropertyBoolean>& bool_observable, Xefis::PropertyInteger& target_property, int value)
{
	bool_observable.set_callback ([&target_property,value,this](Xefis::PropertyBoolean& prop) {
		if (*prop)
		{
			target_property.write (value);
			signal_data_updated();
		}
	});
}


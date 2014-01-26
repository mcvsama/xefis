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


State::State (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	// TODO not hardcoded
	std::string mcp_root = "/panels/mcp";

	_mcp_mins_a.set_path (mcp_root + "/mins-a");
	_mcp_mins_b.set_path (mcp_root + "/mins-b");
	_mcp_appr.set_path (mcp_root + "/appr");
	_mcp_fd.set_path (mcp_root + "/fd");
	_mcp_htrk.set_path (mcp_root + "/htrk");
	_mcp_qnh_a.set_path (mcp_root + "/qnh-a");
	_mcp_qnh_b.set_path (mcp_root + "/qnh-b");
	_mcp_qnh_hpa.set_path (mcp_root + "/qnh-hpa");
	_mcp_std.set_path (mcp_root + "/std");
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

	_mcp_course_display.set_path ("/settings/course/magnetic.integer");

	_setting_efis_fpv_visible.set_path ("/settings/efis/fpv-visible");
	_setting_efis_fpv_visible.set_default (false);
	_setting_efis_show_metric.set_path ("/settings/efis/show-metric");
	_setting_efis_show_metric.set_default (false);
	_setting_efis_fd_visible.set_path ("/settings/efis/cmd-guidance-visible");
	_setting_efis_fd_visible.set_default (false);
	_setting_efis_appr_visible.set_path ("/settings/efis/approach-reference-visible");
	_setting_efis_appr_visible.set_default (false);
	_setting_pressure_qnh.set_path ("/settings/pressure/qnh");
	_setting_pressure_qnh.set_default (29.92_inHg);
	_setting_pressure_display_hpa.set_path ("/settings/efis/display-hpa");
	_setting_pressure_display_hpa.set_default (true);
	_setting_pressure_use_std.set_path ("/settings/pressure/use-std");
	_setting_pressure_use_std.set_default (false);
	_setting_minimums_amsl.set_path ("/settings/minimums/amsl");
	_setting_minimums_setting.set_path ("/settings/minimums/setting");
	_setting_minimums_type.set_path ("/settings/minimums/type");
	_setting_hsi_display_true_heading.set_path ("/settings/hsi/display-true-heading");
	_setting_hsi_display_true_heading.set_default (false);
	_setting_hsi_center_on_track.set_path ("/settings/hsi/center-on-track");
	_setting_hsi_center_on_track.set_default (true);
	_setting_hsi_display_mode_mfd.set_path ("/settings/hsi/display-mode/mfd");
	_setting_hsi_display_mode_mfd.set_default (0);
	_setting_hsi_range.set_path ("/settings/hsi/range");
	_setting_hsi_range.set_default (1_nm);
	_setting_hsi_home_track_visible.set_path ("/settings/hsi/home-track-visible");
	_setting_hsi_home_track_visible.set_default (false);
	_setting_course.set_path ("/settings/course/magnetic");
	_setting_course_visible.set_path ("/settings/course/visible");

	prepare_efis_settings();

	_observables = {
		&_mcp_appr,
		&_mcp_fd,
		&_mcp_htrk,
		&_mcp_qnh_hpa,
		&_mcp_std,
		&_mcp_metric,
		&_mcp_fpv,
		&_mcp_range_ctr,
		&_mcp_hdg_trk,
		&_mcp_mag_tru,
		&_mcp_course_hide,
	};

	_rotary_decoders = {
		_mcp_mins_decoder.get(),
		_mcp_qnh_decoder.get(),
		_mcp_range_decoder.get(),
		_mcp_course_decoder.get(),
	};

	for (auto* decoder: _rotary_decoders)
		decoder->call (0);

	solve_minimums();
	solve_pressure();
	solve_course();

	// EFIS panel

	_efis_mins_mode_button = std::make_unique<Button> ("/panels/mcp/efis/button.mins-mode", [&]() {
		if (_minimums_type == MinimumsType::Baro)
			_minimums_type = MinimumsType::Radio;
		else
			_minimums_type = MinimumsType::Baro;
		solve_minimums();
	});

	// COURSE panel

	_navaid_select_panel = Unique<ButtonOptions> (new ButtonOptions ("/settings/navaid/selected-main", {
		{ "/panels/mcp/navaid/button.off", "/panels/mcp/navaid/led.off", -1, true },
		{ "/panels/mcp/navaid/button.ils", "/panels/mcp/navaid/led.ils", 0 },
		{ "/panels/mcp/navaid/button.vor-l", "/panels/mcp/navaid/led.vor-l", 1 },
		{ "/panels/mcp/navaid/button.vor-r", "/panels/mcp/navaid/led.vor-r", 2 },
	}));

	// NAVAID L/R panel

	_navaid_left_panel = Unique<ButtonOptions> (new ButtonOptions ("/settings/navaid/selected-left", {
		{ "/panels/mcp/navaid-left/button.off", "/panels/mcp/navaid-left/led.off", -1, true },
		{ "/panels/mcp/navaid-left/button.vor", "/panels/mcp/navaid-left/led.vor", 0 },
		{ "/panels/mcp/navaid-left/button.home", "/panels/mcp/navaid-left/led.home", 1 },
	}));

	_navaid_right_panel = Unique<ButtonOptions> (new ButtonOptions ("/settings/navaid/selected-right", {
		{ "/panels/mcp/navaid-right/button.off", "/panels/mcp/navaid-right/led.off", -1, true },
		{ "/panels/mcp/navaid-right/button.vor", "/panels/mcp/navaid-right/led.vor", 0 },
		{ "/panels/mcp/navaid-right/button.home", "/panels/mcp/navaid-right/led.home", 1 },
	}));

	// MFD panel

	_mfd_panel = Unique<ButtonOptions> (new ButtonOptions ("/settings/efis/mfd-mode", {
		{ "/panels/mcp/mfd/button.eicas", "/panels/mcp/mfd/led.eicas", 0, true },
		{ "/panels/mcp/mfd/button.nd", "/panels/mcp/mfd/led.nd", 1 },
		{ "/panels/mcp/mfd/button.chkl", "/panels/mcp/mfd/led.chkl", 2 },
		{ "/panels/mcp/mfd/button.elec", "/panels/mcp/mfd/led.elec", 3 },
	}));

	// AFCS/FBW panel

	_afcs_ap_button = std::make_unique<ToggleButton> ("/panels/mcp/afcs/button.ap", "/panels/mcp/afcs/led.ap");
	_afcs_ap_button->set_callback ([&](bool state) {
		if (state)
			_setting_efis_fd_visible.write (true);
	});
}


void
State::data_updated()
{
	for (ObservableBase* o: _observables)
		o->process();
	for (Xefis::RotaryDecoder* r: _rotary_decoders)
		r->data_updated();

	static std::vector<Action*> actions = {
		_efis_mins_mode_button.get(),
		_navaid_select_panel.get(),
		_navaid_left_panel.get(),
		_navaid_right_panel.get(),
		_afcs_ap_button.get(),
		_mfd_panel.get(),
	};

	for (auto* a: actions)
		a->data_updated();
}


void
State::prepare_efis_settings()
{
	_mcp_mins_decoder = std::make_unique<Xefis::RotaryDecoder> (_mcp_mins_a, _mcp_mins_b, [this](int delta) {
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
	});

	make_toggle (_mcp_appr, _setting_efis_appr_visible);
	make_toggle (_mcp_fd, _setting_efis_fd_visible);
	make_toggle (_mcp_htrk, _setting_hsi_home_track_visible);

	_mcp_qnh_decoder = std::make_unique<Xefis::RotaryDecoder> (_mcp_qnh_a, _mcp_qnh_b, [this](int delta) {
		if (*_setting_pressure_display_hpa)
			_qnh_setting += QNHhPaStep * delta;
		else
			_qnh_setting += QNHinHgStep * delta;

		solve_pressure();
	});

	make_toggle (_mcp_qnh_hpa, _setting_pressure_display_hpa);
	make_toggle (_mcp_std, _setting_pressure_use_std);
	make_toggle (_mcp_metric, _setting_efis_show_metric);
	make_toggle (_mcp_fpv, _setting_efis_fpv_visible);

	_mcp_range_decoder = std::make_unique<Xefis::RotaryDecoder> (_mcp_range_a, _mcp_range_b, [this](int delta) {
		int range_nm = Xefis::symmetric_round (_setting_hsi_range->nm());
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
	});

	make_switch (_mcp_range_ctr, [this]() {
		_setting_hsi_display_mode_mfd.write ((_setting_hsi_display_mode_mfd.read (0) + 1) % 2);
	});

	make_toggle (_mcp_hdg_trk, _setting_hsi_center_on_track);
	make_toggle (_mcp_mag_tru, _setting_hsi_display_true_heading);

	_mcp_course_decoder = std::make_unique<Xefis::RotaryDecoder> (_mcp_course_a, _mcp_course_b, [this](int delta) {
		_course = Xefis::floored_mod (_course + 1_deg * delta, 360_deg);
		int course = Xefis::symmetric_round (_course.deg());
		if (course == 0)
			course = 360;
		_mcp_course_display.write (course);
		solve_course();
	});

	make_switch (_mcp_course_hide, [this]() {
		_course_visible = !_course_visible;
		solve_course();
	});
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
}


void
State::solve_course()
{
	_setting_course.write (_course);
	_setting_course_visible.write (_course_visible);
}


void
State::make_switch (Observable<Xefis::PropertyBoolean>& bool_observable, std::function<void()> callback)
{
	bool_observable.set_callback ([callback,this](Xefis::PropertyBoolean& prop) {
		if (*prop)
			callback();
	});
}


void
State::make_toggle (Observable<Xefis::PropertyBoolean>& bool_observable, Xefis::PropertyBoolean& target_switch)
{
	bool_observable.set_callback ([&](Xefis::PropertyBoolean& prop) {
		if (*prop)
			target_switch.write (!*target_switch);
	});
}


void
State::make_int_writer (Observable<Xefis::PropertyBoolean>& bool_observable, Xefis::PropertyInteger& target_property, int value)
{
	bool_observable.set_callback ([&target_property,value,this](Xefis::PropertyBoolean& prop) {
		if (*prop)
			target_property.write (value);
	});
}


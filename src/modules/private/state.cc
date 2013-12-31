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
	// TODO not hardcoded
	std::string mcp_root = "/panels/mcp";

	_mcp_mins_a.set_path (mcp_root + "/mins-a");
	_mcp_mins_b.set_path (mcp_root + "/mins-b");
	_mcp_mins_mode.set_path (mcp_root + "/mins-mode");
	_mcp_ap.set_path (mcp_root + "/ap");
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

	_setting_efis_fpv_visible.set_path ("/settings/efis/fpv-visible");
	_setting_efis_fpv_visible.set_default (false);
	_setting_efis_show_metric.set_path ("/settings/efis/show-metric");
	_setting_efis_show_metric.set_default (false);
	_setting_efis_fd_visible.set_path ("/settings/efis/flight-director-visible");
	_setting_efis_fd_visible.set_default (false);
	_setting_efis_appr_visible.set_path ("/settings/efis/approach-reference-visible");
	_setting_efis_appr_visible.set_default (false);
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
	_setting_lights_strobe.set_path ("/settings/lights/strobe");
	_setting_lights_strobe.set_default (false);
	_setting_lights_position.set_path ("/settings/lights/position");
	_setting_lights_position.set_default (false);
	_setting_lights_landing.set_path ("/settings/lights/landing");
	_setting_lights_landing.set_default (false);

	prepare_efis_settings();
	prepare_lights_gpws_mfd_panels();

	_observables = {
		&_mcp_mins_mode,
		&_mcp_ap,
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
	};

	for (auto* decoder: _rotary_decoders)
		decoder->call (0);

	solve_minimums();
	solve_pressure();
	solve_course();
}


void
State::data_updated()
{
	for (ObservableBase* o: _observables)
		o->process();
	for (Xefis::RotaryEncoder* r: _rotary_decoders)
		r->data_updated();
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

	make_switch (_mcp_ap, [this]() {
		_setting_efis_fd_visible.write (true);
	});

	make_toggle (_mcp_appr, _setting_efis_appr_visible);
	make_toggle (_mcp_fd, _setting_efis_fd_visible);
	make_toggle (_mcp_htrk, _setting_hsi_home_track_visible);

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
	make_toggle (_mcp_metric, _setting_efis_show_metric);
	make_toggle (_mcp_fpv, _setting_efis_fpv_visible);

	_mcp_range_decoder = std::make_unique<Xefis::RotaryEncoder> (_mcp_range_a, _mcp_range_b, [this](int delta) {
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

		signal_data_updated();
	});

	make_switch (_mcp_range_ctr, [this]() {
		_setting_hsi_display_mode_mfd.write ((_setting_hsi_display_mode_mfd.read (0) + 1) % 2);
	});

	make_toggle (_mcp_hdg_trk, _setting_hsi_center_on_track);
	make_toggle (_mcp_mag_tru, _setting_hsi_display_true_heading);

	_mcp_course_decoder = std::make_unique<Xefis::RotaryEncoder> (_mcp_course_a, _mcp_course_b, [this](int delta) {
		_course = Xefis::floored_mod (_course + 1_deg * delta, 360_deg);
		int course = Xefis::symmetric_round (_course.deg());
		if (course == 0)
			course = 360;
		_mcp_course_display.write (course);
		solve_course();
		signal_data_updated();
	});

	make_switch (_mcp_course_hide, [this]() {
		_course_visible = !_course_visible;
		solve_course();
	});
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
State::solve_course()
{
	if (_course_visible)
		_setting_course.write (_course);
	else
		_setting_course.set_nil();
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


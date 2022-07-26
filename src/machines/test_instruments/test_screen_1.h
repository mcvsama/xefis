/* vim:ts=4
 *
 * Copyleft 2008…2017  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__MACHINES__TEST_INSTRUMENTS__TEST_SCREEN_1_H__INCLUDED
#define XEFIS__MACHINES__TEST_INSTRUMENTS__TEST_SCREEN_1_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/graphics.h>
#include <xefis/core/machine.h>
#include <xefis/core/screen.h>
#include <xefis/modules/instruments/adi.h>
#include <xefis/modules/instruments/flaps.h>
#include <xefis/modules/instruments/gear.h>
#include <xefis/modules/instruments/hsi.h>
#include <xefis/modules/instruments/horizontal_trim.h>
#include <xefis/modules/instruments/label.h>
#include <xefis/modules/instruments/linear_gauge.h>
#include <xefis/modules/instruments/vertical_trim.h>
#include <xefis/modules/instruments/radial_gauge.h>
#include <xefis/support/earth/navigation/navaid_storage.h>

// Neutrino:
#include <neutrino/work_performer.h>

// Standard:
#include <cstddef>
#include <memory>


class TestScreen1: public xf::Screen
{
  public:
	// Ctor
	explicit
	TestScreen1 (xf::ScreenSpec const&, xf::Graphics const&, xf::NavaidStorage const&, xf::Machine&, xf::Logger const& logger);

  private:
	void
	register_instruments();

	void
	place_instruments();

	void
	connect_instruments();

  private:
	xf::Logger											_logger;
	xf::Graphics const&									_graphics;
	xf::NavaidStorage const&							_navaid_storage;
	xf::WorkPerformer									_adi_work_performer;
	xf::WorkPerformer									_hsi_work_performer;
	xf::WorkPerformer									_others_work_performer;

  public:
	xf::Registrant<ADI>									adi;
	xf::Registrant<HSI>									hsi;
	xf::Registrant<RadialGauge<si::Force>>				engine_l_thrust;
	xf::Registrant<RadialGauge<si::AngularVelocity>>	engine_l_speed;
	xf::Registrant<RadialGauge<si::Power>>				engine_l_power;
	xf::Registrant<LinearGauge<si::Current>>			engine_l_current;
	xf::Registrant<LinearGauge<si::Temperature>>		engine_l_temperature;
	xf::Registrant<LinearGauge<si::Voltage>>			engine_l_voltage;
	xf::Registrant<LinearGauge<si::Acceleration>>		engine_l_vibration;
	xf::Registrant<RadialGauge<si::Force>>				engine_r_thrust;
	xf::Registrant<RadialGauge<si::AngularVelocity>>	engine_r_speed;
	xf::Registrant<RadialGauge<si::Power>>				engine_r_power;
	xf::Registrant<LinearGauge<si::Current>>			engine_r_current;
	xf::Registrant<LinearGauge<si::Temperature>>		engine_r_temperature;
	xf::Registrant<LinearGauge<si::Voltage>>			engine_r_voltage;
	xf::Registrant<LinearGauge<si::Acceleration>>		engine_r_vibration;
	xf::Registrant<Label>								label_thrust;
	xf::Registrant<Label>								label_pwr;
	xf::Registrant<Label>								label_n1;
	xf::Registrant<Label>								label_amps;
	xf::Registrant<Label>								label_temp;
	xf::Registrant<Label>								label_volts;
	xf::Registrant<Label>								label_vib;
	xf::Registrant<Gear>								gear;
	xf::Registrant<Flaps>								flaps;
	xf::Registrant<VerticalTrim>						vertical_trim;
	xf::Registrant<HorizontalTrim>						horizontal_trim;
	xf::Registrant<LinearGauge<double>>					glide_ratio;
	xf::Registrant<Label>								glide_ratio_label;
	xf::Registrant<LinearGauge<double>>					load_factor;
	xf::Registrant<Label>								load_factor_label;
};

#endif


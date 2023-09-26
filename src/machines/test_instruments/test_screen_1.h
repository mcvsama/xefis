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
	xf::Logger							_logger;
	xf::Graphics const&					_graphics;
	xf::NavaidStorage const&			_navaid_storage;

  public:
	ADI									adi;
	HSI									hsi;
	RadialGauge<si::Force>				engine_l_thrust;
	RadialGauge<si::AngularVelocity>	engine_l_speed;
	RadialGauge<si::Power>				engine_l_power;
	LinearGauge<si::Current>			engine_l_current;
	LinearGauge<si::Temperature>		engine_l_temperature;
	LinearGauge<si::Voltage>			engine_l_voltage;
	LinearGauge<si::Acceleration>		engine_l_vibration;
	RadialGauge<si::Force>				engine_r_thrust;
	RadialGauge<si::AngularVelocity>	engine_r_speed;
	RadialGauge<si::Power>				engine_r_power;
	LinearGauge<si::Current>			engine_r_current;
	LinearGauge<si::Temperature>		engine_r_temperature;
	LinearGauge<si::Voltage>			engine_r_voltage;
	LinearGauge<si::Acceleration>		engine_r_vibration;
	Label								label_thrust;
	Label								label_pwr;
	Label								label_n1;
	Label								label_amps;
	Label								label_temp;
	Label								label_volts;
	Label								label_vib;
	Gear								gear;
	Flaps								flaps;
	VerticalTrim						vertical_trim;
	HorizontalTrim						horizontal_trim;
	LinearGauge<double>					glide_ratio;
	Label								glide_ratio_label;
	LinearGauge<double>					load_factor;
	Label								load_factor_label;

  private:
	// Since work performers perform tasks provided by modules (painting stuff, etc),
	// they should be deleted first, to make sure no task is being executed when the module
	// itself is deleted.
	xf::WorkPerformer					_adi_work_performer;
	xf::WorkPerformer					_hsi_work_performer;
	xf::WorkPerformer					_others_work_performer;
};

#endif


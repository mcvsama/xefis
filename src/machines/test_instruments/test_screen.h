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

#ifndef XEFIS__MACHINES__TEST_INSTRUMENTS__TEST_SCREEN_H__INCLUDED
#define XEFIS__MACHINES__TEST_INSTRUMENTS__TEST_SCREEN_H__INCLUDED

// Standard:
#include <cstddef>
#include <memory>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/modules/instruments/adi.h>
#include <xefis/modules/instruments/gear.h>
#include <xefis/modules/instruments/label.h>
#include <xefis/modules/instruments/radial_indicator.h>
#include <xefis/core/screen.h>


class TestScreen: public xf::Screen
{
  public:
	// Ctor
	explicit
	TestScreen (xf::ScreenSpec const&);

	void
	create_instruments();

  public:
	std::unique_ptr<ADI_IO>												adi_io						{ std::make_unique<ADI_IO>() };
	std::unique_ptr<RadialIndicatorIO<si::Force>>						engine_l_thrust_io			{ std::make_unique<RadialIndicatorIO<si::Force>>() };
	std::unique_ptr<RadialIndicatorIO<si::AngularVelocity>>				engine_l_speed_io			{ std::make_unique<RadialIndicatorIO<si::AngularVelocity>>() };
	std::unique_ptr<RadialIndicatorIO<si::Temperature>>					engine_l_temperature_io		{ std::make_unique<RadialIndicatorIO<si::Temperature>>() };
	std::unique_ptr<RadialIndicatorIO<si::Power>>						engine_l_power_io			{ std::make_unique<RadialIndicatorIO<si::Power>>() };
	std::unique_ptr<RadialIndicatorIO<si::Force>>						engine_r_thrust_io			{ std::make_unique<RadialIndicatorIO<si::Force>>() };
	std::unique_ptr<RadialIndicatorIO<si::AngularVelocity>>				engine_r_speed_io			{ std::make_unique<RadialIndicatorIO<si::AngularVelocity>>() };
	std::unique_ptr<RadialIndicatorIO<si::Temperature>>					engine_r_temperature_io		{ std::make_unique<RadialIndicatorIO<si::Temperature>>() };
	std::unique_ptr<RadialIndicatorIO<si::Power>>						engine_r_power_io			{ std::make_unique<RadialIndicatorIO<si::Power>>() };
	std::unique_ptr<LabelIO>											label_thr_io				{ std::make_unique<LabelIO>() };
	std::unique_ptr<LabelIO>											label_n1_io					{ std::make_unique<LabelIO>() };
	std::unique_ptr<LabelIO>											label_temp_io				{ std::make_unique<LabelIO>() };
	std::unique_ptr<LabelIO>											label_pwr_io				{ std::make_unique<LabelIO>() };
	std::unique_ptr<GearIO>												gear_io						{ std::make_unique<GearIO>() };

  private:
	std::optional<xf::Registrant<ADI>>									_adi;
	std::optional<xf::Registrant<RadialIndicator<si::Force>>>			_engine_l_thrust;
	std::optional<xf::Registrant<RadialIndicator<si::AngularVelocity>>>	_engine_l_speed;
	std::optional<xf::Registrant<RadialIndicator<si::Temperature>>>		_engine_l_temperature;
	std::optional<xf::Registrant<RadialIndicator<si::Power>>>			_engine_l_power;
	std::optional<xf::Registrant<RadialIndicator<si::Force>>>			_engine_r_thrust;
	std::optional<xf::Registrant<RadialIndicator<si::AngularVelocity>>>	_engine_r_speed;
	std::optional<xf::Registrant<RadialIndicator<si::Temperature>>>		_engine_r_temperature;
	std::optional<xf::Registrant<RadialIndicator<si::Power>>>			_engine_r_power;
	std::optional<xf::Registrant<Label>>								_label_thr;
	std::optional<xf::Registrant<Label>>								_label_n1;
	std::optional<xf::Registrant<Label>>								_label_temp;
	std::optional<xf::Registrant<Label>>								_label_pwr;
	std::optional<xf::Registrant<Gear>>									_gear;
};

#endif


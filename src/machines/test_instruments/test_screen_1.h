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

// Standard:
#include <cstddef>
#include <memory>

// Neutrino:
#include <neutrino/work_performer.h>

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


class TestScreen1: public xf::Screen
{
  public:
	// Ctor
	explicit
	TestScreen1 (xf::ScreenSpec const&, xf::Graphics const&, xf::NavaidStorage const&, xf::Machine&, xf::Logger const& logger);

	void
	create_instruments();

  public:
	std::unique_ptr<ADI_IO>											adi_io						{ std::make_unique<ADI_IO>() };
	std::unique_ptr<HSI_IO>											hsi_io						{ std::make_unique<HSI_IO>() };
	std::unique_ptr<RadialGaugeIO<si::Force>>						engine_l_thrust_io			{ std::make_unique<RadialGaugeIO<si::Force>>() };
	std::unique_ptr<RadialGaugeIO<si::AngularVelocity>>				engine_l_speed_io			{ std::make_unique<RadialGaugeIO<si::AngularVelocity>>() };
	std::unique_ptr<RadialGaugeIO<si::Power>>						engine_l_power_io			{ std::make_unique<RadialGaugeIO<si::Power>>() };
	std::unique_ptr<LinearGaugeIO<si::Current>>						engine_l_current_io			{ std::make_unique<LinearGaugeIO<si::Current>>() };
	std::unique_ptr<LinearGaugeIO<si::Temperature>>					engine_l_temperature_io		{ std::make_unique<LinearGaugeIO<si::Temperature>>() };
	std::unique_ptr<LinearGaugeIO<si::Acceleration>>				engine_l_vibration_io		{ std::make_unique<LinearGaugeIO<si::Acceleration>>() };
	std::unique_ptr<LinearGaugeIO<si::Voltage>>						engine_l_voltage_io			{ std::make_unique<LinearGaugeIO<si::Voltage>>() };
	std::unique_ptr<RadialGaugeIO<si::Force>>						engine_r_thrust_io			{ std::make_unique<RadialGaugeIO<si::Force>>() };
	std::unique_ptr<RadialGaugeIO<si::AngularVelocity>>				engine_r_speed_io			{ std::make_unique<RadialGaugeIO<si::AngularVelocity>>() };
	std::unique_ptr<RadialGaugeIO<si::Power>>						engine_r_power_io			{ std::make_unique<RadialGaugeIO<si::Power>>() };
	std::unique_ptr<LinearGaugeIO<si::Current>>						engine_r_current_io			{ std::make_unique<LinearGaugeIO<si::Current>>() };
	std::unique_ptr<LinearGaugeIO<si::Temperature>>					engine_r_temperature_io		{ std::make_unique<LinearGaugeIO<si::Temperature>>() };
	std::unique_ptr<LinearGaugeIO<si::Acceleration>>				engine_r_vibration_io		{ std::make_unique<LinearGaugeIO<si::Acceleration>>() };
	std::unique_ptr<LinearGaugeIO<si::Voltage>>						engine_r_voltage_io			{ std::make_unique<LinearGaugeIO<si::Voltage>>() };
	std::unique_ptr<LabelIO>										label_pwr_io				{ std::make_unique<LabelIO>() };
	std::unique_ptr<LabelIO>										label_n1_io					{ std::make_unique<LabelIO>() };
	std::unique_ptr<LabelIO>										label_temp_io				{ std::make_unique<LabelIO>() };
	std::unique_ptr<LabelIO>										label_amps_io				{ std::make_unique<LabelIO>() };
	std::unique_ptr<LabelIO>										label_thrust_io				{ std::make_unique<LabelIO>() };
	std::unique_ptr<LabelIO>										label_vib_io				{ std::make_unique<LabelIO>() };
	std::unique_ptr<LabelIO>										label_volts_io				{ std::make_unique<LabelIO>() };
	std::unique_ptr<GearIO>											gear_io						{ std::make_unique<GearIO>() };
	std::unique_ptr<FlapsIO>										flaps_io					{ std::make_unique<FlapsIO>() };
	std::unique_ptr<VerticalTrimIO>									vertical_trim_io			{ std::make_unique<VerticalTrimIO>() };
	std::unique_ptr<HorizontalTrimIO>								horizontal_trim_io			{ std::make_unique<HorizontalTrimIO>() };
	std::unique_ptr<LinearGaugeIO<double>>							glide_ratio_io				{ std::make_unique<LinearGaugeIO<double>>() };
	std::unique_ptr<LabelIO>										glide_ratio_label_io		{ std::make_unique<LabelIO>() };
	std::unique_ptr<LinearGaugeIO<double>>							load_factor_io				{ std::make_unique<LinearGaugeIO<double>>() };
	std::unique_ptr<LabelIO>										load_factor_label_io		{ std::make_unique<LabelIO>() };

  private:
	xf::Logger														_logger;
	xf::Graphics const&												_graphics;
	xf::NavaidStorage const&										_navaid_storage;
	xf::WorkPerformer												_adi_work_performer;
	xf::WorkPerformer												_hsi_work_performer;
	xf::WorkPerformer												_others_work_performer;
	// Instruments:
	std::optional<xf::Registrant<ADI>>								_adi;
	std::optional<xf::Registrant<HSI>>								_hsi;
	std::optional<xf::Registrant<RadialGauge<si::Force>>>			_engine_l_thrust;
	std::optional<xf::Registrant<RadialGauge<si::AngularVelocity>>>	_engine_l_speed;
	std::optional<xf::Registrant<RadialGauge<si::Power>>>			_engine_l_power;
	std::optional<xf::Registrant<LinearGauge<si::Current>>>			_engine_l_current;
	std::optional<xf::Registrant<LinearGauge<si::Temperature>>>		_engine_l_temperature;
	std::optional<xf::Registrant<LinearGauge<si::Acceleration>>>	_engine_l_vibration;
	std::optional<xf::Registrant<LinearGauge<si::Voltage>>>			_engine_l_voltage;
	std::optional<xf::Registrant<RadialGauge<si::Force>>>			_engine_r_thrust;
	std::optional<xf::Registrant<RadialGauge<si::AngularVelocity>>>	_engine_r_speed;
	std::optional<xf::Registrant<RadialGauge<si::Power>>>			_engine_r_power;
	std::optional<xf::Registrant<LinearGauge<si::Current>>>			_engine_r_current;
	std::optional<xf::Registrant<LinearGauge<si::Temperature>>>		_engine_r_temperature;
	std::optional<xf::Registrant<LinearGauge<si::Acceleration>>>	_engine_r_vibration;
	std::optional<xf::Registrant<LinearGauge<si::Voltage>>>			_engine_r_voltage;
	std::optional<xf::Registrant<Label>>							_label_thr;
	std::optional<xf::Registrant<Label>>							_label_n1;
	std::optional<xf::Registrant<Label>>							_label_temp;
	std::optional<xf::Registrant<Label>>							_label_pwr;
	std::optional<xf::Registrant<Label>>							_label_amps;
	std::optional<xf::Registrant<Label>>							_label_vib;
	std::optional<xf::Registrant<Label>>							_label_volts;
	std::optional<xf::Registrant<Gear>>								_gear;
	std::optional<xf::Registrant<Flaps>>							_flaps;
	std::optional<xf::Registrant<VerticalTrim>>						_vertical_trim;
	std::optional<xf::Registrant<HorizontalTrim>>					_horizontal_trim;
	std::optional<xf::Registrant<LinearGauge<double>>>				_glide_ratio;
	std::optional<xf::Registrant<Label>>							_glide_ratio_label;
	std::optional<xf::Registrant<LinearGauge<double>>>				_load_factor;
	std::optional<xf::Registrant<Label>>							_load_factor_label;
};

#endif


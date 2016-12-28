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

#ifndef XEFIS__MODULES__SYSTEMS__ADC_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__ADC_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/module.h>
#include <xefis/core/v2/property.h>
#include <xefis/core/v2/property_observer.h>
#include <xefis/core/v2/setting.h>
#include <xefis/utility/datatable2d.h>
#include <xefis/utility/smoother.h>
#include <xefis/utility/lookahead.h>
#include <xefis/support/airframe/airframe.h>


/**
 * Computations are reliable up to 36,000 ft of altitude and up to about speed of Mach 0.3.
 */
class AirDataComputer: public x2::Module
{
  public:
	/*
	 * Settings
	 */

	x2::Setting<si::Velocity>				setting_ias_valid_minimum			{ this };
	x2::Setting<si::Velocity>				setting_ias_valid_maximum			{ this };
	x2::Setting<bool>						setting_using_ias_sensor			{ this, false };
	x2::Setting<double>						setting_ram_rise_factor				{ this, 0.2 };

	/*
	 * Input
	 */

	x2::PropertyIn<bool>					input_pressure_use_std				{ this, "/settings/pressure/use-std", false };
	x2::PropertyIn<si::Pressure>			input_pressure_qnh					{ this, "/settings/pressure/qnh" };
	x2::PropertyIn<bool>					input_pressure_static_serviceable	{ this, "/sensors/pressure/serviceable" };
	x2::PropertyIn<si::Pressure>			input_pressure_static				{ this, "/sensors/pressure/static" };
	x2::PropertyIn<si::Pressure>			input_pressure_total				{ this, "/sensors/pressure/total" };
	x2::PropertyIn<bool>					input_ias_serviceable				{ this, "/sensors/airspeed/serviceable" };
	x2::PropertyIn<si::Velocity>			input_ias							{ this, "/sensors/airspeed/ias" };
	x2::PropertyIn<si::Temperature>			input_total_air_temperature			{ this, "/sensors/air-temperature/total" };

	/*
	 * Output
	 */

	x2::PropertyOut<si::Pressure>			output_pressure_dynamic				{ this, "/pressure/dynamic" };
	x2::PropertyOut<bool>					output_altitude_amsl_serviceable	{ this, "/altitude/amsl.serviceable" };
	x2::PropertyOut<si::Length>				output_altitude_amsl				{ this, "/altitude/amsl" };
	x2::PropertyOut<si::Length>				output_altitude_amsl_lookahead		{ this, "/altitude/amsl.lookahead" };
	x2::PropertyOut<si::Length>				output_altitude_amsl_qnh			{ this, "/altitude/amsl.qnh" };
	x2::PropertyOut<si::Length>				output_altitude_amsl_std			{ this, "/altitude/amsl.std" };
	x2::PropertyOut<si::Length>				output_density_altitude				{ this, "/density-altitude" };
	x2::PropertyOut<si::Density>			output_air_density_static			{ this, "/air-density/static" };
	x2::PropertyOut<bool>					output_speed_ias_serviceable		{ this, "/speed/ias.serviceable" };
	x2::PropertyOut<si::Velocity>			output_speed_ias					{ this, "/speed/ias" };
	x2::PropertyOut<si::Velocity>			output_speed_ias_lookahead			{ this, "/speed/ias.lookahead" };
	x2::PropertyOut<si::Velocity>			output_speed_tas					{ this, "/speed/tas" };
	x2::PropertyOut<double>					output_speed_mach					{ this, "/speed/mach" };
	x2::PropertyOut<si::Velocity>			output_speed_sound					{ this, "/speed/sound" };
	x2::PropertyOut<bool>					output_vertical_speed_serviceable	{ this, "/vertical-speed/serviceable" };
	x2::PropertyOut<si::Velocity>			output_vertical_speed				{ this, "/vertical-speed/speed" };
	x2::PropertyOut<si::Temperature>		output_static_air_temperature		{ this, "/air-temperature/static" };
	x2::PropertyOut<si::DynamicViscosity>	output_dynamic_viscosity			{ this, "/viscosity/dynamic" };
	x2::PropertyOut<double>					output_reynolds_number				{ this, "/reynolds-number" };

  public:
	// Ctor
	AirDataComputer (xf::Airframe*, std::string const& instance = {});

  protected:
	// Module API
	void
	process (x2::Cycle const&) override;

  private:
	void
	compute_altitude();

	void
	compute_ias();

	void
	compute_ias_lookahead();

	void
	compute_mach();

	void
	compute_sat_and_viscosity();

	void
	compute_density_altitude();

	void
	compute_sound_speed();

	void
	compute_tas();

	void
	compute_vertical_speed();

	void
	compute_reynolds();

  private:
	bool						_ias_in_valid_range					= false;
	bool						_prev_use_standard_pressure			= false;
	si::Time					_hide_alt_lookahead_until			= 0_s;
	si::Length					_prev_altitude_amsl					= 0_ft;
	xf::Airframe*				_airframe							= nullptr;
	Unique<xf::Datatable2D<Temperature, DynamicViscosity>>
								_temperature_to_dynamic_viscosity;
	// Note: PropertyObservers depend on Smoothers, so first Smoothers must be defined,
	// then PropertyObservers, to ensure correct order of destruction.
	xf::Smoother<si::Velocity>	_vertical_speed_smoother			{ 1_s };
	xf::Smoother<si::Length>	_altitude_amsl_smoother				{ 500_ms };
	xf::Smoother<si::Length>	_altitude_amsl_qnh_smoother			{ 500_ms };
	xf::Smoother<si::Length>	_altitude_amsl_std_smoother			{ 500_ms };
	xf::Smoother<si::Velocity>	_speed_ias_smoother					{ 100_ms };
	xf::Smoother<si::Length>	_altitude_amsl_lookahead_i_smoother	{ 100_ms };
	xf::Smoother<si::Length>	_altitude_amsl_lookahead_o_smoother	{ 500_ms };
	xf::Smoother<si::Velocity>	_speed_ias_lookahead_i_smoother		{ 100_ms };
	xf::Smoother<si::Velocity>	_speed_ias_lookahead_o_smoother		{ 1000_ms };
	xf::Lookahead<si::Length>	_altitude_amsl_estimator			{ 10_s };
	xf::Lookahead<si::Velocity>	_speed_ias_estimator				{ 10_s };
	x2::PropertyObserver		_altitude_computer;
	x2::PropertyObserver		_density_altitude_computer;
	x2::PropertyObserver		_ias_computer;
	x2::PropertyObserver		_ias_lookahead_computer;
	x2::PropertyObserver		_sound_speed_computer;
	x2::PropertyObserver		_tas_computer;
	x2::PropertyObserver		_mach_computer;
	x2::PropertyObserver		_sat_computer;
	x2::PropertyObserver		_vertical_speed_computer;
	x2::PropertyObserver		_reynolds_computer;
};

#endif

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


class AirDataComputerIO: public v2::ModuleIO
{
  public:
	/*
	 * Settings
	 */

	v2::Setting<si::Velocity>				ias_valid_minimum			{ this, "ias_valid_minimum" };
	v2::Setting<si::Velocity>				ias_valid_maximum			{ this, "ias_valid_maximum" };
	v2::Setting<bool>						using_ias_sensor			{ this, "using_ias_sensor", false };
	v2::Setting<double>						ram_rise_factor				{ this, "ram_rise_factor", 0.2 };

	/*
	 * Input
	 */

	v2::PropertyIn<bool>					pressure_use_std			{ this, "/settings/pressure/use-std", false };
	v2::PropertyIn<si::Pressure>			pressure_qnh				{ this, "/settings/pressure/qnh" };
	v2::PropertyIn<bool>					pressure_static_serviceable	{ this, "/sensors/pressure/serviceable" };
	v2::PropertyIn<si::Pressure>			pressure_static				{ this, "/sensors/pressure/static" };
	v2::PropertyIn<si::Pressure>			pressure_total				{ this, "/sensors/pressure/total" };
	v2::PropertyIn<bool>					ias_serviceable				{ this, "/sensors/airspeed/serviceable" };
	v2::PropertyIn<si::Velocity>			ias							{ this, "/sensors/airspeed/ias" };
	v2::PropertyIn<si::Temperature>			total_air_temperature		{ this, "/sensors/air-temperature/total" };

	/*
	 * Output
	 */

	v2::PropertyOut<si::Pressure>			pressure_dynamic			{ this, "/pressure/dynamic" };
	v2::PropertyOut<bool>					altitude_amsl_serviceable	{ this, "/altitude/amsl.serviceable" };
	v2::PropertyOut<si::Length>				altitude_amsl				{ this, "/altitude/amsl" };
	v2::PropertyOut<si::Length>				altitude_amsl_lookahead		{ this, "/altitude/amsl.lookahead" };
	v2::PropertyOut<si::Length>				altitude_amsl_qnh			{ this, "/altitude/amsl.qnh" };
	v2::PropertyOut<si::Length>				altitude_amsl_std			{ this, "/altitude/amsl.std" };
	v2::PropertyOut<si::Length>				density_altitude			{ this, "/density-altitude" };
	v2::PropertyOut<si::Density>			air_density_static			{ this, "/air-density/static" };
	v2::PropertyOut<bool>					speed_ias_serviceable		{ this, "/speed/ias.serviceable" };
	v2::PropertyOut<si::Velocity>			speed_ias					{ this, "/speed/ias" };
	v2::PropertyOut<si::Velocity>			speed_ias_lookahead			{ this, "/speed/ias.lookahead" };
	v2::PropertyOut<si::Velocity>			speed_tas					{ this, "/speed/tas" };
	v2::PropertyOut<si::Velocity>			speed_eas					{ this, "/speed/eas" };
	v2::PropertyOut<double>					speed_mach					{ this, "/speed/mach" };
	v2::PropertyOut<si::Velocity>			speed_sound					{ this, "/speed/sound" };
	v2::PropertyOut<bool>					vertical_speed_serviceable	{ this, "/vertical-speed/serviceable" };
	v2::PropertyOut<si::Velocity>			vertical_speed				{ this, "/vertical-speed/speed" };
	v2::PropertyOut<si::Temperature>		static_air_temperature		{ this, "/air-temperature/static" };
	v2::PropertyOut<si::DynamicViscosity>	dynamic_viscosity			{ this, "/viscosity/dynamic" };
	v2::PropertyOut<double>					reynolds_number				{ this, "/reynolds-number" };
};


/**
 * Computations are reliable up to 36,000 ft of altitude and up to about speed of Mach 0.3.
 */
class AirDataComputer: public v2::Module<AirDataComputerIO>
{
  public:
	// Ctor
	explicit
	AirDataComputer (std::unique_ptr<AirDataComputerIO>, xf::Airframe*, std::string const& instance = {});

  protected:
	// Module API
	void
	process (v2::Cycle const&) override;

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
	compute_eas();

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
	v2::PropertyObserver		_altitude_computer;
	v2::PropertyObserver		_density_altitude_computer;
	v2::PropertyObserver		_ias_computer;
	v2::PropertyObserver		_ias_lookahead_computer;
	v2::PropertyObserver		_sound_speed_computer;
	v2::PropertyObserver		_tas_computer;
	v2::PropertyObserver		_eas_computer;
	v2::PropertyObserver		_mach_computer;
	v2::PropertyObserver		_sat_computer;
	v2::PropertyObserver		_vertical_speed_computer;
	v2::PropertyObserver		_reynolds_computer;
};

#endif

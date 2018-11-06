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
#include <xefis/core/logger.h>
#include <xefis/core/module.h>
#include <xefis/core/property.h>
#include <xefis/core/property_observer.h>
#include <xefis/core/setting.h>
#include <xefis/support/airframe/airframe.h>
#include <xefis/utility/lookahead.h>
#include <xefis/utility/smoother.h>


class AirDataComputerIO: public xf::ModuleIO
{
  public:
	/*
	 * Settings
	 */

	xf::Setting<si::Velocity>				ias_valid_minimum			{ this, "ias_valid_minimum" };
	xf::Setting<si::Velocity>				ias_valid_maximum			{ this, "ias_valid_maximum" };
	xf::Setting<bool>						using_ias_sensor			{ this, "using_ias_sensor", false };
	xf::Setting<double>						ram_rise_factor				{ this, "ram_rise_factor", 0.2 };

	/*
	 * Input
	 */

	xf::PropertyIn<bool>					pressure_use_std			{ this, "settings/pressure/use-std", false };
	xf::PropertyIn<si::Pressure>			pressure_qnh				{ this, "settings/pressure/qnh" };
	xf::PropertyIn<bool>					pressure_static_serviceable	{ this, "sensors/pressure/serviceable" };
	xf::PropertyIn<si::Pressure>			pressure_static				{ this, "sensors/pressure/static" };
	xf::PropertyIn<si::Pressure>			pressure_total				{ this, "sensors/pressure/total" };
	xf::PropertyIn<bool>					ias_serviceable				{ this, "sensors/airspeed/serviceable" };
	xf::PropertyIn<si::Velocity>			sensed_ias					{ this, "sensors/airspeed/sensed-ias" };
	xf::PropertyIn<si::Temperature>			total_air_temperature		{ this, "sensors/air-temperature/total" };

	/*
	 * Output
	 */

	xf::PropertyOut<si::Pressure>			recovered_pressure_total	{ this, "pressure/total" };
	xf::PropertyOut<si::Pressure>			pressure_dynamic			{ this, "pressure/dynamic" };
	xf::PropertyOut<bool>					altitude_amsl_serviceable	{ this, "altitude/amsl.serviceable" };
	xf::PropertyOut<si::Length>				altitude_amsl				{ this, "altitude/amsl" };
	xf::PropertyOut<si::Length>				altitude_amsl_lookahead		{ this, "altitude/amsl.lookahead" };
	xf::PropertyOut<si::Length>				altitude_amsl_qnh			{ this, "altitude/amsl.qnh" };
	xf::PropertyOut<si::Length>				altitude_amsl_std			{ this, "altitude/amsl.std" };
	xf::PropertyOut<si::Length>				density_altitude			{ this, "density-altitude" };
	xf::PropertyOut<si::Density>			air_density_static			{ this, "air-density/static" };
	xf::PropertyOut<bool>					speed_ias_serviceable		{ this, "speed/ias.serviceable" };
	xf::PropertyOut<si::Velocity>			speed_ias					{ this, "speed/ias" };
	xf::PropertyOut<si::Velocity>			speed_ias_lookahead			{ this, "speed/ias.lookahead" };
	xf::PropertyOut<si::Velocity>			speed_tas					{ this, "speed/tas" };
	xf::PropertyOut<si::Velocity>			speed_eas					{ this, "speed/eas" };
	xf::PropertyOut<double>					speed_mach					{ this, "speed/mach" };
	xf::PropertyOut<si::Velocity>			speed_sound					{ this, "speed/sound" };
	xf::PropertyOut<bool>					vertical_speed_serviceable	{ this, "vertical-speed/serviceable" };
	xf::PropertyOut<si::Velocity>			vertical_speed				{ this, "vertical-speed/speed" };
	xf::PropertyOut<si::Temperature>		static_air_temperature		{ this, "air-temperature/static" };
	xf::PropertyOut<si::DynamicViscosity>	dynamic_viscosity			{ this, "viscosity/dynamic" };
	xf::PropertyOut<double>					reynolds_number				{ this, "reynolds-number" };
};


/**
 * Computations are reliable up to 36,000 ft of altitude and up to about speed of Mach 0.3.
 */
class AirDataComputer: public xf::Module<AirDataComputerIO>
{
  private:
	static constexpr char kLoggerScope[]				= "mod::AirDataComputer";

  public:
	// Ctor
	explicit
	AirDataComputer (std::unique_ptr<AirDataComputerIO>, xf::Airframe*, xf::Logger const&, std::string_view const& instance = {});

  protected:
	// Module API
	void
	process (xf::Cycle const&) override;

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
	compute_speed_of_sound();

	void
	compute_tas();

	void
	compute_eas();

	void
	compute_vertical_speed();

	void
	compute_reynolds();

	void
	recover_total_pressure();

  private:
	xf::Logger					_logger;
	bool						_ias_in_valid_range					= false;
	bool						_prev_use_standard_pressure			= false;
	si::Time					_hide_alt_lookahead_until			= 0_s;
	si::Length					_prev_altitude_amsl					= 0_ft;
	xf::Airframe*				_airframe							= nullptr;
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
	xf::PropertyObserver		_total_pressure_computer;
	xf::PropertyObserver		_altitude_computer;
	xf::PropertyObserver		_density_altitude_computer;
	xf::PropertyObserver		_ias_computer;
	xf::PropertyObserver		_ias_lookahead_computer;
	xf::PropertyObserver		_speed_of_sound_computer;
	xf::PropertyObserver		_tas_computer;
	xf::PropertyObserver		_eas_computer;
	xf::PropertyObserver		_mach_computer;
	xf::PropertyObserver		_sat_computer;
	xf::PropertyObserver		_vertical_speed_computer;
	xf::PropertyObserver		_reynolds_computer;
};

#endif

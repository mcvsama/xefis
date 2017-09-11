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

#ifndef XEFIS__MODULES__SYSTEMS__PC_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__PC_H__INCLUDED

// Standard:
#include <cstddef>
#include <array>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/module.h>
#include <xefis/core/v2/property.h>
#include <xefis/core/v2/property_observer.h>
#include <xefis/core/v2/setting.h>
#include <xefis/utility/smoother.h>
#include <xefis/utility/range_smoother.h>


class PerformanceComputerIO: public v2::ModuleIO
{
  public:
	/*
	 * Settings
	 */

	v2::Setting<si::Velocity>			tev_min_ias					{ this, "tev_min_ias", 0_kt };

	/*
	 * Input
	 */

	v2::PropertyIn<si::Velocity>		speed_ias					{ this, "/speed.ias" };
	v2::PropertyIn<si::Velocity>		speed_tas					{ this, "/speed.tas" };
	v2::PropertyIn<si::Velocity>		speed_gs					{ this, "/speed.gs" };
	v2::PropertyIn<si::Velocity>		vertical_speed				{ this, "/vertical-speed" };
	v2::PropertyIn<si::Length>			altitude_amsl_std			{ this, "/altitude.amsl" };
	v2::PropertyIn<si::Angle>			track_lateral_true			{ this, "/track.lateral.true" };
	v2::PropertyIn<si::Angle>			orientation_heading_true	{ this, "/orientation.heading.true" };
	v2::PropertyIn<si::Angle>			magnetic_declination		{ this, "/magnetic-declination" };
	v2::PropertyIn<si::Length>			density_altitude			{ this, "/density-altitude" };
	v2::PropertyIn<si::Density>			air_density_static			{ this, "/air-density.static" };
	v2::PropertyIn<si::Mass>			aircraft_mass				{ this, "/aircraft-mass" };
	v2::PropertyIn<si::Angle>			flaps_angle					{ this, "/flaps-angle" };
	v2::PropertyIn<si::Angle>			spoilers_angle				{ this, "/spoilers-angle" };
	v2::PropertyIn<si::Angle>			aoa_alpha					{ this, "/aoa.alpha" };
	v2::PropertyIn<si::Acceleration>	load						{ this, "/load" };
	v2::PropertyIn<si::Angle>			bank_angle					{ this, "/bank-angle" };
	v2::PropertyIn<si::Acceleration>	y_acceleration				{ this, "/acceleration.y" };
	v2::PropertyIn<si::Acceleration>	z_acceleration				{ this, "/acceleration.x" };

	/*
	 * Output
	 */

	v2::PropertyOut<si::Angle>			wind_from_true				{ this, "/wind.from.true" };
	v2::PropertyOut<si::Angle>			wind_from_magnetic			{ this, "/wind.from.magnetic" };
	v2::PropertyOut<si::Velocity>		wind_tas					{ this, "/wind.tas" };
	v2::PropertyOut<double>				glide_ratio					{ this, "/glide-ratio" };
	v2::PropertyOut<std::string>		glide_ratio_string			{ this, "/glide-ratio-string" };
	v2::PropertyOut<si::Power>			total_energy_variometer		{ this, "/total-energy-variometer" };
	// Current stall IAS (depends on current bank angle):
	v2::PropertyOut<si::Velocity>		v_s							{ this, "/v.s" };
	// Stall IAS with wings level:
	v2::PropertyOut<si::Velocity>		v_s_0_deg					{ this, "/v.s-0-deg" };
	// Stall IAS at 5° bank angle:
	v2::PropertyOut<si::Velocity>		v_s_5_deg					{ this, "/v.s-5-deg" };
	// Stall IAS at 30° bank angle:
	v2::PropertyOut<si::Velocity>		v_s_30_deg					{ this, "/v.s-30-deg" };
	// Rotation IAS on take-off:
	v2::PropertyOut<si::Velocity>		v_r							{ this, "/v.r" };
	// Max maneuvering IAS:
	v2::PropertyOut<si::Velocity>		v_a							{ this, "/v.a" };
	// Approach IAS:
	v2::PropertyOut<si::Velocity>		v_approach					{ this, "/v.approach" };
	// Take-off decision speed: TODO compute:
	v2::PropertyOut<si::Velocity>		v_1							{ this, "/v.1" };
	// One engine inoperative decision IAS: TODO compute:
	v2::PropertyOut<si::Velocity>		v_2							{ this, "/v.2" };
	// Best unpowered range IAS (best glide IAS): TODO compute:
	v2::PropertyOut<si::Velocity>		v_bg						{ this, "/v.bg" };
	// Best powered range IAS: TODO compute
	v2::PropertyOut<si::Velocity>		v_br						{ this, "/v.br" };
	// Maximum unpowered airborne time IAS (minimum descent IAS): TODO compute
	v2::PropertyOut<si::Velocity>		v_md						{ this, "/v.md" };
	// Maximum powered airborne time IAS (best endurance IAS): TODO compute
	v2::PropertyOut<si::Velocity>		v_be						{ this, "/v.be" };
	// Best angle of climb (shortest ground distance climb): TODO compute
	v2::PropertyOut<si::Velocity>		v_x							{ this, "/v.x" };
	// Best rate of climb (shortest time climb): TODO compute
	v2::PropertyOut<si::Velocity>		v_y							{ this, "/v.y" };
	v2::PropertyOut<si::Angle>			critical_aoa				{ this, "/aoa.critical" };
	v2::PropertyOut<bool>				stall						{ this, "/stall" };
	v2::PropertyOut<double>				lift_coefficient			{ this, "/lift-coefficient" };
	v2::PropertyOut<si::Velocity>		estimated_ias				{ this, "/estimated.ias" };
	v2::PropertyOut<si::Velocity>		estimated_ias_error			{ this, "/estimated.ias-error" };
	v2::PropertyOut<si::Angle>			estimated_aoa				{ this, "/estimated.aoa" };
	v2::PropertyOut<si::Angle>			estimated_aoa_error			{ this, "/estimated.aoa-error" };
	v2::PropertyOut<si::Angle>			slip_skid					{ this, "/slip-skid" };
};


class PerformanceComputer: public v2::Module<PerformanceComputerIO>
{
  public:
	// Ctor
	explicit
	PerformanceComputer (std::unique_ptr<PerformanceComputerIO>, xf::Airframe*, std::string const& instance = {});

  protected:
	void
	process (v2::Cycle const&) override;

	void
	compute_wind();

	void
	compute_glide_ratio();

	void
	compute_total_energy_variometer();

	void
	compute_speeds();

	void
	compute_speeds_vbg();

	std::optional<si::Velocity>
	get_stall_ias (Angle const& max_bank_angle) const;

	std::optional<si::Velocity>
	tas_to_ias (si::Velocity const& tas) const;

	void
	compute_critical_aoa();

	void
	compute_C_L();

	void
	compute_C_D();

	void
	compute_estimations();

	void
	compute_slip_skid();

	/**
	 * Convert AOA to IAS for current environment and configuration.
	 * Automatically includes flaps/spoilers angle, so parameter @aoa
	 * should only be wings AOA.
	 *
	 * May return empty result if it's not possible to compute TAS.
	 */
	std::optional<si::Velocity>
	aoa_to_tas_now (si::Angle const& aoa, std::optional<si::Acceleration> const& load = {}) const;

  private:
	xf::Airframe*					_airframe;
	si::Energy						_prev_total_energy					= 0_J;
	// Note: PropertyObservers depend on Smoothers, so first Smoothers must be defined,
	// then PropertyObservers, to ensure correct order of destruction.
	xf::RangeSmoother<si::Angle>	_wind_direction_smoother			{ { 0.0_deg, 360.0_deg }, 5_s };
	xf::Smoother<si::Velocity>		_wind_speed_smoother				{ 5_s };
	xf::Smoother<si::Power>			_total_energy_variometer_smoother	{ 1_s };
	xf::Smoother<double>			_cl_smoother						{ 1_s };
	v2::PropertyObserver			_wind_computer;
	v2::PropertyObserver			_glide_ratio_computer;
	v2::PropertyObserver			_total_energy_variometer_computer;
	v2::PropertyObserver			_speeds_computer;
	v2::PropertyObserver			_aoa_computer;
	v2::PropertyObserver			_cl_computer;
	v2::PropertyObserver			_estimations_computer;
	v2::PropertyObserver			_slip_skid_computer;
};

#endif

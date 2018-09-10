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
#include <xefis/core/module.h>
#include <xefis/core/property.h>
#include <xefis/core/property_observer.h>
#include <xefis/core/setting.h>
#include <xefis/utility/smoother.h>
#include <xefis/utility/range_smoother.h>


class PerformanceComputerIO: public xf::ModuleIO
{
  public:
	/*
	 * Settings
	 */

	xf::Setting<si::Velocity>			tev_min_ias					{ this, "tev_min_ias", 0_kt };

	/*
	 * Input
	 */

	xf::PropertyIn<si::Velocity>		speed_ias					{ this, "speed.ias" };
	xf::PropertyIn<si::Velocity>		speed_tas					{ this, "speed.tas" };
	xf::PropertyIn<si::Velocity>		speed_gs					{ this, "speed.gs" };
	xf::PropertyIn<si::Velocity>		vertical_speed				{ this, "vertical-speed" };
	xf::PropertyIn<si::Length>			altitude_amsl_std			{ this, "altitude.amsl" };
	xf::PropertyIn<si::Angle>			track_lateral_true			{ this, "track.lateral.true" };
	xf::PropertyIn<si::Angle>			orientation_heading_true	{ this, "orientation.heading.true" };
	xf::PropertyIn<si::Angle>			magnetic_declination		{ this, "magnetic-declination" };
	xf::PropertyIn<si::Length>			density_altitude			{ this, "density-altitude" };
	xf::PropertyIn<si::Density>			air_density_static			{ this, "air-density.static" };
	xf::PropertyIn<si::Mass>			aircraft_mass				{ this, "aircraft-mass" };
	xf::PropertyIn<si::Angle>			flaps_angle					{ this, "flaps-angle" };
	xf::PropertyIn<si::Angle>			spoilers_angle				{ this, "spoilers-angle" };
	xf::PropertyIn<si::Angle>			aoa_alpha					{ this, "aoa.alpha" };
	xf::PropertyIn<si::Acceleration>	load						{ this, "load" };
	xf::PropertyIn<si::Angle>			bank_angle					{ this, "bank-angle" };
	xf::PropertyIn<si::Acceleration>	y_acceleration				{ this, "acceleration.y" };
	xf::PropertyIn<si::Acceleration>	z_acceleration				{ this, "acceleration.x" };

	/*
	 * Output
	 */

	xf::PropertyOut<si::Angle>			wind_from_true				{ this, "wind.from.true" };
	xf::PropertyOut<si::Angle>			wind_from_magnetic			{ this, "wind.from.magnetic" };
	xf::PropertyOut<si::Velocity>		wind_tas					{ this, "wind.tas" };
	xf::PropertyOut<double>				glide_ratio					{ this, "glide-ratio" };
	xf::PropertyOut<std::string>		glide_ratio_string			{ this, "glide-ratio-string" };
	xf::PropertyOut<si::Power>			total_energy_variometer		{ this, "total-energy-variometer" };
	// Current stall IAS (depends on current bank angle):
	xf::PropertyOut<si::Velocity>		v_s							{ this, "v.s" };
	// Stall IAS with wings level:
	xf::PropertyOut<si::Velocity>		v_s_0_deg					{ this, "v.s-0-deg" };
	// Stall IAS at 5° bank angle:
	xf::PropertyOut<si::Velocity>		v_s_5_deg					{ this, "v.s-5-deg" };
	// Stall IAS at 30° bank angle:
	xf::PropertyOut<si::Velocity>		v_s_30_deg					{ this, "v.s-30-deg" };
	// Rotation IAS on take-off:
	xf::PropertyOut<si::Velocity>		v_r							{ this, "v.r" };
	// Max maneuvering IAS:
	xf::PropertyOut<si::Velocity>		v_a							{ this, "v.a" };
	// Approach IAS:
	xf::PropertyOut<si::Velocity>		v_approach					{ this, "v.approach" };
	// Take-off decision speed: TODO compute:
	xf::PropertyOut<si::Velocity>		v_1							{ this, "v.1" };
	// One engine inoperative decision IAS: TODO compute:
	xf::PropertyOut<si::Velocity>		v_2							{ this, "v.2" };
	// Best unpowered range IAS (best glide IAS): TODO compute:
	xf::PropertyOut<si::Velocity>		v_bg						{ this, "v.bg" };
	// Best powered range IAS: TODO compute
	xf::PropertyOut<si::Velocity>		v_br						{ this, "v.br" };
	// Maximum unpowered airborne time IAS (minimum descent IAS): TODO compute
	xf::PropertyOut<si::Velocity>		v_md						{ this, "v.md" };
	// Maximum powered airborne time IAS (best endurance IAS): TODO compute
	xf::PropertyOut<si::Velocity>		v_be						{ this, "v.be" };
	// Best angle of climb (shortest ground distance climb): TODO compute
	xf::PropertyOut<si::Velocity>		v_x							{ this, "v.x" };
	// Best rate of climb (shortest time climb): TODO compute
	xf::PropertyOut<si::Velocity>		v_y							{ this, "v.y" };
	xf::PropertyOut<si::Angle>			critical_aoa				{ this, "aoa.critical" };
	xf::PropertyOut<bool>				stall						{ this, "stall" };
	xf::PropertyOut<double>				lift_coefficient			{ this, "lift-coefficient" };
	xf::PropertyOut<si::Velocity>		estimated_ias				{ this, "estimated.ias" };
	xf::PropertyOut<si::Velocity>		estimated_ias_error			{ this, "estimated.ias-error" };
	xf::PropertyOut<si::Angle>			estimated_aoa				{ this, "estimated.aoa" };
	xf::PropertyOut<si::Angle>			estimated_aoa_error			{ this, "estimated.aoa-error" };
	xf::PropertyOut<si::Angle>			slip_skid					{ this, "slip-skid" };
};


class PerformanceComputer: public xf::Module<PerformanceComputerIO>
{
  public:
	// Ctor
	explicit
	PerformanceComputer (std::unique_ptr<PerformanceComputerIO>, xf::Airframe*, std::string_view const& instance = {});

  protected:
	void
	process (xf::Cycle const&) override;

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
	xf::PropertyObserver			_wind_computer;
	xf::PropertyObserver			_glide_ratio_computer;
	xf::PropertyObserver			_total_energy_variometer_computer;
	xf::PropertyObserver			_speeds_computer;
	xf::PropertyObserver			_aoa_computer;
	xf::PropertyObserver			_cl_computer;
	xf::PropertyObserver			_estimations_computer;
	xf::PropertyObserver			_slip_skid_computer;
};

#endif

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

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/setting.h>
#include <xefis/core/sockets/module_socket.h>
#include <xefis/support/airframe/airframe.h>
#include <xefis/support/sockets/socket_observer.h>
#include <xefis/utility/smoother.h>
#include <xefis/utility/range_smoother.h>

// Standard:
#include <cstddef>
#include <array>


namespace si = neutrino::si;
using namespace neutrino::si::literals;


class PerformanceComputerIO: public xf::Module
{
  public:
	/*
	 * Settings
	 */

	xf::Setting<si::Velocity>		tev_min_ias					{ this, "tev_min_ias", 0_kt };

	/*
	 * Input
	 */

	xf::ModuleIn<si::Velocity>		speed_ias					{ this, "speed.ias" };
	xf::ModuleIn<si::Velocity>		speed_tas					{ this, "speed.tas" };
	xf::ModuleIn<si::Velocity>		speed_gs					{ this, "speed.gs" };
	xf::ModuleIn<si::Velocity>		vertical_speed				{ this, "vertical-speed" };
	xf::ModuleIn<si::Length>		altitude_amsl_std			{ this, "altitude.amsl" };
	xf::ModuleIn<si::Angle>			track_lateral_true			{ this, "track.lateral.true" };
	xf::ModuleIn<si::Angle>			orientation_heading_true	{ this, "orientation.heading.true" };
	xf::ModuleIn<si::Angle>			magnetic_declination		{ this, "magnetic-declination" };
	xf::ModuleIn<si::Length>		density_altitude			{ this, "density-altitude" };
	xf::ModuleIn<si::Density>		air_density_static			{ this, "air-density.static" };
	xf::ModuleIn<si::Mass>			aircraft_mass				{ this, "aircraft-mass" };
	xf::ModuleIn<si::Angle>			flaps_angle					{ this, "flaps-angle" };
	xf::ModuleIn<si::Angle>			spoilers_angle				{ this, "spoilers-angle" };
	xf::ModuleIn<si::Angle>			aoa_alpha					{ this, "aoa.alpha" };
	xf::ModuleIn<si::Acceleration>	load						{ this, "load" };
	xf::ModuleIn<si::Angle>			bank_angle					{ this, "bank-angle" };
	xf::ModuleIn<si::Acceleration>	y_acceleration				{ this, "acceleration.y" };
	xf::ModuleIn<si::Acceleration>	z_acceleration				{ this, "acceleration.x" };

	/*
	 * Output
	 */

	xf::ModuleOut<si::Angle>		wind_from_true				{ this, "wind.from.true" };
	xf::ModuleOut<si::Angle>		wind_from_magnetic			{ this, "wind.from.magnetic" };
	xf::ModuleOut<si::Velocity>		wind_tas					{ this, "wind.tas" };
	xf::ModuleOut<double>			glide_ratio					{ this, "glide-ratio" };
	xf::ModuleOut<std::string>		glide_ratio_string			{ this, "glide-ratio-string" };
	xf::ModuleOut<si::Power>		total_energy_variometer		{ this, "total-energy-variometer" };
	// Current stall IAS (depends on current bank angle):
	xf::ModuleOut<si::Velocity>		v_s							{ this, "v.s" };
	// Stall IAS with wings level:
	xf::ModuleOut<si::Velocity>		v_s_0_deg					{ this, "v.s-0-deg" };
	// Stall IAS at 5° bank angle:
	xf::ModuleOut<si::Velocity>		v_s_5_deg					{ this, "v.s-5-deg" };
	// Stall IAS at 30° bank angle:
	xf::ModuleOut<si::Velocity>		v_s_30_deg					{ this, "v.s-30-deg" };
	// Rotation IAS on take-off:
	xf::ModuleOut<si::Velocity>		v_r							{ this, "v.r" };
	// Max maneuvering IAS:
	xf::ModuleOut<si::Velocity>		v_a							{ this, "v.a" };
	// Approach IAS:
	xf::ModuleOut<si::Velocity>		v_approach					{ this, "v.approach" };
	// Take-off decision speed: TODO compute:
	xf::ModuleOut<si::Velocity>		v_1							{ this, "v.1" };
	// One engine inoperative decision IAS: TODO compute:
	xf::ModuleOut<si::Velocity>		v_2							{ this, "v.2" };
	// Best unpowered range IAS (best glide IAS): TODO compute:
	xf::ModuleOut<si::Velocity>		v_bg						{ this, "v.bg" };
	// Best powered range IAS: TODO compute
	xf::ModuleOut<si::Velocity>		v_br						{ this, "v.br" };
	// Maximum unpowered airborne time IAS (minimum descent IAS): TODO compute
	xf::ModuleOut<si::Velocity>		v_md						{ this, "v.md" };
	// Maximum powered airborne time IAS (best endurance IAS): TODO compute
	xf::ModuleOut<si::Velocity>		v_be						{ this, "v.be" };
	// Best angle of climb (shortest ground distance climb): TODO compute
	xf::ModuleOut<si::Velocity>		v_x							{ this, "v.x" };
	// Best rate of climb (shortest time climb): TODO compute
	xf::ModuleOut<si::Velocity>		v_y							{ this, "v.y" };
	xf::ModuleOut<si::Angle>		critical_aoa				{ this, "aoa.critical" };
	xf::ModuleOut<bool>				stall						{ this, "stall" };
	xf::ModuleOut<double>			lift_coefficient			{ this, "lift-coefficient" };
	xf::ModuleOut<si::Velocity>		estimated_ias				{ this, "estimated.ias" };
	xf::ModuleOut<si::Velocity>		estimated_ias_error			{ this, "estimated.ias-error" };
	xf::ModuleOut<si::Angle>		estimated_aoa				{ this, "estimated.aoa" };
	xf::ModuleOut<si::Angle>		estimated_aoa_error			{ this, "estimated.aoa-error" };
	xf::ModuleOut<si::Angle>		slip_skid					{ this, "slip-skid" };

  public:
	using xf::Module::Module;
};


class PerformanceComputer: public PerformanceComputerIO
{
  public:
	// Ctor
	explicit
	PerformanceComputer (xf::Airframe*, std::string_view const& instance = {});

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
	get_stall_ias (si::Angle max_bank_angle) const;

	std::optional<si::Velocity>
	tas_to_ias (si::Velocity tas) const;

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
	aoa_to_tas_now (si::Angle aoa, std::optional<si::Acceleration> load = {}) const;

  private:
	PerformanceComputerIO&			_io									{ *this };
	xf::Airframe*					_airframe;
	si::Energy						_prev_total_energy					{ 0_J };
	// Note: SocketObservers depend on Smoothers, so first Smoothers must be defined,
	// then SocketObservers, to ensure correct order of destruction.
	xf::RangeSmoother<si::Angle>	_wind_direction_smoother			{ { 0.0_deg, 360.0_deg }, 5_s };
	xf::Smoother<si::Velocity>		_wind_speed_smoother				{ 5_s };
	xf::Smoother<si::Power>			_total_energy_variometer_smoother	{ 1_s };
	xf::Smoother<double>			_cl_smoother						{ 1_s };
	xf::SocketObserver				_wind_computer;
	xf::SocketObserver				_glide_ratio_computer;
	xf::SocketObserver				_total_energy_variometer_computer;
	xf::SocketObserver				_speeds_computer;
	xf::SocketObserver				_aoa_computer;
	xf::SocketObserver				_cl_computer;
	xf::SocketObserver				_estimations_computer;
	xf::SocketObserver				_slip_skid_computer;
};

#endif

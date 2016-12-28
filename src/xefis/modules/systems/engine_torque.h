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

#ifndef XEFIS__MODULES__SYSTEMS__ENGINE_TORQUE_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__ENGINE_TORQUE_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/module.h>
#include <xefis/core/v2/property.h>
#include <xefis/core/v2/setting.h>
#include <xefis/utility/temporal.h>
#include <xefis/utility/datatable2d.h>


/**
 * This module computes motor torque from the measured current drawn by motor.
 * These are the formulas:
 *   Kv = x [RPM / V]
 *   Kt = 1 / Kv = 1/x [Nm / A]
 * So Kv of the motor is needed as a setting. Also engine efficiency is needed either as a constant or a Datatable2D,
 * as a function of motor rotational speed.
 */
class EngineTorque: public x2::Module
{
  private:
	using EfficiencyDatatable	= xf::Datatable2D<si::AngularVelocity, double>;
	using EngineEfficiency		= boost::variant<double, EfficiencyDatatable>;
	using MotorKv				= decltype (1.0_rpm / 1.0_V);

  public:
	/*
	 * Settings
	 */

	x2::Setting<EngineEfficiency>		setting_motor_efficiency	{ this };
	x2::Setting<MotorKv>				setting_motor_kv			{ this };

	/*
	 * Input
	 */

	x2::PropertyIn<si::AngularVelocity>	input_engine_speed			{ this, "/engine-speed" };
	x2::PropertyIn<si::Current>			input_engine_current		{ this, "/engine-current" };

	/*
	 * Output
	 */

	x2::PropertyOut<si::Torque>			output_engine_torque		{ this, "/engine-torque" };

  public:
	// Ctor
	explicit EngineTorque (std::string const& instance = {});

  protected:
	// Module API
	void
	process (x2::Cycle const&) override;

  private:
	void
	compute_torque (double motor_efficiency);

	void
	compute_torque (EfficiencyDatatable const& motor_efficiency);
};

#endif


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
#include <variant>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/module.h>
#include <xefis/core/v2/property.h>
#include <xefis/core/v2/setting.h>
#include <xefis/utility/temporal.h>
#include <xefis/utility/datatable2d.h>


class EngineTorqueIO: public v2::ModuleIO
{
  public:
	using EfficiencyDatatable	= xf::Datatable2D<si::AngularVelocity, double>;
	using EngineEfficiency		= std::variant<double, EfficiencyDatatable>;
	using MotorKv				= decltype (1.0_rpm / 1.0_V);

  public:
	/*
	 * Settings
	 */

	v2::Setting<EngineEfficiency>		motor_efficiency	{ this, "motor_efficiency" };
	v2::Setting<MotorKv>				motor_kv			{ this, "motor_kv" };

	/*
	 * Input
	 */

	v2::PropertyIn<si::AngularVelocity>	engine_speed		{ this, "/engine-speed" };
	v2::PropertyIn<si::Current>			engine_current		{ this, "/engine-current" };

	/*
	 * Output
	 */

	v2::PropertyOut<si::Torque>			engine_torque		{ this, "/engine-torque" };
};


/**
 * This module computes motor torque from the measured current drawn by motor.
 * These are the formulas:
 *   Kv = x [RPM / V]
 *   Kt = 1 / Kv = 1/x [Nm / A]
 * So Kv of the motor is needed as a setting. Also engine efficiency is needed either as a constant or a Datatable2D,
 * as a function of motor rotational speed.
 */
class EngineTorque: public v2::Module<EngineTorqueIO>
{
  private:
	using EfficiencyDatatable = EngineTorqueIO::EfficiencyDatatable;

  public:
	// Ctor
	explicit
	EngineTorque (std::unique_ptr<EngineTorqueIO>, std::string const& instance = {});

  protected:
	// Module API
	void
	process (v2::Cycle const&) override;

  private:
	void
	compute_torque (double motor_efficiency);

	void
	compute_torque (EfficiencyDatatable const& motor_efficiency);
};

#endif


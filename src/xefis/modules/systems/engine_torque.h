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

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/setting.h>
#include <xefis/core/sockets/module_socket.h>
#include <xefis/utility/temporal.h>

// Neutrino:
#include <neutrino/math/field.h>

// Standard:
#include <cstddef>
#include <variant>


namespace si = neutrino::si;
using namespace neutrino::si::literals;


class EngineTorqueIO: public xf::Module
{
  public:
	using EfficiencyField	= xf::Field<si::AngularVelocity, double>;
	using EngineEfficiency	= std::variant<double, EfficiencyField>;
	using MotorKv			= decltype (1_rpm / 1_V);

  public:
	/*
	 * Settings
	 */

	xf::Setting<EngineEfficiency>		motor_efficiency	{ this, "motor_efficiency" };
	xf::Setting<MotorKv>				motor_kv			{ this, "motor_kv" };

	/*
	 * Input
	 */

	xf::ModuleIn<si::AngularVelocity>	engine_speed		{ this, "engine-speed" };
	xf::ModuleIn<si::Current>			engine_current		{ this, "engine-current" };

	/*
	 * Output
	 */

	xf::ModuleOut<si::Torque>			engine_torque		{ this, "engine-torque" };

  public:
	using xf::Module::Module;
};


/**
 * This module computes motor torque from the measured current drawn by motor.
 * These are the formulas:
 *   Kv = x [RPM / V]
 *   Kt = 1 / Kv = 1/x [Nm / A]
 * So Kv of the motor is needed as a setting. Also engine efficiency is needed either as a constant or a Field,
 * as a function of motor rotational speed.
 */
class EngineTorque: public EngineTorqueIO
{
  private:
	using EfficiencyField = EngineTorqueIO::EfficiencyField;

  public:
	// Ctor
	explicit
	EngineTorque (std::string_view const& instance = {});

  protected:
	// Module API
	void
	process (xf::Cycle const&) override;

  private:
	void
	compute_torque (double motor_efficiency);

	void
	compute_torque (EfficiencyField const& motor_efficiency);

  private:
	EngineTorqueIO& _io { *this };
};

#endif


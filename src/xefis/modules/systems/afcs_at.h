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

#ifndef XEFIS__MODULES__SYSTEMS__AFCS_AT_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__AFCS_AT_H__INCLUDED

// Standard:
#include <cstddef>
#include <utility>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/module.h>
#include <xefis/core/v2/property.h>
#include <xefis/core/v2/property_observer.h>
#include <xefis/core/v2/setting.h>
#include <xefis/utility/pid_control.h>
#include <xefis/utility/smoother.h>
#include <xefis/utility/v2/actions.h>


class AFCS_AT_IO: public v2::ModuleIO
{
  public:
	enum class SpeedMode
	{
		None		= 0,
		Thrust		= 1,
		Airspeed	= 2,
	};

  public:
	/*
	 * Settings
	 */

	v2::Setting<xf::PIDControl<si::Velocity, si::Force>::Settings>
									ias_pid_settings		{ this, "ias_pid_settings" };
	v2::Setting<double>				ias_pid_gain			{ this, "ias_pid_gain", 1.0 };
	v2::Setting<si::Force>			output_thrust_minimum	{ this, "output_thrust_minimum", 0.0_N };
	v2::Setting<si::Force>			output_thrust_maximum	{ this, "output_thrust_maximum", 1.0_N };

	/*
	 * Input
	 */

	v2::PropertyIn<SpeedMode>		cmd_speed_mode			{ this, "/cmd/speed-mode" };
	v2::PropertyIn<si::Force>		cmd_thrust				{ this, "/cmd/thrust" };
	v2::PropertyIn<si::Velocity>	cmd_ias					{ this, "/cmd/ias" };
	v2::PropertyIn<si::Velocity>	measured_ias			{ this, "/measurements/ias" };

	/*
	 * Output
	 */

	v2::PropertyOut<si::Force>		thrust					{ this, "/thrust" };

	/*
	 * Input/Output
	 */

	v2::PropertyOut<bool>			disengage_at			{ this, "/disengage-at" };
};


class AFCS_AT: public v2::Module<AFCS_AT_IO>
{
  public:
	// Ctor
	explicit
	AFCS_AT (std::unique_ptr<AFCS_AT_IO>, std::string const& instance = {});

  protected:
	// Module API
	void
	initialize() override;

	// Module API
	void
	process (v2::Cycle const&) override;

  private:
	void
	compute_thrust();

	/**
	 * Ensure _speed_mode has a valid allowed value.
	 */
	void
	clamp_speed_mode();

  private:
	xf::PIDControl<si::Velocity, si::Force>	_ias_pid;
	xf::Smoother<si::Force>					_ias_pid_smoother		{ 250_ms };
	v2::PropertyObserver					_thrust_computer;
};

#endif

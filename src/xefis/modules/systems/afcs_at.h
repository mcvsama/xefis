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


class AFCS_AT: public x2::Module
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

	x2::Setting<xf::PIDControl<si::Velocity, si::Force>::Settings>
											setting_ias_pid_settings		{ this };
	x2::Setting<double>						setting_ias_pid_gain			{ this, 1.0 };
	x2::Setting<si::Force>					setting_output_thrust_minimum	{ this, 0.0_N };
	x2::Setting<si::Force>					setting_output_thrust_maximum	{ this, 1.0_N };

	/*
	 * Input
	 */

	x2::PropertyIn<SpeedMode>				input_cmd_speed_mode			{ this, "/cmd/speed-mode" };
	x2::PropertyIn<si::Force>				input_cmd_thrust				{ this, "/cmd/thrust" };
	x2::PropertyIn<si::Velocity>			input_cmd_ias					{ this, "/cmd/ias" };
	x2::PropertyIn<si::Velocity>			input_measured_ias				{ this, "/measurements/ias" };

	/*
	 * Output
	 */

	x2::PropertyOut<si::Force>				output_thrust					{ this, "/thrust" };

	/*
	 * Input/Output
	 */
	x2::PropertyOut<bool>					io_disengage_at					{ this, "/disengage-at" };

  public:
	// Ctor
	explicit
	AFCS_AT (std::string const& instance = {});

  protected:
	// Module API
	void
	initialize() override;

	// Module API
	void
	process (x2::Cycle const&) override;

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
	x2::PropertyObserver					_thrust_computer;
};

#endif

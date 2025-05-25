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

#ifndef XEFIS__MODULES__SYSTEMS__AFCS_AUTOTHROTTLE_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__AFCS_AUTOTHROTTLE_H__INCLUDED

// Local:
#include "afcs_api.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/setting.h>
#include <xefis/core/sockets/module_socket.h>
#include <xefis/support/control/pid_controller.h>
#include <xefis/support/sockets/socket_observer.h>
#include <xefis/utility/smoother.h>

// Standard:
#include <cstddef>
#include <utility>


namespace si = neutrino::si;
using namespace neutrino::si::literals;


class AFCS_Autothrottle_IO: public xf::Module
{
  public:
	/*
	 * Settings
	 */

	xf::Setting<xf::PIDSettings<>>	ias_pid_settings		{ this, "ias_pid_settings" };
	xf::Setting<double>				ias_pid_gain			{ this, "ias_pid_gain", 1.0 };
	xf::Setting<si::Force>			output_thrust_minimum	{ this, "output_thrust_minimum", 0.0_N };
	xf::Setting<si::Force>			output_thrust_maximum	{ this, "output_thrust_maximum", 1.0_N };

	/*
	 * Input
	 */

	xf::ModuleIn<afcs::SpeedMode>	cmd_speed_mode			{ this, "cmd/speed-mode" };
	xf::ModuleIn<si::Force>			cmd_thrust				{ this, "cmd/thrust" };
	xf::ModuleIn<si::Velocity>		cmd_ias					{ this, "cmd/ias" };
	xf::ModuleIn<si::Velocity>		measured_ias			{ this, "measurements/ias" };

	/*
	 * Output
	 */

	xf::ModuleOut<si::Force>		thrust					{ this, "thrust" };

	/*
	 * Input/Output
	 */

	// Set to true when AT can't control the throttle for any reason:
	xf::ModuleOut<bool>				disengage_at			{ this, "disengage-at" };

  public:
	using xf::Module::Module;
};


class AFCS_Autothrottle: public AFCS_Autothrottle_IO
{
  public:
	// Ctor
	explicit
	AFCS_Autothrottle (xf::ProcessingLoop&, std::string_view const instance = {});

  protected:
	// Module API
	void
	initialize() override;

	// Module API
	void
	process (xf::Cycle const&) override;

  private:
	void
	compute_thrust();

	/**
	 * Ensure _speed_mode has a valid allowed value.
	 */
	void
	clamp_speed_mode();

  private:
	AFCS_Autothrottle_IO&						_io					{ *this };
	xf::PIDController<si::Velocity, si::Force>	_ias_pid;
	xf::Smoother<si::Force>						_ias_pid_smoother	{ 250_ms };
	xf::SocketObserver							_thrust_computer;
};

#endif

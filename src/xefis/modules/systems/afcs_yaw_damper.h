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

#ifndef XEFIS__MODULES__SYSTEMS__AFCS_YAW_DAMPER_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__AFCS_YAW_DAMPER_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/setting.h>
#include <xefis/core/sockets/module_socket.h>
#include <xefis/support/control/pid_controller.h>
#include <xefis/support/sockets/socket_observer.h>

// Neutrino:
#include <neutrino/si/si.h>

// Standard:
#include <cstddef>
#include <string>


namespace nu = neutrino;
namespace si = nu::si;
using namespace nu::si::literals;


class AFCS_YawDamper_IO: public xf::Module
{
  public:
	/*
	 * Settings
	 */

	xf::Setting<xf::PIDSettings<>>	rudder_pid_settings	{ this, "rudder_pid_settings" };
	xf::Setting<double>				rudder_pid_gain		{ this, "rudder_pid_gain", 1.0 };
	xf::Setting<si::Angle>			deflection_limit	{ this, "deflection_limit" };

	/*
	 * Input
	 */

	xf::ModuleIn<bool>				enabled				{ this, "enabled" };
	xf::ModuleIn<si::Force>			slip_skid			{ this, "slip-skid" };

	/*
	 * Output
	 */

	xf::ModuleOut<si::Angle>		rudder_deflection	{ this, "rudder-deflection" };

  public:
	using xf::Module::Module;
};


/**
 * Controls rudder to obtain zero slip-skid value.
 */
class AFCS_YawDamper: public AFCS_YawDamper_IO
{
  public:
	// Ctor
	explicit
	AFCS_YawDamper (xf::ProcessingLoop& loop, std::string_view const instance = {});

  private:
	// Module API
	void
	initialize() override;

	// Module API
	void
	process (xf::Cycle const&) override;

	/**
	 * Compute rudder.
	 */
	void
	compute();

  private:
	AFCS_YawDamper_IO&						_io { *this };
	xf::PIDController<si::Force, si::Angle>	_rudder_pid;
	xf::SocketObserver						_rudder_computer;
};

#endif

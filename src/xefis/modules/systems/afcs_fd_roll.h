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

#ifndef XEFIS__MODULES__SYSTEMS__AFCS_FD_ROLL_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__AFCS_FD_ROLL_H__INCLUDED

// Local:
#include "afcs_api.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/setting.h>
#include <xefis/core/sockets/module_socket.h>
#include <xefis/support/control/pid_controller.h>
#include <xefis/support/sockets/socket_observer.h>
#include <xefis/utility/range_smoother.h>

// Standard:
#include <cstddef>


namespace si = neutrino::si;
using namespace neutrino::si::literals;


class AFCS_FD_Roll_IO: public xf::Module
{
  public:
	using DirectionPID = xf::PIDController<si::Angle, si::Angle>;

  public:
	/*
	 * Settings
	 */

	xf::Setting<DirectionPID::Settings>	hdg_pid_settings		{ this, "hdg_pid_settings" };
	xf::Setting<DirectionPID::Settings>	trk_pid_settings		{ this, "trk_pid_settings" };

	/*
	 * Input
	 */

	xf::ModuleIn<bool>					autonomous				{ this, "autonomous" };
	xf::ModuleIn<si::Angle>				roll_limits				{ this, "roll-limits" };
	xf::ModuleIn<afcs::RollMode>		cmd_roll_mode			{ this, "cmd-roll-mode" };
	xf::ModuleIn<si::Angle>				cmd_magnetic_hdg		{ this, "cmd-magnetic-heading" };
	xf::ModuleIn<si::Angle>				cmd_magnetic_trk		{ this, "cmd-magnetic-track" };
	xf::ModuleIn<si::Angle>				measured_magnetic_hdg	{ this, "measured-magnetic-heading" };
	xf::ModuleIn<si::Angle>				measured_magnetic_trk	{ this, "measured-magnetic-track" };

	/*
	 * Output
	 */

	xf::ModuleOut<si::Angle>			roll					{ this, "output-roll" };
	xf::ModuleOut<bool>					operative				{ this, "operative" };

  public:
	using xf::Module::Module;
};


/**
 * Computes desired roll values to follow.
 * Output depends on roll-mode setting.
 */
// TODO disengage if outside safe limits, unless autonomous flag is set
// (autonomous flag tells whether user has still possibility to control the airplane,
// that is he is in the range of radio communication).
class AFCS_FD_Roll: public AFCS_FD_Roll_IO
{
  private:
	static constexpr char	kLoggerScope[]	= "mod::AFCS_FD_Roll";

	using DirectionPID = AFCS_FD_Roll_IO::DirectionPID;

  public:
	// Ctor
	explicit
	AFCS_FD_Roll (xf::Logger const&, std::string_view const& instance = {});

  protected:
	// Module API
	void
	initialize() override;

	// Module API
	void
	process (xf::Cycle const&) override;

	// Module API
	void
	rescue (xf::Cycle const&, std::exception_ptr) override;

  private:
	/**
	 * Compute all needed data and write to output sockets.
	 */
	void
	compute_roll();

	/**
	 * Compute roll angle for given PID and measured values and parameters.
	 */
	std::optional<si::Angle>
	compute_roll (xf::PIDController<si::Angle, si::Angle>& pid,
				  xf::ModuleIn<si::Angle> const& cmd_direction,
				  xf::ModuleIn<si::Angle> const& measured_direction,
				  si::Time update_dt) const;

	/**
	 * Override the "operative" output depending on "autonomous" flag.
	 */
	void
	check_autonomous();

  private:
	AFCS_FD_Roll_IO&				_io						{ *this };
	xf::Logger						_logger;
	DirectionPID					_magnetic_hdg_pid;
	DirectionPID					_magnetic_trk_pid;
	xf::RangeSmoother<si::Angle>	_output_roll_smoother	{ { -180.0_deg, +180.0_deg }, 2.5_s };
	xf::SocketObserver				_roll_computer;
};

#endif

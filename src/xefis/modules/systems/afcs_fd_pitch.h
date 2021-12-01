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

#ifndef XEFIS__MODULES__SYSTEMS__AFCS_FD_PITCH_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__AFCS_FD_PITCH_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/setting.h>
#include <xefis/core/sockets/module_socket.h>
#include <xefis/support/control/pid_controller.h>
#include <xefis/support/sockets/socket_observer.h>
#include <xefis/utility/range_smoother.h>

// Local:
#include "afcs_api.h"


namespace si = neutrino::si;
using namespace neutrino::si::literals;


class AFCS_FD_Pitch_IO: public xf::ModuleIO
{
  public:
	using IAS_PID		= xf::PIDController<si::Velocity, si::Angle>;
	using MachPID		= xf::PIDController<double, si::Angle>;
	using AltitudePID	= xf::PIDController<si::Length, si::Angle>;
	using VS_PID		= xf::PIDController<si::Velocity, si::Angle>;
	using FPA_PID		= xf::PIDController<si::Angle, si::Angle>;

  public:
	/*
	 * Settings
	 */

	xf::Setting<IAS_PID::Settings>		ias_pid_settings		{ this, "ias_pid_settings" };
	xf::Setting<MachPID::Settings>		mach_pid_settings		{ this, "mach_pid_settings" };
	xf::Setting<AltitudePID::Settings>	altitude_pid_settings	{ this, "altitude_pid_settings" };
	xf::Setting<VS_PID::Settings>		vs_pid_settings			{ this, "vs_pid_settings" };
	xf::Setting<FPA_PID::Settings>		fpa_pid_settings		{ this, "fpa_pid_settings" };

	/*
	 * Input
	 */

	xf::ModuleIn<bool>					autonomous				{ this, "autonomous" };
	xf::ModuleIn<si::Angle>				pitch_limits			{ this, "pitch-limits" };
	xf::ModuleIn<afcs::PitchMode>		cmd_pitch_mode			{ this, "cmd-pitch-mode" };
	xf::ModuleIn<si::Velocity>			cmd_ias					{ this, "cmd-ias" };
	xf::ModuleIn<double>				cmd_mach				{ this, "cmd-match" };
	xf::ModuleIn<si::Length>			cmd_alt					{ this, "cmd-altitude" };
	xf::ModuleIn<si::Velocity>			cmd_vs					{ this, "cmd-vs" };
	xf::ModuleIn<si::Angle>				cmd_fpa					{ this, "cmd-fpa" };
	xf::ModuleIn<si::Velocity>			measured_ias			{ this, "measured-ias" };
	xf::ModuleIn<double>				measured_mach			{ this, "measured-mach" };
	xf::ModuleIn<si::Length>			measured_alt			{ this, "measured-altitude" };
	xf::ModuleIn<si::Velocity>			measured_vs				{ this, "measured-vs" };
	xf::ModuleIn<si::Angle>				measured_fpa			{ this, "measured-fpa" };

	/*
	 * Output
	 */

	xf::ModuleOut<si::Angle>			pitch					{ this, "output-pitch" };
	xf::ModuleOut<bool>					operative				{ this, "operative" };
};


/**
 * Computes desired pitch values to follow.
 * Output depends on pitch-mode setting.
 */
// TODO disengage if outside safe limits, unless autonomous flag is set
// (autonomous flag tells whether user has still possibility to control the airplane,
// that is he is in the range of radio communication).
class AFCS_FD_Pitch: public xf::Module<AFCS_FD_Pitch_IO>
{
  private:
	static constexpr char	kLoggerScope[]	= "mod::AFCS_FD_Pitch";

  private:
	using IAS_PID		= AFCS_FD_Pitch_IO::IAS_PID;
	using MachPID		= AFCS_FD_Pitch_IO::MachPID;
	using AltitudePID	= AFCS_FD_Pitch_IO::AltitudePID;
	using VS_PID		= AFCS_FD_Pitch_IO::VS_PID;
	using FPA_PID		= AFCS_FD_Pitch_IO::FPA_PID;

  public:
	// Ctor
	explicit
	AFCS_FD_Pitch (std::unique_ptr<AFCS_FD_Pitch_IO>, xf::Logger const&, std::string_view const& instance = {});

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
	compute_pitch();

	/**
	 * Compute result angle from PID and parameters.
	 */
	template<class Input, class Control>
		std::optional<si::Angle>
		compute_pitch (xf::PIDController<Input, Control>& pid,
					   xf::ModuleIn<Input> const& cmd_param,
					   xf::ModuleIn<Input> const& measured_param,
					   si::Time update_dt) const;

	/**
	 * Override the "operative" output depending on "autonomous" flag.
	 */
	void
	check_autonomous();

  private:
	xf::Logger						_logger;
	IAS_PID							_ias_pid;
	MachPID							_mach_pid;
	AltitudePID						_altitude_pid;
	VS_PID							_vs_pid;
	FPA_PID							_fpa_pid;
	xf::RangeSmoother<si::Angle>	_output_pitch_smoother	{ { -180.0_deg, +180.0_deg }, 2.5_s };
	xf::SocketObserver				_pitch_computer;
};

#endif

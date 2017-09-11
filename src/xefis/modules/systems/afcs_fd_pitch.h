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
#include <xefis/core/v2/module.h>
#include <xefis/core/v2/property.h>
#include <xefis/core/v2/property_observer.h>
#include <xefis/core/v2/setting.h>
#include <xefis/utility/pid_control.h>
#include <xefis/utility/range_smoother.h>

// Local:
#include "afcs_api.h"


class AFCS_FD_Pitch_IO: public v2::ModuleIO
{
  public:
	using IAS_PID		= xf::PIDControl<si::Velocity, si::Angle>;
	using MachPID		= xf::PIDControl<double, si::Angle>;
	using AltitudePID	= xf::PIDControl<si::Length, si::Angle>;
	using VS_PID		= xf::PIDControl<si::Velocity, si::Angle>;
	using FPA_PID		= xf::PIDControl<si::Angle, si::Angle>;

  public:
	/*
	 * Settings
	 */

	v2::Setting<IAS_PID::Settings>			ias_pid_settings		{ this, "ias_pid_settings" };
	v2::Setting<MachPID::Settings>			mach_pid_settings		{ this, "mach_pid_settings" };
	v2::Setting<AltitudePID::Settings>		altitude_pid_settings	{ this, "altitude_pid_settings" };
	v2::Setting<VS_PID::Settings>			vs_pid_settings			{ this, "vs_pid_settings" };
	v2::Setting<FPA_PID::Settings>			fpa_pid_settings		{ this, "fpa_pid_settings" };

	/*
	 * Input
	 */

	v2::PropertyIn<bool>					autonomous				{ this, "/autonomous" };
	v2::PropertyIn<si::Angle>				pitch_limits			{ this, "/pitch-limits" };
	v2::PropertyIn<afcs_api::PitchMode>		cmd_pitch_mode			{ this, "/cmd-pitch-mode" };
	v2::PropertyIn<si::Velocity>			cmd_ias					{ this, "/cmd-ias" };
	v2::PropertyIn<double>					cmd_mach				{ this, "/cmd-match" };
	v2::PropertyIn<si::Length>				cmd_alt					{ this, "/cmd-altitude" };
	v2::PropertyIn<si::Velocity>			cmd_vs					{ this, "/cmd-vs" };
	v2::PropertyIn<si::Angle>				cmd_fpa					{ this, "/cmd-fpa" };
	v2::PropertyIn<si::Velocity>			measured_ias			{ this, "/measured-ias" };
	v2::PropertyIn<double>					measured_mach			{ this, "/measured-mach" };
	v2::PropertyIn<si::Length>				measured_alt			{ this, "/measured-altitude" };
	v2::PropertyIn<si::Velocity>			measured_vs				{ this, "/measured-vs" };
	v2::PropertyIn<si::Angle>				measured_fpa			{ this, "/measured-fpa" };

	/*
	 * Output
	 */

	v2::PropertyOut<si::Angle>				pitch					{ this, "/output-pitch" };
	v2::PropertyOut<bool>					operative				{ this, "/operative" };
};


/**
 * Computes desired pitch values to follow.
 * Output depends on pitch-mode setting.
 */
// TODO disengage if outside safe limits, unless autonomous flag is set
// (autonomous flag tells whether user has still possibility to control the airplane,
// that is he is in the range of radio communication).
class AFCS_FD_Pitch: public v2::Module<AFCS_FD_Pitch_IO>
{
  private:
	using IAS_PID		= AFCS_FD_Pitch_IO::IAS_PID;
	using MachPID		= AFCS_FD_Pitch_IO::MachPID;
	using AltitudePID	= AFCS_FD_Pitch_IO::AltitudePID;
	using VS_PID		= AFCS_FD_Pitch_IO::VS_PID;
	using FPA_PID		= AFCS_FD_Pitch_IO::FPA_PID;

  public:
	// Ctor
	explicit
	AFCS_FD_Pitch (std::unique_ptr<AFCS_FD_Pitch_IO>, std::string const& instance = {});

  protected:
	// Module API
	void
	initialize() override;

	// Module API
	void
	process (v2::Cycle const&) override;

	// Module API
	void
	rescue (std::exception_ptr) override;

  private:
	/**
	 * Compute all needed data and write to output properties.
	 */
	void
	compute_pitch();

	/**
	 * Compute result angle from PID and parameters.
	 */
	template<class Input, class Control>
		std::optional<si::Angle>
		compute_pitch (xf::PIDControl<Input, Control>& pid,
					   v2::PropertyIn<Input> const& cmd_param,
					   v2::PropertyIn<Input> const& measured_param,
					   si::Time update_dt) const;

	/**
	 * Override the "operative" output depending on "autonomous" flag.
	 */
	void
	check_autonomous();

  private:
	IAS_PID							_ias_pid;
	MachPID							_mach_pid;
	AltitudePID						_altitude_pid;
	VS_PID							_vs_pid;
	FPA_PID							_fpa_pid;
	xf::RangeSmoother<si::Angle>	_output_pitch_smoother	{ { -180.0_deg, +180.0_deg }, 2.5_s };
	v2::PropertyObserver			_pitch_computer;
};

#endif

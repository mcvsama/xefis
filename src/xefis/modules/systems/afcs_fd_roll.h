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


/**
 * Computes desired roll values to follow.
 * Output depends on roll-mode setting.
 */
// TODO disengage if outside safe limits, unless autonomous flag is set
// (autonomous flag tells whether user has still possibility to control the airplane,
// that is he is in the range of radio communication).
class AFCS_FD_Roll: public v2::Module
{
  private:
	using DirectionPID = xf::PIDControl<si::Angle, si::Angle>;

  public:
	/*
	 * Settings
	 */

	v2::Setting<DirectionPID::Settings>	setting_hdg_pid_settings	{ this };
	v2::Setting<DirectionPID::Settings>	setting_trk_pid_settings	{ this };

	/*
	 * Input
	 */

	v2::PropertyIn<bool>				input_autonomous			{ this, "/autonomous" };
	v2::PropertyIn<si::Angle>			input_roll_limits			{ this, "/roll-limits" };
	v2::PropertyIn<afcs_api::RollMode>	input_cmd_roll_mode			{ this, "/cmd-roll-mode" };
	v2::PropertyIn<si::Angle>			input_cmd_magnetic_hdg		{ this, "/cmd-magnetic-heading" };
	v2::PropertyIn<si::Angle>			input_cmd_magnetic_trk		{ this, "/cmd-magnetic-track" };
	v2::PropertyIn<si::Angle>			input_measured_magnetic_hdg	{ this, "/measured-magnetic-heading" };
	v2::PropertyIn<si::Angle>			input_measured_magnetic_trk	{ this, "/measured-magnetic-track" };

	/*
	 * Output
	 */

	v2::PropertyOut<si::Angle>			output_roll					{ this, "/output-roll" };
	v2::PropertyOut<bool>				output_operative			{ this, "/operative" };

  public:
	// Ctor
	explicit
	AFCS_FD_Roll (std::string const& instance = {});

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
	compute_roll();

	/**
	 * Compute roll angle for given PID and measured values and parameters.
	 */
	Optional<si::Angle>
	compute_roll (xf::PIDControl<si::Angle, si::Angle>& pid,
				  v2::PropertyIn<si::Angle> const& cmd_direction,
				  v2::PropertyIn<si::Angle> const& measured_direction,
				  si::Time update_dt) const;

	/**
	 * Override the "operative" output depending on "autonomous" flag.
	 */
	void
	check_autonomous();

  private:
	DirectionPID					_magnetic_hdg_pid;
	DirectionPID					_magnetic_trk_pid;
	xf::RangeSmoother<si::Angle>	_output_roll_smoother	{ { -180.0_deg, +180.0_deg }, 2.5_s };
	v2::PropertyObserver			_roll_computer;
};

#endif

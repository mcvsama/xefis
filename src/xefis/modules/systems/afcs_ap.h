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

#ifndef XEFIS__MODULES__SYSTEMS__AFCS_AP_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__AFCS_AP_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/module.h>
#include <xefis/core/v2/property.h>
#include <xefis/core/v2/property_observer.h>
#include <xefis/core/v2/setting.h>
#include <xefis/utility/pid_control.h>
#include <xefis/utility/smoother.h>


/**
 * Steers control surfaces (ailerons, elevator) to obtain desired orientation (pitch, roll).
 */
class AFCS_AP: public x2::Module
{
  public:
	/*
	 * Settings
	 */

	x2::Setting<double>			setting_overall_gain			{ this, 1.0 };
	x2::Setting<double>			setting_pitch_gain				{ this, 1.0 };
	x2::Setting<double>			setting_roll_gain				{ this, 1.0 };
	x2::Setting<xf::PIDControl<si::Angle, si::Angle>::Settings>
								setting_pitch_pid_settings		{ this };
	x2::Setting<xf::PIDControl<si::Angle, si::Angle>::Settings>
								setting_roll_pid_settings		{ this };

	/*
	 * Input
	 */

	x2::PropertyIn<si::Angle>	input_cmd_pitch					{ this, "/cmd-pitch" };
	x2::PropertyIn<si::Angle>	input_cmd_roll					{ this, "/cmd-roll" };
	x2::PropertyIn<si::Angle>	input_measured_pitch			{ this, "/measured-pitch" };
	x2::PropertyIn<si::Angle>	input_measured_roll				{ this, "/measured-roll" };
	x2::PropertyIn<si::Angle>	input_elevator_minimum			{ this, "/limits/elevator/minimum" };
	x2::PropertyIn<si::Angle>	input_elevator_maximum			{ this, "/limits/elevator/maximum" };
	x2::PropertyIn<si::Angle>	input_ailerons_minimum			{ this, "/limits/ailerons/minimum" };
	x2::PropertyIn<si::Angle>	input_ailerons_maximum			{ this, "/limits/ailerons/maximum" };

	/*
	 * Output
	 */

	x2::PropertyOut<bool>		output_serviceable				{ this, "/serviceable" };
	x2::PropertyOut<si::Angle>	output_elevator					{ this, "/elevator" };
	x2::PropertyOut<si::Angle>	output_ailerons					{ this, "/ailerons" };

  public:
	// Ctor
	explicit AFCS_AP (std::string const& instance = {});

  protected:
	// Module API
	void
	initialize() override;

	// Module API
	void
	process (x2::Cycle const&) override;

	// Module API
	void
	rescue (std::exception_ptr) override;

  private:
	/**
	 * Do all FBW computations and write to output properties.
	 */
	void
	compute_ap();

	/**
	 * Check properties and diagnose problem on the log.
	 */
	void
	diagnose();

  private:
	xf::PIDControl<si::Angle, si::Angle>	_elevator_pid;
	xf::PIDControl<si::Angle, si::Angle>	_ailerons_pid;
	xf::Smoother<si::Angle>					_elevator_smoother	{ 50_ms };
	xf::Smoother<si::Angle>					_ailerons_smoother	{ 50_ms };
	x2::PropertyObserver					_ap_computer;
};

#endif

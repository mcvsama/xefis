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


class AFCS_AP_IO: public v2::ModuleIO
{
  public:
	/*
	 * Settings
	 */

	v2::Setting<double>			overall_gain			{ this, "overall_gain", 1.0 };
	v2::Setting<xf::PIDControl<si::Angle, si::Angle>::Settings>
								pitch_pid_settings		{ this, "pitch_pid_settings" };
	v2::Setting<double>			pitch_gain				{ this, "pitch_gain", 1.0 };
	v2::Setting<xf::PIDControl<si::Angle, si::Angle>::Settings>
								roll_pid_settings		{ this, "roll_pid_settings" };
	v2::Setting<double>			roll_gain				{ this, "roll_gain", 1.0 };

	/*
	 * Input
	 */

	v2::PropertyIn<si::Angle>	cmd_pitch				{ this, "/cmd-pitch" };
	v2::PropertyIn<si::Angle>	cmd_roll				{ this, "/cmd-roll" };
	v2::PropertyIn<si::Angle>	measured_pitch			{ this, "/measured-pitch" };
	v2::PropertyIn<si::Angle>	measured_roll			{ this, "/measured-roll" };
	v2::PropertyIn<si::Angle>	elevator_minimum		{ this, "/limits/elevator/minimum" };
	v2::PropertyIn<si::Angle>	elevator_maximum		{ this, "/limits/elevator/maximum" };
	v2::PropertyIn<si::Angle>	ailerons_minimum		{ this, "/limits/ailerons/minimum" };
	v2::PropertyIn<si::Angle>	ailerons_maximum		{ this, "/limits/ailerons/maximum" };

	/*
	 * Output
	 */

	v2::PropertyOut<bool>		serviceable				{ this, "/serviceable" };
	v2::PropertyOut<si::Angle>	elevator				{ this, "/elevator" };
	v2::PropertyOut<si::Angle>	ailerons				{ this, "/ailerons" };
};


/**
 * Steers control surfaces (ailerons, elevator) to obtain desired orientation (pitch, roll).
 */
class AFCS_AP: public v2::Module<AFCS_AP_IO>
{
  public:
	// Ctor
	explicit
	AFCS_AP (std::unique_ptr<AFCS_AP_IO>, std::string const& instance = {});

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
	v2::PropertyObserver					_ap_computer;
};

#endif

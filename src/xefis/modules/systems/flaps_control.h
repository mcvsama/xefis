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

#ifndef XEFIS__MODULES__SYSTEMS__FLAPS_CONTROL_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__FLAPS_CONTROL_H__INCLUDED

// Standard:
#include <cstddef>
#include <set>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/module.h>
#include <xefis/core/v2/property.h>
#include <xefis/core/v2/setting.h>
#include <xefis/utility/range.h>
#include <xefis/utility/v2/actions.h>


class FlapsControl: public x2::Module
{
	static constexpr si::Time kUpdateInterval = 10_ms;

  public:
	/*
	 * Settings
	 */

	// How fast flaps should extend/retract:
	x2::Setting<si::AngularVelocity>	setting_angular_velocity	{ this, 10_deg / 1_s };
	// Range of output_control property:
	x2::Setting<xf::Range<double>>		setting_control_extents		{ this, { 0.0, 1.0 } };

	/*
	 * Input
	 */

	x2::PropertyIn<bool>				input_up					{ this, "/input/up", false };
	x2::PropertyIn<bool>				input_down					{ this, "/input/down", false };
	x2::PropertyIn<si::Angle>			input_setting				{ this, "/input/setting", 0_deg };

	/*
	 * Output
	 */

	x2::PropertyOut<si::Angle>			output_setting				{ this, "/output/setting" };
	x2::PropertyOut<si::Angle>			output_current				{ this, "/output/current" };
	x2::PropertyOut<double>				output_control				{ this, "/output/control" };

  public:
	// Ctor
	FlapsControl (xf::Airframe&, std::string const& instance = {});

  protected:
	// Module API
	void
	process (x2::Cycle const&) override;

  private:
	void
	update_flap_position();

  private:
	std::set<si::Angle>			_settings_list;
	xf::Range<si::Angle>		_extents;
	si::Angle					_setting;
	si::Angle					_current;
	Unique<QTimer>				_timer;
	x2::PropChangedTo<bool>		_input_up_clicked		{ input_up, true }; // TODO C++17 parameter deduction
	x2::PropChangedTo<bool>		_input_down_clicked		{ input_down, true }; // TODO C++17 parameter deduction
	x2::PropChanged<si::Angle>	_input_setting_changed	{ input_setting }; // TODO C++17 parameter deduction
};

#endif

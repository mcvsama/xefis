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

#ifndef XEFIS__MODULES__SYSTEMS__FLAPS_BUGS_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__FLAPS_BUGS_H__INCLUDED

// Standard:
#include <cstddef>
#include <set>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/module.h>
#include <xefis/core/v2/property.h>
#include <xefis/core/v2/setting.h>
#include <xefis/support/airframe/flaps.h>
#include <xefis/utility/v2/actions.h>


/**
 * Computes two speed bugs - for two adjacent flap settings - that should
 * be displayed on EFIS' speed ladder.
 */
class FlapsBugs: public x2::Module
{
  public:
	/*
	 * Settings
	 */

	x2::Setting<double>				setting_margin_factor	{ this, 1.2 };

	/*
	 * Input
	 */

	x2::PropertyIn<si::Angle>		input_flaps_setting		{ this, "/flaps-setting" };

	/*
	 * Output
	 */

	x2::PropertyOut<std::string>	output_flaps_up_label	{ this, "/flaps-up-label" };
	x2::PropertyOut<si::Velocity>	output_flaps_up_speed	{ this, "/flaps-up-speed" };
	x2::PropertyOut<std::string>	output_flaps_a_label	{ this, "/flaps-a-label" };
	x2::PropertyOut<si::Velocity>	output_flaps_a_speed	{ this, "/flaps-a-speed" };
	x2::PropertyOut<std::string>	output_flaps_b_label	{ this, "/flaps-b-label" };
	x2::PropertyOut<si::Velocity>	output_flaps_b_speed	{ this, "/flaps-b-speed" };

  public:
	// Ctor
	FlapsBugs (xf::Flaps const& flaps, std::string const& instance = {});

  protected:
	// Module API
	void
	process (x2::Cycle const&) override;

  private:
	xf::Flaps const&			_flaps;
	x2::PropChanged<si::Angle>	_flaps_setting_changed	{ input_flaps_setting };
};

#endif

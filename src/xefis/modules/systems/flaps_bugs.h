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


class FlapsBugsIO: public v2::ModuleIO
{
  public:
	/*
	 * Settings
	 */

	v2::Setting<double>				setting_margin_factor	{ this, 1.2 };

	/*
	 * Input
	 */

	v2::PropertyIn<si::Angle>		input_flaps_setting		{ this, "/flaps-setting" };

	/*
	 * Output
	 */

	v2::PropertyOut<std::string>	output_flaps_up_label	{ this, "/flaps-up-label" };
	v2::PropertyOut<si::Velocity>	output_flaps_up_speed	{ this, "/flaps-up-speed" };
	v2::PropertyOut<std::string>	output_flaps_a_label	{ this, "/flaps-a-label" };
	v2::PropertyOut<si::Velocity>	output_flaps_a_speed	{ this, "/flaps-a-speed" };
	v2::PropertyOut<std::string>	output_flaps_b_label	{ this, "/flaps-b-label" };
	v2::PropertyOut<si::Velocity>	output_flaps_b_speed	{ this, "/flaps-b-speed" };
};


/**
 * Computes two speed bugs - for two adjacent flap settings - that should
 * be displayed on EFIS' speed ladder.
 */
class FlapsBugs: public v2::Module<FlapsBugsIO>
{
  public:
	// Ctor
	explicit
	FlapsBugs (std::unique_ptr<FlapsBugsIO>, xf::Flaps const& flaps, std::string const& instance = {});

  protected:
	// Module API
	void
	process (v2::Cycle const&) override;

  private:
	xf::Flaps const&			_flaps;
	v2::PropChanged<si::Angle>	_flaps_setting_changed	{ io.input_flaps_setting };
};

#endif

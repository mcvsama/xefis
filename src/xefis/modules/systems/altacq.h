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

#ifndef XEFIS__MODULES__SYSTEMS__ALTACQ_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__ALTACQ_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/module.h>
#include <xefis/core/v2/property.h>
#include <xefis/core/v2/property_observer.h>
#include <xefis/core/v2/setting.h>
#include <xefis/utility/v2/actions.h>
#include <xefis/utility/smoother.h>


class AltAcq: public x2::Module
{
  public:
	/*
	 * Settings
	 */

	x2::Setting<si::Length>			setting_minimum_altitude_difference	{ this };
	x2::Setting<si::Length>			setting_flag_diff_on				{ this, 1000_ft };
	x2::Setting<si::Length>			setting_flag_diff_off				{ this, 100_ft };

	/*
	 * Input
	 */

	x2::PropertyIn<si::Length>		input_altitude_amsl					{ this, "/altitude-amsl" };
	x2::PropertyIn<si::Length>		input_altitude_acquire_amsl			{ this, "/altitude-acquire-amsl" };
	x2::PropertyIn<si::Velocity>	input_vertical_speed				{ this, "/vertical-speed" };
	x2::PropertyIn<si::Velocity>	input_ground_speed					{ this, "/ground-speed" };

	/*
	 * Output
	 */

	x2::PropertyOut<si::Length>		output_altitude_acquire_distance	{ this, "/acquire-distance" };
	x2::PropertyOut<bool>			output_altitude_acquire_flag		{ this, "/acquire-flag" };

  public:
	// Ctor
	explicit
	AltAcq (std::string const& instance = {});

  protected:
	// Module API
	void
	process (x2::Cycle const&) override;

	void
	compute_altitude_acquire_distance();

  private:
	bool						_flag_armed							{ false };
	// Note: PropertyObservers depend on Smoothers, so first Smoothers must be defined,
	// then PropertyObservers, to ensure correct order of destruction.
	xf::Smoother<si::Length>	_output_smoother					{ 2_s };
	x2::PropertyObserver		_output_computer;
	x2::PropChanged<si::Length>	_altitude_amsl_changed				{ input_altitude_amsl };
	x2::PropChanged<si::Length>	_altitude_acquire_amsl_changed		{ input_altitude_acquire_amsl };
};

#endif

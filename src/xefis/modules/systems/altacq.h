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


class AltAcqIO: public v2::ModuleIO
{
  public:
	/*
	 * Settings
	 */

	v2::Setting<si::Length>			minimum_altitude_difference	{ this, "minimum_altitude_difference" };
	v2::Setting<si::Length>			flag_diff_on				{ this, "flag_diff_on", 1000_ft };
	v2::Setting<si::Length>			flag_diff_off				{ this, "flag_diff_off", 100_ft };

	/*
	 * Input
	 */

	v2::PropertyIn<si::Length>		altitude_amsl				{ this, "/altitude-amsl" };
	v2::PropertyIn<si::Length>		altitude_acquire_amsl		{ this, "/altitude-acquire-amsl" };
	v2::PropertyIn<si::Velocity>	vertical_speed				{ this, "/vertical-speed" };
	v2::PropertyIn<si::Velocity>	ground_speed				{ this, "/ground-speed" };

	/*
	 * Output
	 */

	v2::PropertyOut<si::Length>		altitude_acquire_distance	{ this, "/acquire-distance" };
	v2::PropertyOut<bool>			altitude_acquire_flag		{ this, "/acquire-flag" };
};


class AltAcq: public v2::Module<AltAcqIO>
{
  public:
	// Ctor
	explicit
	AltAcq (std::unique_ptr<AltAcqIO>, std::string const& instance = {});

  protected:
	// Module API
	void
	process (v2::Cycle const&) override;

	void
	compute_altitude_acquire_distance();

  private:
	bool						_flag_armed							{ false };
	// Note: PropertyObservers depend on Smoothers, so first Smoothers must be defined,
	// then PropertyObservers, to ensure correct order of destruction.
	xf::Smoother<si::Length>	_output_smoother					{ 2_s };
	v2::PropertyObserver		_output_computer;
	v2::PropChanged<si::Length>	_altitude_amsl_changed				{ io.altitude_amsl };
	v2::PropChanged<si::Length>	_altitude_acquire_amsl_changed		{ io.altitude_acquire_amsl };
};

#endif

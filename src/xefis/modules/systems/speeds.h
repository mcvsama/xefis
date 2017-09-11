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

#ifndef XEFIS__MODULES__SYSTEMS__SPEEDS_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__SPEEDS_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/module.h>
#include <xefis/core/v2/property.h>
#include <xefis/core/v2/property_observer.h>


class SpeedsIO: public v2::ModuleIO
{
  public:
	/*
	 * Input
	 */

	v2::PropertyIn<si::Angle>		flaps_angle				{ this, "/flaps-angle" };
	v2::PropertyIn<si::Velocity>	stall_speed_5deg		{ this, "/stall-speed" };

	/*
	 * Output
	 */

	v2::PropertyOut<si::Velocity>	speed_minimum			{ this, "speed.minimum" };
	v2::PropertyOut<si::Velocity>	speed_minimum_maneuver	{ this, "speed.minimum-maneuver" };
	v2::PropertyOut<si::Velocity>	speed_maximum_maneuver	{ this, "speed.maximum-maneuver" };
	v2::PropertyOut<si::Velocity>	speed_maximum			{ this, "speed.maximum" };
};


class Speeds: public v2::Module<SpeedsIO>
{
  public:
	// Ctor
	explicit
	Speeds (std::unique_ptr<SpeedsIO>, xf::Airframe*, std::string const& instance = {});

	// Module API
	void
	process (v2::Cycle const&) override;

  private:
	void
	compute();

	template<class T>
		static T
		max (std::optional<T>, T);

	template<class T>
		static T
		min (std::optional<T>, T);

  private:
	xf::Airframe*			_airframe;
	v2::PropertyObserver	_speeds_computer;
};

#endif


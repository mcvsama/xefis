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

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/airframe/airframe.h>

// Local:
#include "speeds.h"


Speeds::Speeds (xf::Airframe* airframe, std::string const& instance):
	Module (instance),
	_airframe (airframe)
{
	_speeds_computer.set_callback (std::bind (&Speeds::compute, this));
	_speeds_computer.observe ({
		&input_flaps_angle,
		&input_stall_speed_5deg,
	});
}


void
Speeds::process (x2::Cycle const& cycle)
{
	_speeds_computer.process (cycle.update_time());
}


void
Speeds::compute()
{
	xf::Flaps const& flaps = _airframe->flaps();

	Optional<si::Velocity> minimum;
	Optional<si::Velocity> maximum;

	// Flaps speed limits:
	if (input_flaps_angle)
	{
		auto flaps_range = flaps.get_speed_range (*input_flaps_angle);
		minimum = max (minimum, flaps_range.min());
		maximum = min (maximum, flaps_range.max());
	}

	// Stall speed:
	if (input_stall_speed_5deg)
		minimum = max (minimum, *input_stall_speed_5deg);

	// TODO compute output_speed_minimum, output_speed_maximum

	output_speed_minimum_maneuver = minimum;
	output_speed_maximum_maneuver = maximum;
}


template<class T>
	inline T
	Speeds::max (Optional<T> opt_val, T val)
	{
		if (opt_val)
			return std::max (*opt_val, val);
		return val;
	}


template<class T>
	inline T
	Speeds::min (Optional<T> opt_val, T val)
	{
		if (opt_val)
			return std::min (*opt_val, val);
		return val;
	}


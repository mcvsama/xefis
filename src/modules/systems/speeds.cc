/* vim:ts=4
 *
 * Copyleft 2012…2014  Michał Gawron
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

// Qt:
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module_manager.h>
#include <xefis/core/application.h>
#include <xefis/airframe/airframe.h>

// Local:
#include "speeds.h"


XEFIS_REGISTER_MODULE_CLASS ("systems/speeds", Speeds);


Speeds::Speeds (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	parse_properties (config, {
		{ "input.flaps-angle", _input_flaps_angle, false },
		{ "output.speed.minimum", _output_speed_minimum, true },
		{ "output.speed.minimum-maneuver", _output_speed_minimum_maneuver, true },
		{ "output.speed.maximum-maneuver", _output_speed_maximum_maneuver, true },
		{ "output.speed.maximum", _output_speed_maximum, true },
	});

	_speeds_computer.set_callback (std::bind (&Speeds::compute, this));
	_speeds_computer.observe ({
		&_input_flaps_angle,
	});
}


void
Speeds::data_updated()
{
	_speeds_computer.data_updated (update_time());
}


void
Speeds::compute()
{
	Xefis::Flaps const& flaps = module_manager()->application()->airframe()->flaps();

	Optional<Speed> minimum;
	Optional<Speed> maximum;

	if (_input_flaps_angle.valid())
	{
		auto flaps_range = flaps.get_speed_range (*_input_flaps_angle);
		minimum = max (minimum, flaps_range.min());
		maximum = min (maximum, flaps_range.max());
	}

	_output_speed_minimum_maneuver = minimum;
	_output_speed_maximum_maneuver = maximum;
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


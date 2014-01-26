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

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "mixer.h"


XEFIS_REGISTER_MODULE_CLASS ("systems/mixer", Mixer);


Mixer::Mixer (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	parse_settings (config, {
		{ "input-a-factor", _input_a_factor, true },
		{ "input-b-factor", _input_b_factor, true },
	});

	parse_properties (config, {
		{ "input.a.value", _input_a_value, true },
		{ "input.b.value", _input_b_value, true },
		{ "output.value", _output_value, true },
	});
}


void
Mixer::data_updated()
{
	if (_input_a_value.fresh() || _input_b_value.fresh())
	{
		Optional<double> a = _input_a_value.get_optional();
		Optional<double> b = _input_b_value.get_optional();

		if (a || b)
		{
			double sum = 0.0;
			if (a)
				sum += _input_a_factor * *a;
			if (b)
				sum += _input_b_factor * *b;

			_output_value = sum;
		}
		else
			_output_value.set_nil();
	}
}


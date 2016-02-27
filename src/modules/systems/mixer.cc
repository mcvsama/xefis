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

// Local:
#include "mixer.h"


XEFIS_REGISTER_MODULE_CLASS ("systems/mixer", Mixer)


Mixer::Mixer (xf::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	parse_settings (config, {
		{ "input.0.factor", _input_0_factor, false },
		{ "input.1.factor", _input_1_factor, false },
		{ "output-minimum", _output_minimum, false },
		{ "output-maximum", _output_maximum, false },
	});

	parse_properties (config, {
		{ "input.0.value", _input_0_value, true },
		{ "input.1.value", _input_1_value, true },
		{ "output.value", _output_value, true },
	});

	if (_output_minimum && _output_maximum && *_output_minimum > *_output_maximum)
		log() << "Warning: maximum value is less than the minimum value." << std::endl;
}


void
Mixer::data_updated()
{
	if (_input_0_value.fresh() || _input_1_value.fresh())
	{
		Optional<double> a = _input_0_value.get_optional();
		Optional<double> b = _input_1_value.get_optional();

		if (a || b)
		{
			double sum = 0.0;
			if (a)
				sum += _input_0_factor * *a;
			if (b)
				sum += _input_1_factor * *b;

			if (_output_minimum && sum < *_output_minimum)
				sum = *_output_minimum;

			if (_output_maximum && sum > *_output_maximum)
				sum = *_output_maximum;

			_output_value = sum;
		}
		else
			_output_value.set_nil();
	}
}


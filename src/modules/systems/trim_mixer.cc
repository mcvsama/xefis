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
#include <xefis/core/module.h>
#include <xefis/core/property.h>
#include <xefis/core/property_observer.h>
#include <xefis/core/module_manager.h>
#include <xefis/core/sound_manager.h>

// Local:
#include "trim_mixer.h"


XEFIS_REGISTER_MODULE_CLASS ("systems/trim-mixer", TrimMixer);


TrimMixer::TrimMixer (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	parse_properties (config, {
		{ "input.axis", _input_axis, true },
		{ "input.trim-value", _input_trim_value, true },
		{ "output.axis", _output_axis, true },
	});

	_mix_computer.set_callback (std::bind (&TrimMixer::compute_mix, this));
	_mix_computer.observe ({
		&_input_axis,
		&_input_trim_value,
	});
}


void
TrimMixer::data_updated()
{
	_mix_computer.data_updated (update_time());
}


void
TrimMixer::compute_mix()
{
	if (_input_axis.valid() && _input_trim_value.valid())
		_output_axis = Xefis::limit (*_input_axis + *_input_trim_value, -1.0, 1.0);
	else
		_output_axis.set_nil();
}


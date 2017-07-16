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
#include <xefis/core/stdexcept.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/numeric.h>

// Local:
#include "transistor.h"


XEFIS_REGISTER_MODULE_CLASS ("helpers/transistor", Transistor)


Transistor::Transistor (v1::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	parse_settings (config, {
		{ "transition-time", _transition_time, true },
	});

	parse_properties (config, {
		{ "input.0.value", _input_0_value, true },
		{ "input.1.value", _input_1_value, true },
		{ "input.selected", _input_selected, true },
		{ "output.value", _output_value, true },
	});

	_transistor = std::make_unique<xf::Transistor<v1::PropertyFloat::Type>> (_transition_time);

	_observer.set_callback (std::bind (&Transistor::input_changed, this));
	_observer.observe ({
		&_input_0_value,
		&_input_1_value,
		&_input_selected,
	});
}


void
Transistor::data_updated()
{
	_observer.data_updated (update_time());
}


void
Transistor::input_changed()
{
	Time dt = _observer.update_dt();

	int v = _input_selected.read (0);
	// If incorrect, fallback to 0, which is the default:
	if (v != 0 && v != 1)
		v = 0;

	if (v == 0)
		_transistor->select_input<0>();
	else
		_transistor->select_input<1>();

	auto v0 = _input_0_value.read (_last_0_value);
	auto v1 = _input_1_value.read (_last_1_value);

	_output_value = _transistor->process (v0, v1, dt);

	if (_input_0_value.valid())
		_last_0_value = *_input_0_value;
	if (_input_1_value.valid())
		_last_1_value = *_input_1_value;
}


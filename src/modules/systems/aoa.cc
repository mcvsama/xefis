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
#include <xefis/config/exception.h>
#include <xefis/utility/qdom.h>

// Local:
#include "aoa.h"


XEFIS_REGISTER_MODULE_CLASS ("systems/aoa", AOA);


AOA::AOA (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	parse_settings (config, {
		{ "normal-critical-aoa", _normal_critical_aoa, true },
		{ "flaps-factor", _setting_flaps_factor, false },
		{ "spoilers-factor", _setting_spoilers_factor, false },
	});

	parse_properties (config, {
		// Input:
		{ "input.flaps-angle", _input_flaps_angle, false },
		{ "input.spoilers-angle", _input_spoilers_angle, false },
		{ "input.aoa.alpha", _input_aoa_alpha, false },
		// Output:
		{ "output.critical-aoa", _output_critical_aoa, true },
		{ "output.stall", _output_stall, false },
	});

	_critical_aoa_computer.set_minimum_dt (1_ms);
	_critical_aoa_computer.set_callback (std::bind (&AOA::compute_critical_aoa, this));
	_critical_aoa_computer.observe ({
		&_input_flaps_angle,
		&_input_spoilers_angle,
		&_input_aoa_alpha,
	});
}


void
AOA::data_updated()
{
	_critical_aoa_computer.data_updated (update_time());
}


void
AOA::compute_critical_aoa()
{
	Angle result = _normal_critical_aoa;
	if (_input_flaps_angle.valid())
		result += _setting_flaps_factor * *_input_flaps_angle;
	if (_input_spoilers_angle.valid())
		result += _setting_spoilers_factor * *_input_spoilers_angle;

	_output_critical_aoa = result;

	if (_output_stall.configured())
	{
		if (_input_aoa_alpha.valid())
			_output_stall = *_input_aoa_alpha >= *_output_critical_aoa;
		else
			_output_stall.set_nil();
	}
}


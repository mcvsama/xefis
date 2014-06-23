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
#include "engine_torque.h"


XEFIS_REGISTER_MODULE_CLASS ("systems/engine-torque", EngineTorque);


EngineTorque::EngineTorque (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	parse_settings (config, {
		{ "linear-coefficient", _linear_coefficient, true },
		{ "derivative-coefficient", _derivative_coefficient, true },
		{ "total-coefficient", _total_coefficient, false },
	});

	parse_properties (config, {
		{ "input.engine-rpm", _input_engine_rpm, true },
		{ "output.engine-torque", _output_engine_torque, true },
	});
}


void
EngineTorque::data_updated()
{
	if (_input_engine_rpm.fresh())
	{
		if (_input_engine_rpm.valid())
		{
			if (_previous_engine_spd)
			{
				Frequency engine_spd = *_input_engine_rpm;
				Frequency derivative = (engine_spd - _previous_engine_spd->value()) / (update_time() - _previous_engine_spd->update_time()).s();
				_output_engine_torque = 1_Nm * (_derivative_coefficient * derivative + _linear_coefficient * engine_spd).Hz();
			}
			else
				_previous_engine_spd = *_input_engine_rpm;
		}
		else
		{
			_previous_engine_spd.reset();
			_output_engine_torque.set_nil();
		}
	}
}


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
#include <xefis/utility/qdom.h>
#include <xefis/utility/numeric.h>

// Local:
#include "afcs_roll_autotrim.h"


XEFIS_REGISTER_MODULE_CLASS ("systems/afcs-roll-autotrim", AFCS_RollAutotrim);


AFCS_RollAutotrim::AFCS_RollAutotrim (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	parse_settings (config, {
		{ "airspeed-coefficient", _airspeed_coefficient, true },
		{ "engine-torque-coefficient", _engine_torque_coefficient, true },
		{ "total-coefficient", _total_coefficient, true },
	});

	parse_properties (config, {
		{ "input.ias", _input_airspeed, true },
		{ "input.engine-torque", _input_engine_torque, true },
		{ "output.ailerons-correction", _output_ailerons_correction, true },
	});
}


void
AFCS_RollAutotrim::data_updated()
{
	if (_input_airspeed.fresh() || _input_engine_torque.fresh())
	{
		if (_input_airspeed.valid() && _input_engine_torque.valid())
		{
			xf::PropertyFloat::Type correction = _airspeed_coefficient / _input_airspeed->mps()
											   + _engine_torque_coefficient * _input_engine_torque->Nm();
			_output_ailerons_correction = _total_coefficient * correction;
		}
		else
			_output_ailerons_correction.set_nil();
	}
}


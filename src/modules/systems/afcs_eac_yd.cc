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
#include "afcs_eac_yd.h"


XEFIS_REGISTER_MODULE_CLASS ("systems/afcs-eac-yd", AFCS_EAC_YD);


AFCS_EAC_YD::AFCS_EAC_YD (xf::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config),
	_rudder_pid (1.0, 0.1, 0.0, 0.0)
{
	parse_settings (config, {
		{ "rudder-p", _rudder_p, true },
		{ "rudder-i", _rudder_i, true },
		{ "rudder-d", _rudder_d, true },
		{ "rudder-gain", _rudder_gain, true },
		{ "limit", _limit, true },
	});

	parse_properties (config, {
		{ "input.enabled", _input_enabled, true },
		{ "input.slip-skid-g", _input_slip_skid_g, true },
		{ "output.rudder", _output_rudder, true },
	});

	_rudder_pid.set_pid (_rudder_p, _rudder_i, _rudder_d);
	_rudder_pid.set_gain (_rudder_gain);
	_rudder_pid.set_i_limit ({ -0.1f, +0.1f });
	_rudder_pid.set_output_limit ({ -_limit, +_limit });

	_rudder_computer.set_callback (std::bind (&AFCS_EAC_YD::compute, this));
	_rudder_computer.observe ({
		&_input_enabled,
		&_input_slip_skid_g,
	});
}


void
AFCS_EAC_YD::data_updated()
{
	_rudder_computer.data_updated (update_time());
}


void
AFCS_EAC_YD::compute()
{
	Time dt = _rudder_computer.update_dt();

	if (_input_enabled.read (false))
	{
		if (_input_slip_skid_g.valid())
		{
			_rudder_pid.set_target (0.0);
			_rudder_pid.process (*_input_slip_skid_g, dt);
			_output_rudder = _rudder_pid.output();
		}
		else
			_output_rudder.set_nil();
	}
	else
		_output_rudder = 0.0;
}


/* vim:ts=4
 *
 * Copyleft 2012…2013  Michał Gawron
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
#include "autothrottle.h"


XEFIS_REGISTER_MODULE_CLASS ("systems/autothrottle", Autothrottle);


Autothrottle::Autothrottle (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config),
	_thrust_pid (_thrust_pid_p, _thrust_pid_i, _thrust_pid_d, 0.0),
	_ias_pid (_ias_pid_p, _ias_pid_i, _ias_pid_d, 0.0)
{
	for (auto* pid: { &_thrust_pid, &_ias_pid })
		pid->set_i_limit ({ -0.05f, +0.05f });

	parse_settings (config, {
		{ "thrust.pid.p", _thrust_pid_p, false },
		{ "thrust.pid.i", _thrust_pid_i, false },
		{ "thrust.pid.d", _thrust_pid_d, false },
		{ "ias.pid.p", _ias_pid_p, false },
		{ "ias.pid.i", _ias_pid_i, false },
		{ "ias.pid.d", _ias_pid_d, false },
	});

	parse_properties (config, {
		{ "power-limit.max", _power_limit_max, true },
		{ "power-limit.min", _power_limit_min, true },
		{ "cmd.speed-mode", _cmd_speed_mode, true },
		{ "cmd.thrust", _cmd_thrust, true },
		{ "cmd.ias", _cmd_ias, true },
		{ "measured.thrust", _measured_thrust, true },
		{ "measured.ias", _measured_ias, true },
		{ "output.power", _output_power, true },
		{ "disengage-at", _disengage_at, true },
	});

	_thrust_pid.set_pid (_thrust_pid_p, _thrust_pid_i, _thrust_pid_d),
	_ias_pid.set_pid (_ias_pid_p, _ias_pid_i, _ias_pid_d),

	speed_mode_changed();
}


void
Autothrottle::data_updated()
{
	// Don't process if dt is too small:
	_dt += update_dt();
	if (_dt < 0.005_s)
		return;

	if (*_enabled)
	{
		// TODO
	}
	else
	{
		_computed_output_power = 0.0;
	}

	_output_power.write (_output_power_smoother.process (_computed_output_power, _dt));

	_dt = Time::now();
}


void
Autothrottle::speed_mode_changed()
{
	Xefis::PropertyInteger::Type m = Xefis::limit<decltype (m)> (_cmd_speed_mode.read (-1), 0, static_cast<decltype (m)> (SpeedMode::sentinel) - 1);
	_speed_mode = static_cast<SpeedMode> (m);
}


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
		{ "ias-to-throttle-scale", _ias_to_throttle_scale, false },
	});

	parse_properties (config, {
		{ "cmd.speed-mode", _cmd_speed_mode, true },
		{ "cmd.thrust", _cmd_thrust, true },
		{ "cmd.ias", _cmd_ias, true },
		{ "measured.thrust", _measured_thrust, true },
		{ "measured.ias", _measured_ias, true },
		{ "output.throttle", _output_throttle, true },
		{ "disengage-at", _disengage_at, true },
	});

	// Update PID params according to settings:
	_thrust_pid.set_pid (_thrust_pid_p, _thrust_pid_i, _thrust_pid_d),
	_ias_pid.set_pid (_ias_pid_p, _ias_pid_i, _ias_pid_d),

	speed_mode_changed();
}


void
Autothrottle::data_updated()
{
	bool disengage = false;
	double computed_thrust = 0.0;

	// Don't process if dt is too small:
	_dt += update_dt();
	if (_dt < 5_ms)
		return;

	if (_cmd_speed_mode.fresh())
		speed_mode_changed();

	switch (_speed_mode)
	{
		case SpeedMode::Thrust:
			if (_cmd_thrust.is_nil() || _measured_thrust.is_nil())
			{
				_thrust_pid.reset();
				disengage = true;
			}
			else
			{
				_thrust_pid.set_target (*_cmd_thrust);
				_thrust_pid.process (*_measured_thrust, _dt.s());
				computed_thrust = _thrust_pid.output();
			}
			break;

		case SpeedMode::Airspeed:
			if (_cmd_ias.is_nil() || _measured_ias.is_nil())
			{
				_ias_pid.reset();
				disengage = true;
			}
			else
			{
				// This is more tricky, since we measure IAS, but control thrust.
				// There's no 1:1 correlaction between them.
				_ias_pid.set_target (_cmd_ias->kt());
				_ias_pid.process (_measured_ias->kt(), _dt.s());
				computed_thrust = Xefis::limit (_ias_pid.output() / _ias_to_throttle_scale, -1.0, 1.0);
				computed_thrust = Xefis::renormalize (computed_thrust, { -1.0, 1.0 }, { 0.0, 1.0 });
			}
			break;

		case SpeedMode::None:
		case SpeedMode::sentinel:
			_computed_output_throttle = 0.0;
			break;
	}

	_output_throttle.write (_output_throttle_smoother.process (computed_thrust, _dt));

	if (disengage || _disengage_at.is_nil())
		_disengage_at.write (disengage);

	_dt = 0_s;
}


void
Autothrottle::speed_mode_changed()
{
	Xefis::PropertyInteger::Type m = Xefis::limit<decltype (m)> (_cmd_speed_mode.read (-1), 0, static_cast<decltype (m)> (SpeedMode::sentinel) - 1);
	_speed_mode = static_cast<SpeedMode> (m);
}


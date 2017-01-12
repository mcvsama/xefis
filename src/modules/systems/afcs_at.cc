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

// Qt:
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/numeric.h>
#include <xefis/utility/range.h>

// Local:
#include "afcs_at.h"


XEFIS_REGISTER_MODULE_CLASS ("systems/afcs-at", AFCS_AT)


AFCS_AT::AFCS_AT (xf::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	_ias_pid.set_integral_limit ({ -5.0_m, +5.0_m });

	parse_settings (config, {
		{ "output.thrust.minimum", _output_thrust_minimum, true },
		{ "output.thrust.maximum", _output_thrust_maximum, true },
		{ "ias.pid.p", _ias_pid_p, false },
		{ "ias.pid.i", _ias_pid_i, false },
		{ "ias.pid.d", _ias_pid_d, false },
	});

	parse_properties (config, {
		{ "cmd.speed-mode", _cmd_speed_mode, true },
		{ "cmd.thrust", _cmd_thrust, true },
		{ "cmd.ias", _cmd_ias, true },
		{ "measured.ias", _measured_ias, true },
		{ "output.thrust", _output_thrust, true },
		{ "disengage-at", _disengage_at, true },
	});

	// Extents:
	_output_thrust_extent = { _output_thrust_minimum, _output_thrust_maximum };
	// Update PID params according to settings:
	_ias_pid.set_pid ({ _ias_pid_p, _ias_pid_i, _ias_pid_d });

	_thrust_computer.set_minimum_dt (5_ms);
	_thrust_computer.set_callback (std::bind (&AFCS_AT::compute_thrust, this));
	_thrust_computer.add_depending_smoothers ({
		&_ias_pid_smoother,
	});
	_thrust_computer.observe ({
		&_cmd_speed_mode,
		&_cmd_thrust,
		&_cmd_ias,
		&_measured_ias,
	});

	speed_mode_changed();
}


void
AFCS_AT::data_updated()
{
	_thrust_computer.data_updated (update_time());
}


void
AFCS_AT::compute_thrust()
{
	bool disengage = false;
	si::Force computed_thrust = 0.0_N;
	Time dt = _thrust_computer.update_dt();

	if (_cmd_speed_mode.fresh())
		speed_mode_changed();

	switch (_speed_mode)
	{
		case SpeedMode::Thrust:
			if (_cmd_thrust.is_nil())
				disengage = true;
			else
				computed_thrust = *_cmd_thrust;
			break;

		case SpeedMode::Airspeed:
			if (_cmd_ias.is_nil() || _measured_ias.is_nil())
			{
				_ias_pid.reset();
				_ias_pid_smoother.reset();
				disengage = true;
			}
			else
			{
				// This is more tricky, since we measure IAS, but control thrust.
				// There's no 1:1 correlaction between them.
				// TODO use _ias_pid.set_output_limit (...);
				computed_thrust = xf::clamped (_ias_pid_smoother (_ias_pid (*_cmd_ias, *_measured_ias, dt), dt), _output_thrust_extent);
				// TODO make PID control the change rate of thrust, not the thrust directly. Maybe incorporate
				// something into the PIDControl object itself? Or create another function-like class.
			}
			break;

		case SpeedMode::None:
		case SpeedMode::sentinel:
			break;
	}

	_output_thrust.write (computed_thrust);

	if (disengage || _disengage_at.is_nil())
		_disengage_at.write (disengage);
}


void
AFCS_AT::speed_mode_changed()
{
	xf::PropertyInteger::Type m = xf::clamped<decltype (m)> (_cmd_speed_mode.read (-1), 0, static_cast<decltype (m)> (SpeedMode::sentinel) - 1);
	_speed_mode = static_cast<SpeedMode> (m);
}


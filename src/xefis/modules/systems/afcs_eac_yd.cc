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

// Local:
#include "afcs_eac_yd.h"

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>


AFCS_EAC_YD::AFCS_EAC_YD (std::string_view const& instance):
	AFCS_EAC_YD_IO (instance)
{
	_rudder_pid.set_integral_limit ({ -0.1_Ns, +0.1_Ns });
	_rudder_pid.set_output_limit ({ -*_io.deflection_limit, *_io.deflection_limit });

	_rudder_computer.set_callback (std::bind (&AFCS_EAC_YD::compute, this));
	_rudder_computer.observe ({
		&_io.enabled,
		&_io.slip_skid,
	});
}


void
AFCS_EAC_YD::initialize()
{
	_rudder_pid.set_pid (*_io.rudder_pid_settings);
	_rudder_pid.set_gain (*_io.rudder_pid_gain);
}


void
AFCS_EAC_YD::process (xf::Cycle const& cycle)
{
	_rudder_computer.process (cycle.update_time());
}


void
AFCS_EAC_YD::compute()
{
	si::Time dt = _rudder_computer.update_dt();

	if (_io.enabled.value_or (false))
	{
		if (_io.slip_skid)
			_io.rudder_deflection = _rudder_pid (0.0_N, *_io.slip_skid, dt);
		else
			_io.rudder_deflection = xf::nil;
	}
	else
		_io.rudder_deflection = 0.0_deg;
}


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
#include "afcs_roll_autotrim.h"

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>


AFCS_RollAutotrim::AFCS_RollAutotrim (std::string_view const& instance):
	AFCS_RollAutotrim_IO (instance)
{ }


void
AFCS_RollAutotrim::process (xf::Cycle const&)
{
	if (_io.measured_ias && _io.measured_engine_torque)
	{
		// TODO Do this correctly, now it's just too simple.
		auto ias_part = *_io.ias_coefficient / _io.measured_ias->in<si::MeterPerSecond>();
		auto torque_part = *_io.engine_torque_coefficient * _io.measured_engine_torque->in<si::NewtonMeter>();
		si::Angle correction = 1_deg * (ias_part + torque_part);

		_io.ailerons_correction = *_io.total_coefficient * correction;
	}
	else
		_io.ailerons_correction = xf::nil;
}


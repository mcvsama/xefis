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
#include "afcs_roll_autotrim.h"


AFCS_RollAutotrim::AFCS_RollAutotrim (std::unique_ptr<AFCS_RollAutotrim_IO> module_io, std::string const& instance):
	Module (std::move (module_io), instance)
{ }


void
AFCS_RollAutotrim::process (v2::Cycle const&)
{
	if (io.measured_ias && io.measured_engine_torque)
	{
		// TODO Do this correctly, now it's just too simple.
		auto ias_part = *io.ias_coefficient / io.measured_ias->quantity<MeterPerSecond>();
		auto torque_part = *io.engine_torque_coefficient * io.measured_engine_torque->quantity<NewtonMeter>();
		si::Angle correction = 1_deg * (ias_part + torque_part);

		io.ailerons_correction = *io.total_coefficient * correction;
	}
	else
		io.ailerons_correction.set_nil();
}


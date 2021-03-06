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
#include <variant>

// Neutrino:
#include <neutrino/variant.h>

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "engine_torque.h"


EngineTorque::EngineTorque (std::unique_ptr<EngineTorqueIO> module_io, std::string_view const& instance):
	Module (std::move (module_io), instance)
{ }


void
EngineTorque::process (xf::Cycle const&)
{
	std::visit ([&] (auto&& efficiency) {
		compute_torque (std::forward<decltype (efficiency)> (efficiency));
	}, *io.motor_efficiency);
}


void
EngineTorque::compute_torque (double motor_efficiency)
{
	if (io.engine_current)
		io.engine_torque = si::convert (motor_efficiency * *io.engine_current / *io.motor_kv);
	else
		io.engine_torque = xf::nil;
}


void
EngineTorque::compute_torque (EfficiencyField const& motor_efficiency)
{
	std::optional<double> efficiency;

	if (io.engine_speed && (efficiency = motor_efficiency.value (*io.engine_speed)))
		compute_torque (*efficiency);
	else
		io.engine_torque = xf::nil;
}


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
#include "engine_torque.h"

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/variant.h>

// Standard:
#include <cstddef>
#include <variant>


EngineTorque::EngineTorque (std::string_view const& instance):
	EngineTorqueIO (instance)
{ }


void
EngineTorque::process (xf::Cycle const&)
{
	std::visit ([&] (auto&& efficiency) {
		compute_torque (std::forward<decltype (efficiency)> (efficiency));
	}, *_io.motor_efficiency);
}


void
EngineTorque::compute_torque (double motor_efficiency)
{
	if (_io.engine_current)
		_io.engine_torque = si::convert (motor_efficiency * *_io.engine_current / *_io.motor_kv);
	else
		_io.engine_torque = xf::nil;
}


void
EngineTorque::compute_torque (EfficiencyField const& motor_efficiency)
{
	std::optional<double> efficiency;

	if (_io.engine_speed && (efficiency = motor_efficiency.value (*_io.engine_speed)))
		compute_torque (*efficiency);
	else
		_io.engine_torque = xf::nil;
}


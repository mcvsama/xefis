/* vim:ts=4
 *
 * Copyleft 2008…2018  Michał Gawron
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
#include <cmath>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/space.h>
#include <xefis/support/nature/physics.h>

// Local:
#include "airframe.h"


namespace xf::sim {

Airframe::Airframe (BodyShape&& body_shape):
	Body (std::move (body_shape))
{ }


ForceTorque<ECEFFrame>
Airframe::forces (Atmosphere const& atmosphere) const
{
	ForceTorque<AirframeFrame> body_total;

	for (auto& part: shape().parts())
	{
		auto const atm = complete_atmosphere_state_at (part->aircraft_relative_position(), atmosphere);
		auto const part_force_torque = part->forces (atm);

		body_total += part_force_torque;
	}

	return body_to_base_rotation() * body_total;
}


AtmosphereState<AirframeFrame>
Airframe::complete_atmosphere_state_at (SpaceVector<si::Length, AirframeFrame> const com_relative_part_position, Atmosphere const& atmosphere) const
{
	auto const ecef_part_position = position() + body_to_base_rotation() * com_relative_part_position;
	auto const ecef_relative_wind = atmosphere.wind_at (ecef_part_position) - velocity();
	auto const body_relative_wind = base_to_body_rotation() * ecef_relative_wind;
	auto const body_angular_velocity = base_to_body_rotation() * angular_velocity();
	auto const body_rotation_wind = cross_product (-body_angular_velocity, com_relative_part_position);

	auto const air = atmosphere.air_at (ecef_part_position);
	auto const wind = body_relative_wind + body_rotation_wind;

	return { air, wind };
}

} // namespace xf::sim


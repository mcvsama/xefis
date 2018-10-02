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

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "body.h"


namespace xf::sim {

void
Body::evolve (SpaceVector<si::Force> const& force, SpaceMatrix<si::Torque> const& torque, si::Time dt)
{
	SpaceVector<si::Acceleration> linear_a = force / _mass;
	SpaceVector<si::Velocity> dv = linear_a * dt;

	SpaceMatrix<si::BaseAngularAcceleration> angular_a = _inversed_moment_of_inertia * torque;
	SpaceMatrix<si::BaseAngularVelocity> dw = angular_a * dt;

	_velocity += dv;
	_angular_velocity += dw;

	_position += _velocity * dt;
	_orientation += (_angular_velocity * dt) * _orientation;
}

} // namespace xf::sim


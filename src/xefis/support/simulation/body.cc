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
#include <xefis/support/math/geometry.h>

// Local:
#include "body.h"


namespace xf::sim {

Body::Body (BodyShape&& shape):
	_shape (std::move (shape))
{ }


void
Body::act (ForceTorque<ECEFFrame> const& force_torque, si::Time dt)
{
	SpaceVector<si::Acceleration, ECEFFrame> const linear_a = force_torque.force() / _shape.mass();
	SpaceVector<si::Velocity, ECEFFrame> const dv = linear_a * dt;

	SpaceVector<si::BaseAngularAcceleration, ECEFFrame> const angular_a = _body_to_ecef_transform * _shape.inversed_moment_of_inertia() * (_ecef_to_body_transform * force_torque.torque());
	SpaceVector<si::BaseAngularVelocity, ECEFFrame> const dw = angular_a * dt;

	_velocity += dv;
	_angular_velocity += dw;

	_position += _velocity * dt;
	_body_to_ecef_transform += make_pseudotensor (_angular_velocity * dt) * _body_to_ecef_transform;

	_body_to_ecef_transform = vector_normalized (orthogonalized (_body_to_ecef_transform));
	_ecef_to_body_transform = inv (_body_to_ecef_transform);
}

} // namespace xf::sim


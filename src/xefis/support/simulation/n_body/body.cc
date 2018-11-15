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
	auto const a = force_torque.force() / _shape.mass();
	auto const dv = a * dt;

	auto const& o2b = body_to_base_rotation();
	auto const& b2o = base_to_body_rotation();

	auto const angular_a = o2b * _shape.inversed_moment_of_inertia() * (b2o * force_torque.torque());
	auto const dw = angular_a * dt;

	_velocity += dv;
	_angular_velocity += dw;

	translate_frame (_velocity * dt);
	rotate_frame (_angular_velocity * dt);
}

} // namespace xf::sim


/* vim:ts=4
 *
 * Copyleft 2019  Michał Gawron
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
#include "angular_servo.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/geometry.h>

// Neutrino:
#include <neutrino/numeric.h>

// Standard:
#include <cstddef>
#include <array>


namespace xf::sim {

AngularServo::AngularServo (MassMoments<rigid_body::BodySpace> const& mass_moments):
	Body (mass_moments)
{ }


std::unique_ptr<AngularServo>
make_standard_9gram_servo()
{
	std::array<PointMass<rigid_body::BodySpace>, 8> masses;
	std::size_t i = 0;

	for (auto const x: { -11.5_mm, +11.5_mm })
		for (auto const y: { -6_mm, +6_mm })
			for (auto const z: { -14_mm, +14_mm })
				masses[i++] = { 9_gr / masses.size(), SpaceLength<rigid_body::BodySpace> (0.5 * x, 0.5 * y, 0.5 * z), math::zero };

	auto const mass_moments = MassMoments<rigid_body::BodySpace>::from_point_masses (begin (masses), end (masses));

	return std::make_unique<AngularServo> (mass_moments);
}

} // namespace xf::sim


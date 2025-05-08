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
#include "utility.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/nature/constants.h>
#include <xefis/support/nature/mass_moments.h>
#include <xefis/support/simulation/shapes/various_shapes.h>

// Standard:
#include <cstddef>
#include <memory>


namespace xf::rigid_body {

std::unique_ptr<Body>
make_earth (bool include_actual_sphere)
{
	auto const make_orb = [](si::Length const radius, std::size_t n_slices, std::size_t n_stacks) {
		// An orb colored with ECEF colors: X/Null Island = red, Y/(90°/0°) = green, Z/North = blue:
		return rigid_body::make_centered_sphere_shape ({
			.radius = radius,
			.n_slices = n_slices,
			.n_stacks = n_stacks,
			.material = rigid_body::kBlackMatte,
			.setup_material = [](rigid_body::ShapeMaterial& material, si::LonLat const position) {
				auto const [r, g, b] = to_cartesian (position);

				auto const pos_r = std::clamp<float> (r, 0.0f, +1.0f);
				auto const pos_g = std::clamp<float> (g, 0.0f, +1.0f);
				auto const pos_b = std::clamp<float> (b, 0.0f, +1.0f);

				auto const neg_r = std::clamp<float> (r, -1.0f, 0.0f);
				auto const neg_g = std::clamp<float> (g, -1.0f, 0.0f);
				auto const neg_b = std::clamp<float> (b, -1.0f, 0.0f);

				material.gl_ambient_color = GLColor();
				material.gl_diffuse_color = GLColor();
				material.gl_specular_color = GLColor();

				auto const power = [](float value) -> float {
					auto const v2 = value * value;
					return v2 * v2;
				};

				material.gl_emission_color = GLColor {
					power (pos_r) + power (-neg_g) + power (-neg_b),
					power (pos_g) + power (-neg_r) + power (-neg_b),
					power (pos_b) + power (-neg_r) + power (-neg_g),
				};
			},
		});
	};

	// Small 1_m orb:
	auto shape = make_orb (1_m, 18, 9);

	if (include_actual_sphere)
		shape += make_orb (kEarthMeanRadius, 360, 180);

	auto earth = std::make_unique<Body> (MassMoments<BodyCOM> (kEarthMass, math::coordinate_system_cast<BodyCOM, BodyCOM> (kEarthMomentOfInertia)));
	earth->set_shape (shape);
	return earth;
}

} // namespace xf::rigid_body


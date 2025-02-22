/* vim:ts=4
 *
 * Copyleft 2025  Michał Gawron
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
#include "cie_1931.h"

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>


namespace xf {

QColor
cie_xyz_to_rgb (math::Vector<double, 3> const& xyz)
{
	static constexpr auto transform = math::Matrix<float, 3, 3> {
		+3.2406, -1.5372, -0.4986,
		-0.9689, +1.8758, +0.0415,
		+0.0557, -0.2040, +1.0570,
	};

	auto rgb = transform * xyz;

	// Clamp negative values to 0:
	for (auto& v: rgb.array())
		v = std::max (v, 0.0);

	// Normalize if any component exceeds 1:
	auto const max = std::max ({ rgb[0], rgb[1], rgb[2] });

	if (max > 1.0)
		rgb = rgb / max;

	return QColor::fromRgbF (static_cast<float> (rgb[0]), static_cast<float> (rgb[1]), static_cast<float> (rgb[2]));
}

} // namespace xf


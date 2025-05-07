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
#include "hsl.h"

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>


namespace xf {

QColor
hsl_interpolation (float x, QColor const& color0, QColor const& color1)
{
	x = std::clamp (x, 0.0f, 1.0f);

	auto const hsv1 = color0.toHsl();
	auto const hsv2 = color1.toHsl();

	auto const h = static_cast<int> (hsv1.hue() + x * (hsv2.hue() - hsv1.hue()));
	auto const s = static_cast<int> (hsv1.saturation() + x * (hsv2.saturation() - hsv1.saturation()));
	auto const l = static_cast<int> (hsv1.lightness() + x * (hsv2.lightness() - hsv1.lightness()));
	auto const a = static_cast<int> (hsv1.alpha() + x * (hsv2.alpha() - hsv1.alpha()));

	return QColor::fromHsl (h, s, l, a);
}

} // namespace xf


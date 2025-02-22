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

#ifndef XEFIS__SUPPORT__COLOR__BLACKBODY_H__INCLUDED
#define XEFIS__SUPPORT__COLOR__BLACKBODY_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/nature/constants.h>

// Qt:
#include <QColor>

// Standard:
#include <cstddef>


namespace xf {

/**
 * Planck function for blackbody radiation at temperature T.
 * Returned SI unit is [W·m⁻³·sr⁻¹], si::SpectralRadiance
 */
[[nodiscard]]
constexpr si::SpectralRadiance
blackbody_spectral_radiance (si::Length const wavelength, si::Temperature const T)
{
	// Shorthands:
	constexpr auto h = kPlankConstant;
	constexpr auto c = kSpeedOfLight;
	constexpr auto k = kBoltzmannConstant;

	auto const numerator = 2 * h * c * c;
	auto const exponent = (h * c) / (wavelength * k * T);
	auto const denominator = pow<5> (wavelength) * (std::exp (exponent) - 1.0);

	return si::convert (numerator / denominator);
}


[[nodiscard]]
math::Vector<float, 2>
calculate_cie_xy_blackbody_color (si::Temperature const temperature, std::function<float (float)> const attenuator = nullptr);


/**
 * Calculate QColor from color temperature.
 */
[[nodiscard]]
QColor
qcolor_from_temperature (si::Temperature const temperature);

} // namespace xf

#endif


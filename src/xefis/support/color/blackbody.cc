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
#include "blackbody.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/color/cie_1931.h>

// Standard:
#include <cstddef>


namespace xf {

math::Vector<float, 2>
calculate_cie_xy_blackbody_color (si::Temperature const temperature, std::function<float (float)> const attenuator)
{
	// Using a simple table for the CIE 1931 2° Standard Observer at 10 nm intervals.

	// Reference optical depth parameters (for Rayleigh scattering).
	// We assume tau_ref at 550 nm is 0.1 (this is an assumed value for demonstration).
	constexpr auto tau_ref = 0.1;
	constexpr auto wavelength_ref = 550_nm;

	// Integration step (in nm); here we assume 10 nm intervals:
	// TODO actually check the interval from iterated table
	auto const wavelength_delta = 10_nm;
	// Variables to accumulate the XYZ tristimulus values:
	float x = 0.0, y = 0.0, z = 0.0;

	// Loop over wavelengths:
	for (auto const& wc: kCie1931XYZTable)
	{
		// Wavelength-dependent optical depth (Rayleigh ~ λ⁻⁴ (wavelength⁻⁴)):
		auto const scattering = tau_ref * std::pow (wavelength_ref / wc.wavelength, 4.0);
		// Radiance of the black body:
		auto radiance = blackbody_spectral_radiance (wc.wavelength, temperature);

		if (attenuator)
			radiance = radiance * attenuator (scattering);
		else
			radiance = radiance * std::exp (-scattering);

		// Basic rectangular integration:
		auto const bar = (radiance * wavelength_delta.in<si::Meter>()).base_value(); // TODO si::convert?

		// Multiply by the color matching functions and add to the integrated x/y/z values:
		x += bar * wc.color.x();
		y += bar * wc.color.y();
		z += bar * wc.color.z();
	}

	// Normalize and compute chromaticity coordinates:
	auto const sum = x + y + z;
	auto const cie_x = x / sum;
	auto const cie_y = y / sum;

	return { cie_x, cie_y };
}


QColor
qcolor_from_temperature (si::Temperature const temperature)
{
	double t = temperature / 100.0_K; // Scale the temperature.
	double red, green, blue;

	if (t <= 66.0)
		red = 255;
	else
		red = 329.698727446 * pow (t - 60.0, -0.1332047592);

	if (t <= 66.0)
		green = 99.4708025861 * log (t) - 161.1195681661;
	else
		green = 288.1221695283 * pow (t - 60.0, -0.0755148492);

	// Calculate Blue
	if (t >= 66)
		blue = 255;
	else if (t <= 19)
		blue = 0;
	else
		blue = 138.5177312231 * log (t - 10.0) - 305.0447927307;

	red = std::clamp (red, 0.0, 255.0);
	green = std::clamp (green, 0.0, 255.0);
	blue = std::clamp (blue, 0.0, 255.0);

	return QColor::fromRgb (std::round (red), std::round (green), std::round (blue));
}

} // namespace xf


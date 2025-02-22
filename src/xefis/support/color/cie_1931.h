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

#ifndef XEFIS__SUPPORT__COLOR__CIE_1931_H__INCLUDED
#define XEFIS__SUPPORT__COLOR__CIE_1931_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/si/si.h>

// Qt:
#include <QColor>

// Standard:
#include <cstddef>


namespace xf {

using CieXYZColor = math::Vector<float, 3>;


struct CieXYZWavelengthColor
{
	si::Length	wavelength;
	CieXYZColor	color;
};


static constexpr auto const kCie1931XYZTable = std::array<CieXYZWavelengthColor, 41> {{
	{ 380_nm, { 0.0014f, 0.0000f, 0.0065f } },
	{ 390_nm, { 0.0042f, 0.0001f, 0.0201f } },
	{ 400_nm, { 0.0143f, 0.0004f, 0.0679f } },
	{ 410_nm, { 0.0435f, 0.0012f, 0.2074f } },
	{ 420_nm, { 0.1344f, 0.0040f, 0.6456f } },
	{ 430_nm, { 0.2839f, 0.0116f, 1.3856f } },
	{ 440_nm, { 0.3483f, 0.0230f, 1.7471f } },
	{ 450_nm, { 0.3362f, 0.0380f, 1.7721f } },
	{ 460_nm, { 0.2908f, 0.0600f, 1.6692f } },
	{ 470_nm, { 0.1954f, 0.0910f, 1.2876f } },
	{ 480_nm, { 0.0956f, 0.1390f, 0.8130f } },
	{ 490_nm, { 0.0320f, 0.2080f, 0.4652f } },
	{ 500_nm, { 0.0049f, 0.3230f, 0.2720f } },
	{ 510_nm, { 0.0093f, 0.5030f, 0.1582f } },
	{ 520_nm, { 0.0633f, 0.7100f, 0.0782f } },
	{ 530_nm, { 0.1655f, 0.8620f, 0.0422f } },
	{ 540_nm, { 0.2904f, 0.9540f, 0.0203f } },
	{ 550_nm, { 0.4334f, 0.9949f, 0.0088f } },
	{ 560_nm, { 0.5945f, 0.9950f, 0.0039f } },
	{ 570_nm, { 0.7621f, 0.9520f, 0.0021f } },
	{ 580_nm, { 0.9163f, 0.8700f, 0.0017f } },
	{ 590_nm, { 1.0263f, 0.7570f, 0.0011f } },
	{ 600_nm, { 1.0622f, 0.6310f, 0.0008f } },
	{ 610_nm, { 1.0026f, 0.5030f, 0.0003f } },
	{ 620_nm, { 0.8544f, 0.3810f, 0.0002f } },
	{ 630_nm, { 0.6424f, 0.2650f, 0.0000f } },
	{ 640_nm, { 0.4479f, 0.1750f, 0.0000f } },
	{ 650_nm, { 0.2835f, 0.1070f, 0.0000f } },
	{ 660_nm, { 0.1649f, 0.0610f, 0.0000f } },
	{ 670_nm, { 0.0874f, 0.0320f, 0.0000f } },
	{ 680_nm, { 0.0468f, 0.0170f, 0.0000f } },
	{ 690_nm, { 0.0227f, 0.0082f, 0.0000f } },
	{ 700_nm, { 0.0114f, 0.0041f, 0.0000f } },
	{ 710_nm, { 0.0058f, 0.0021f, 0.0000f } },
	{ 720_nm, { 0.0029f, 0.0010f, 0.0000f } },
	{ 730_nm, { 0.0014f, 0.0005f, 0.0000f } },
	{ 740_nm, { 0.0007f, 0.0002f, 0.0000f } },
	{ 750_nm, { 0.0003f, 0.0001f, 0.0000f } },
	{ 760_nm, { 0.0002f, 0.0001f, 0.0000f } },
	{ 770_nm, { 0.0001f, 0.0000f, 0.0000f } },
	{ 780_nm, { 0.0000f, 0.0000f, 0.0000f } },
}};


/**
 * Convert CIE 1931 X, Y parameters to approximate the correlated color temperature (CCT).
 */
[[nodiscard]]
constexpr si::Temperature
cie_xy_to_cct (float cie_x, float cie_y)
{
	// McCamy's empirical formula for approximation of CCT:
	auto n_val = (cie_x - 0.3320) / (cie_y - 0.1858);
	return -449.0_K * std::pow (n_val, 3) + 3525.0_K * std::pow (n_val, 2) - 6823.3_K * n_val + 5520.33_K;
}


/**
 * Convenience overload.
 */
[[nodiscard]]
constexpr si::Temperature
cie_xy_to_cct (math::Vector<float, 2> const color_xy)
{
	return cie_xy_to_cct (color_xy.x(), color_xy.y());
}


/**
 * Convenience overload.
 */
[[nodiscard]]
constexpr si::Temperature
cie_xyz_to_cct (math::Vector<float, 3> const color)
{
	return cie_xy_to_cct (color.x(), color.y());
}


[[nodiscard]]
QColor
cie_xyz_to_rgb (math::Vector<double, 3> const& xyz);

} // namespace xf

#endif


/* vim:ts=4
 *
 * Copyleft 2008…2013  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__NATURE_H__INCLUDED
#define XEFIS__SUPPORT__NATURE_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>


namespace xf {

constexpr auto						kGravitationalConstant			= 6.67408313131e-11 * 1_m3 / 1_kg / 1_s / 1_s;
constexpr auto						kUniversalGasConstant			= 8.3144598_J / 1_mol / 1_K;
constexpr si::MolarMass				kAirMolarMass					= 0.0289644_kg / 1_mol;
constexpr si::SpecificHeatCapacity	kDryAirSpecificConstant			= 287.058_J / 1_kg / 1_K;

// Sound speed in STD sea level:
constexpr si::Acceleration			kStdGravitationalAcceleration	= 9.80665_mps2;

// Sound speed in STD sea level at 15°C:
constexpr si::Velocity				kStdSpeedOfSound				= 661.4788_kt;

// STD sea level pressure at 15°C:
constexpr si::Pressure				kStdAirPressure					= 1013.25_hPa;

// STD sea level air density at 15°C:
constexpr si::Density				kStdAirDensity					= 1.225_kg / 1_m3;

} // namespace xf

#endif


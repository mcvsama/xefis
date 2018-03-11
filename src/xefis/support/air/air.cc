/* vim:ts=4
 *
 * Copyleft 2012…2018  Michał Gawron
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

// Local:
#include "air.h"


namespace xf {

xf::Datatable2D<si::Temperature, si::DynamicViscosity> const&
temperature_to_dynamic_viscosity()
{
	// Map of temperature <-> dynamic viscosity taken from
	// <http://www.engineeringtoolbox.com/air-absolute-kinematic-viscosity-d_601.html>
	static std::map<si::Temperature, si::DynamicViscosity> const
	temperature_to_dynamic_viscosity_map {
		{  -40_degF, 157.591e-7_Pas },
		{  -20_degF, 159.986e-7_Pas },
		{    0_degF, 157.591e-7_Pas },
		{   10_degF, 164.776e-7_Pas },
		{   20_degF, 167.650e-7_Pas },
		{   30_degF, 171.482e-7_Pas },
		{   40_degF, 172.440e-7_Pas },
		{   50_degF, 176.272e-7_Pas },
		{   60_degF, 179.625e-7_Pas },
		{   70_degF, 182.978e-7_Pas },
		{   80_degF, 184.894e-7_Pas },
		{   90_degF, 186.810e-7_Pas },
		{  100_degF, 188.726e-7_Pas },
		{  120_degF, 192.558e-7_Pas },
		{  140_degF, 197.827e-7_Pas },
		{  160_degF, 202.138e-7_Pas },
		{  180_degF, 207.886e-7_Pas },
		{  200_degF, 215.071e-7_Pas },
		{  300_degF, 238.063e-7_Pas },
		{  400_degF, 250.996e-7_Pas },
		{  500_degF, 277.820e-7_Pas },
		{  750_degF, 326.199e-7_Pas },
		{ 1000_degF, 376.015e-7_Pas },
		{ 1500_degF, 455.050e-7_Pas },
	};

	static xf::Datatable2D<si::Temperature, si::DynamicViscosity> const
	temperature_to_dynamic_viscosity { std::move (temperature_to_dynamic_viscosity_map) };

	return temperature_to_dynamic_viscosity;
}

} // namespace xf


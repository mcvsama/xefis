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

#ifndef XEFIS__SUPPORT__AERODYNAMICS__REYNOLDS_H__INCLUDED
#define XEFIS__SUPPORT__AERODYNAMICS__REYNOLDS_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/numeric.h>
#include <neutrino/strong_type.h>

// Standard:
#include <cstddef>


namespace xf {

using Reynolds = StrongType<double, struct ReynoldsType>;


constexpr Reynolds
reynolds_number (si::Density fluid_density, si::Velocity fluid_speed, si::Length characteristic_dimension, si::DynamicViscosity mu)
{
	return Reynolds (fluid_density * fluid_speed * characteristic_dimension / mu);
}


constexpr Reynolds
reynolds_number (si::Velocity fluid_speed, si::Length characteristic_dimension, si::KinematicViscosity nu)
{
	return Reynolds (fluid_speed * characteristic_dimension / nu);
}


constexpr si::Pressure
dynamic_pressure (si::Density fluid_density, si::Velocity fluid_speed)
{
	return 0.5 * fluid_density * square (fluid_speed);
}

} // namespace xf

#endif


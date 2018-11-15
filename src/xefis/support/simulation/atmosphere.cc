/* vim:ts=4
 *
 * Copyleft 2008…2018  Michał Gawron
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

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/nature/constants.h>

// Local:
#include "atmosphere.h"


namespace xf::sim {

Air
Atmosphere::air_at (SpaceVector<si::Length, ECEFFrame> const& position) const
{
	return air_at_radius (abs (position));
}


Air
Atmosphere::air_at_radius (si::Length radius) const
{
	return air_at_amsl (radius - kEarthMeanRadius);
}


Air
Atmosphere::air_at_amsl (si::Length geometric_altitude_amsl) const
{
	Air air;
	air.density = standard_density (geometric_altitude_amsl);
	air.pressure = standard_pressure (geometric_altitude_amsl);
	air.temperature = standard_temperature (geometric_altitude_amsl);
	air.dynamic_viscosity = dynamic_air_viscosity (air.temperature);
	air.speed_of_sound = speed_of_sound (air.temperature);
	return air;
}


SpaceVector<si::Velocity, ECEFFrame>
Atmosphere::wind_at (SpaceVector<si::Length, ECEFFrame> const&) const
{
	// TODO model
	return { 0_mps, 0_mps, 0_mps };
}


AtmosphereState<ECEFFrame>
Atmosphere::state_at (SpaceVector<si::Length, ECEFFrame> const& position) const
{
	return {
		air_at (position),
		wind_at (position),
	};
}

} // namespace xf::sim


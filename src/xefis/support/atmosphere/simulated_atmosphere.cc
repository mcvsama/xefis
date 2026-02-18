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
#include "simulated_atmosphere.h"

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/math/field.h>

// Standard:
#include <cstddef>


namespace xf {

si::Density
SimulatedAtmosphere::density_at (SpaceLength<ECEFSpace> const& position) const
{
	// TODO add variations in density
	return _standard_atmosphere.density_at (position);
}


si::Pressure
SimulatedAtmosphere::pressure_at (SpaceLength<ECEFSpace> const& position) const
{
	// TODO add variations in pressure
	return _standard_atmosphere.pressure_at (position);
}


si::Temperature
SimulatedAtmosphere::temperature_at (SpaceLength<ECEFSpace> const& position) const
{
	// TODO add variations in temperature
	return _standard_atmosphere.temperature_at (position);
}


SpaceVector<si::Velocity, ECEFSpace>
SimulatedAtmosphere::wind_velocity_at (SpaceLength<ECEFSpace> const& position) const
{
	// TODO Maybe use Perlin noise for winds. http://flafla2.github.io/2014/08/09/perlinnoise.html
	// TODO Use winds to affect density/pressure at that point.
	// TODO Maybe the winds model should be a separate class plugged in into Atmosphere object?
	return _standard_atmosphere.wind_velocity_at (position);
}

} // namespace xf

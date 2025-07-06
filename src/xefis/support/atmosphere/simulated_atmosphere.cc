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

Air<ECEFSpace>
SimulatedAtmosphere::air_at (SpaceLength<ECEFSpace> const& position) const
{
	// TODO Maybe use Perlin noise for winds. http://flafla2.github.io/2014/08/09/perlinnoise.html
	// TODO Use winds to affect density/pressure at that point.
	// TODO Maybe the winds model should be a separate class plugged in into Atmosphere object?
	auto air = _standard_atmosphere.air_at (position);
	// TODO add wind
	// TODO add variations in densite
	// TODO add variations in temperature
	// TODO add variations in pressure
	// TODO update dynamic_viscosity that depends on temperature
	// TODO update speed_of_sound that depends on temperature
	// TODO or rather move calculations of dynamic_viscosity and speed_of_sound to Air class, in ctor.
	return air;
}

} // namespace xf


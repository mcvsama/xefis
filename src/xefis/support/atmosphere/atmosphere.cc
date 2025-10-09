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
#include "atmosphere.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/nature/constants.h>

// Standard:
#include <cstddef>


namespace xf {

Air<ECEFSpace>
Atmosphere::air_at (SpaceLength<ECEFSpace> const& position) const
{
	auto const temperature = temperature_at (position);

	// TODO Maybe move calculations of dynamic_viscosity and speed_of_sound to Air class, in ctor.
	return {
		.density = density_at (position),
		.pressure = pressure_at (position),
		.temperature = temperature,
		.dynamic_viscosity = dynamic_air_viscosity (temperature),
		.speed_of_sound = speed_of_sound (temperature),
		.velocity = wind_velocity_at (position),
	};
}

} // namespace xf


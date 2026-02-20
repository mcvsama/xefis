/* vim:ts=4
 *
 * Copyleft 2026  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__UNIVERSE__MOON_POSITION_H__INCLUDED
#define XEFIS__SUPPORT__UNIVERSE__MOON_POSITION_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/coordinate_systems.h>
#include <xefis/support/math/geometry_types.h>
#include <xefis/support/universe/coordinate_systems.h>

// Standard:
#include <cstddef>


namespace xf {

[[nodiscard]]
EclipticCoordinates
compute_moon_ecliptic_position (si::Time const unix_time);

[[nodiscard]]
SpaceLength<WorldSpace>
compute_moon_position_in_ecef (si::Time const unix_time);

} // namespace xf

#endif

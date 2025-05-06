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

#ifndef XEFIS__SUPPORT__UNIVERSE__COORDINATE_SYSTEMS_H__INCLUDED
#define XEFIS__SUPPORT__UNIVERSE__COORDINATE_SYSTEMS_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>


namespace xf {

struct EclipticCoordinates
{
	// Ecliptic longitude of the Sun (also known as lambda):
	si::Angle	longitude;

	// Ecliptic latitude of the Sun (always nearly 0°) (AKA beta):
	si::Angle	latitude;

	// Distance of the Sun from the Earth (in astronomical units) (also known as R):
	double		distance_from_earth;
};


struct EquatorialCoordinates
{
	// The angular distance measured eastward along the celestial equator from the vernal equinox:
	si::Angle	right_ascension;

	// The angular distance north (positive) or south (negative) of the celestial equator
	// (represents the Sun's position relative to Earth's equator):
	si::Angle	declination;
};


struct HorizontalCoordinates
{
	// The angle between the object (e.g., the Sun) and the observer's local horizon.
	// Positive values indicate the object is above the horizon.
	si::Angle	altitude;

	// The direction of the object along the horizon, typically measured clockwise from true north:
	si::Angle	azimuth;
};

} // namespace xf

#endif


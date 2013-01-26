/* vim:ts=4
 *
 * Copyleft 2012…2013  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__UTILITY__NAVIGATION_H__INCLUDED
#define XEFIS__UTILITY__NAVIGATION_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/numeric.h>// XXX needed for deg_to_rad
#include <xefis/utility/lonlat.h>


/**
 * Compute angle between two great arcs on a sphere.
 * Arcs are given by three points, the second one lies on the intersection.
 * Result is in degrees.
 */
inline Degrees
great_arcs_angle (LonLat const& a, LonLat const& common, LonLat const& b)
{
	LonLat z1 (a.lon() - common.lon(), a.lat() - common.lat());
	LonLat zero (0.0, 0.0);
	LonLat z2 (b.lon() - common.lon(), b.lat() - common.lat());

	std::complex<LonLat::ValueType> x1 (z1.lon(), z1.lat());
	std::complex<LonLat::ValueType> x2 (z2.lon(), z2.lat());

	return floored_mod (rad_to_deg (std::arg (x1) - std::arg (x2)), 360.0);
}

#endif


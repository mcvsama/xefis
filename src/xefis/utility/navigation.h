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


/**
 * Compute angle between two great arcs on a sphere.
 * Arcs are given by three points, the second one lies on the intersection.
 * Result is in degrees.
 */
inline Angle
great_arcs_angle (LonLat const& a, LonLat const& common, LonLat const& b)
{
	LonLat z1 (a.lon() - common.lon(), a.lat() - common.lat());
	LonLat zero (0_deg, 0_deg);
	LonLat z2 (b.lon() - common.lon(), b.lat() - common.lat());

	std::complex<LonLat::ValueType::ValueType> x1 (z1.lon().deg(), z1.lat().deg());
	std::complex<LonLat::ValueType::ValueType> x2 (z2.lon().deg(), z2.lat().deg());

	return 1_deg * floored_mod<double> ((1_rad * (std::arg (x1) - std::arg (x2))).deg(), 360.0);
}

#endif


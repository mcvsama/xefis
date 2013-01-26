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
 * --
 * Here be basic, global functions and macros like asserts, debugging helpers, etc.
 */

#ifndef XEFIS__CONFIG__CONSTANTS_H__INCLUDED
#define XEFIS__CONFIG__CONSTANTS_H__INCLUDED

// Standards:
#include <cmath>

// Local:
#include "types.h"


constexpr float EARTH_MEAN_RADIUS_KM	= 6367.46f;
constexpr float EARTH_MEAN_RADIUS_NM	= 3438.15f;
constexpr float STD_PRESSURE			= 29.92f;

inline bool
is_std_pressure (InHg pressure)
{
	return std::abs (STD_PRESSURE - pressure) < 0.01f;
}

#endif


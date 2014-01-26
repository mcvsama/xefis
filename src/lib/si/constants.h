/* vim:ts=4
 *
 * Copyleft 2012…2014  Michał Gawron
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

#ifndef SI__CONSTANTS_H__INCLUDED
#define SI__CONSTANTS_H__INCLUDED

// Local:
#include "length.h"


namespace SI {

constexpr Length EARTH_MEAN_RADIUS	= 6367.46_km;

} // namespace SI

#endif


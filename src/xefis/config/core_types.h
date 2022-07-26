/* vim:ts=4
 *
 * Copyleft 2012…2016  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__CONFIG__CORE_TYPES_H__INCLUDED
#define XEFIS__CONFIG__CORE_TYPES_H__INCLUDED

// Lib:
#include <lib/half/half.hpp>
#include <type_safe/integer.hpp>

// Neutrino:
#include <neutrino/core_types.h>

// Standard:
#include <cstddef>


namespace ts = type_safe;


inline half_float::half
operator "" _half (long double value)
{
	return half_float::half (static_cast<float> (value));
}


using float16_t = half_float::half;

#endif


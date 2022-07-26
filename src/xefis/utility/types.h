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

#ifndef XEFIS__UTILITY__TYPES_H__INCLUDED
#define XEFIS__UTILITY__TYPES_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>
#include <type_traits>


// Add some Xefis-specific stuff to the neutrino namespace.
namespace neutrino {

/**
 * Gives corresponding floating-point type (half, float, double, long double…) for required byte-width
 * (number of bytes in the type).
 */
template<size_t Width>
	struct float_for_width;


template<>
	struct float_for_width<2>
	{
		using type = float16_t;
	};

} // namespace neutrino

#endif


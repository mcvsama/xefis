/* vim:ts=4
 *
 * Copyleft 2008…2012  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__UTILITY__NUMERIC_H__INCLUDED
#define XEFIS__UTILITY__NUMERIC_H__INCLUDED

// Standard:
#include <cstddef>
#include <cmath>

// Xefisi:
#include <xefis/config/all.h>


/**
 * Return x > 0.0 ? 1.0 : -1.0.
 */
inline float
sgn (float const& v) noexcept
{
	union {
		float f;
		uint32_t i;
	} u = {v};
	const uint32_t s = u.i & (1 << 31);
	u.f = 1.0f;
	u.i |= s;
	return u.f;
}


/**
 * Works properly only on floats.
 * \param	n - dividend
 * \param	d - divisor
 */
template<class Number>
	Number
	floored_mod (Number n, Number d)
	{
		return n - (d * std::floor (n / d));
	}


template<class Value>
	inline constexpr Value
	bound (Value value, Value min, Value max) noexcept
	{
		return value < min
			? min
			: value > max
				? max
				: value;
	}

#endif


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

#ifndef XEFIS__UTILITY__NUMERIC_H__INCLUDED
#define XEFIS__UTILITY__NUMERIC_H__INCLUDED

// Standard:
#include <cstddef>
#include <complex>
#include <cmath>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/range.h>
#include <xefis/utility/lonlat.h>


inline constexpr float
renormalize (float value, float a1, float b1, float a2, float b2) noexcept
{
	return b1 == a1
		? a2
		: (b2 - a2) / (b1 - a1) * value + (-(b2 - a2) / (b1 - a1) * a1 + a2);
}


inline constexpr float
renormalize (float value, Range<float> range1, Range<float> range2) noexcept
{
	return renormalize (value, range1.min(), range1.max(), range2.min(), range2.max());
}


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


template<class Number>
	Number
	floored_mod (Number n, Number min, Number max)
	{
		return floored_mod (n - min, max - min) + min;
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


template<class Value>
	inline constexpr Value
	bound (Value value, Range<Value> range) noexcept
	{
		return range.min() <= range.max()
			? bound (value, range.min(), range.max())
			: bound (value, range.max(), range.min());
	}


/**
 * Convert radians to degrees.
 */
template<class Value>
	inline constexpr Value
	rad_to_deg (Value radians)
	{
		return radians * (180.0 / M_PI);
	}


/**
 * Convert degrees to radians.
 */
template<class Value>
	inline constexpr Value
	deg_to_rad (Value degrees)
	{
		return degrees * (M_PI / 180.0);
	}


/**
 * Convert feet to nautical miles.
 */
template<class Value>
	inline constexpr Value
	ft_to_nm (Value value)
	{
		return value / 6076.11549;
	}

#endif


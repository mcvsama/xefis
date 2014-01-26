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


namespace Xefis {

inline constexpr double
renormalize (double value, double a1, double b1, double a2, double b2) noexcept
{
	return b1 == a1
		? a2
		: (b2 - a2) / (b1 - a1) * value + (-(b2 - a2) / (b1 - a1) * a1 + a2);
}


inline constexpr double
renormalize (double value, Range<double> range1, Range<double> range2) noexcept
{
	return renormalize (value, range1.min(), range1.max(), range2.min(), range2.max());
}


template<class T>
	inline constexpr int
	sgn (T x, std::false_type) noexcept
	{
		return T (0) < x;
	}


template<class T>
	inline constexpr int
	sgn (T x, std::true_type) noexcept
	{
		return (T (0) < x) - (x < T (0));
	}


/**
 * Return signum (x) (-1, 0 or 1).
 */
template<class T>
	inline constexpr int
	sgn (T x) noexcept
	{
		return sgn (x, std::is_signed<T>());
	}


template<class T = int, class S>
	inline constexpr T
	symmetric_round (S s) noexcept
	{
		return static_cast<T> (sgn (s) * 0.5 + s);
	}


/**
 * Works properly only on floats.
 * \param	n - dividend
 * \param	d - divisor
 */
template<class Number>
	inline constexpr Number
	floored_mod (Number n, Number d)
	{
		return n - (d * std::floor (n / d));
	}


template<>
	inline constexpr int
	floored_mod<int> (int n, int d)
	{
		return (n % d) >= 0 ? (n % d) : (n % d) + std::abs (d);
	}


template<class Number>
	inline constexpr Number
	floored_mod (Number n, Number min, Number max)
	{
		return floored_mod (n - min, max - min) + min;
	}


template<class Value>
	inline constexpr Value
	limit (Value value, Value min, Value max) noexcept
	{
		return value < min
			? min
			: value > max
				? max
				: value;
	}


template<class Value>
	inline constexpr Value
	limit (Value value, Range<Value> range) noexcept
	{
		return range.min() <= range.max()
			? limit (value, range.min(), range.max())
			: limit (value, range.max(), range.min());
	}


template<class Value>
	inline constexpr Value
	magnetic_to_true (Value mag, Value declination)
	{
		return floored_mod (mag + declination, 360.0);
	}


inline constexpr Angle
magnetic_to_true (Angle mag, Angle declination)
{
	return floored_mod (mag + declination, 360.0_deg);
}


template<class Value>
	inline constexpr Value
	true_to_magnetic (Value tru, Value declination)
	{
		return floored_mod (tru - declination, 360.0);
	}


inline constexpr Angle
true_to_magnetic (Angle tru, Angle declination)
{
	return floored_mod (tru - declination, 360.0_deg);
}

} // namespace Xefis

#endif


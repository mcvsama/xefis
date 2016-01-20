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

#ifndef XEFIS__UTILITY__NUMERIC_H__INCLUDED
#define XEFIS__UTILITY__NUMERIC_H__INCLUDED

// Standard:
#include <cstddef>
#include <type_traits>
#include <complex>
#include <cmath>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/config/types.h>
#include <xefis/core/stdexcept.h>
#include <xefis/utility/range.h>


namespace Xefis {

template<class A, class B,
		 class = typename std::enable_if<
			 (std::is_floating_point<A>::value || si::is_quantity<A>::value) &&
			 (std::is_floating_point<B>::value || si::is_quantity<B>::value)
		 >::type>
	inline constexpr B
	renormalize (A a, A a_min, A a_max, B b_min, B b_max)
	{
		return a_min == a_max
			? b_min
			: ((a - a_min) / (a_max - a_min)) * (b_max - b_min) + b_min;
	}


template<class A, class B>
	inline constexpr B
	renormalize (A value, Range<A> range1, Range<B> range2) noexcept
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
 * For floats.
 * \param	n - dividend
 * \param	d - divisor
 */
template<class Number>
	inline constexpr Number
	floored_mod (Number n, typename std::enable_if<std::is_floating_point<Number>::value || si::is_quantity<Number>::value, Number>::type d)
	{
		return n - (d * std::floor (n / d));
	}


/**
 * For integral types.
 * \param	n - dividend
 * \param	d - divisor
 */
template<class Number>
	inline constexpr Number
	floored_mod (Number n, typename std::enable_if<std::is_integral<Number>::value, Number>::type d)
	{
		return (n % d) >= 0 ? (n % d) : (n % d) + std::abs (d);
	}


/**
 * For SI types.
 * \param	n - dividend
 * \param	d - divisor
 */
template<template<class, class> class Quantity, class Unit, class Value>
	inline constexpr Quantity<Unit, Value>
	floored_mod (Quantity<Unit, Value> n, Quantity<Unit, Value> d)
	{
		return n - (d * std::floor (n / d));
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
	return floored_mod (mag + declination, 360_deg);
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
	return floored_mod (tru - declination, 360_deg);
}


inline int
digit_from_ascii (char c)
{
	if ('0' <= c && c <= '9')
		return c - '0';
	throw InvalidFormat ("non-numeric character '"_str + c + "'");
}

} // namespace Xefis

#endif


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

#ifndef NEUTRINO__NUMERIC_H__INCLUDED
#define NEUTRINO__NUMERIC_H__INCLUDED

// Standard:
#include <algorithm>
#include <complex>
#include <cmath>
#include <cstddef>
#include <numeric>
#include <type_traits>

// Neutrino:
#include <neutrino/c++20.h>
#include <neutrino/math/math.h>
#include <neutrino/types.h>
#include <neutrino/stdexcept.h>
#include <neutrino/range.h>


namespace neutrino {

template<class V, class B, class A = V,
		 class = typename std::enable_if_t<
			 (std::is_floating_point_v<A> || si::is_quantity_v<A>) &&
			 (std::is_floating_point_v<B> || si::is_quantity_v<B>)
		 >>
	[[nodiscard]]
	constexpr B
	renormalize (V v, A a_min, A a_max, B b_min, B b_max)
	{
		return a_min == a_max
			? b_min
			: ((v - a_min) / (a_max - a_min)) * (b_max - b_min) + b_min;
	}


template<class V, class B, class A = V>
	[[nodiscard]]
	constexpr B
	renormalize (V value, Range<A> range1, Range<B> range2) noexcept
	{
		return renormalize (value, range1.min(), range1.max(), range2.min(), range2.max());
	}


template<class Value, std::size_t Size>
	[[nodiscard]]
	constexpr auto
	renormalize (double v, double a_min, double a_max, math::Vector<Value, Size> const& b_min, math::Vector<Value, Size> const& b_max)
	{
		math::Vector<Value, Size> result { math::zero };

		for (std::size_t i = 0; i < result.kRows; ++i)
			result[i] = renormalize (v, a_min, a_max, b_min[i], b_max[i]);

		return result;
	}


template<class Value, std::size_t Size>
	[[nodiscard]]
	constexpr auto
	renormalize (Value v, Range<Value> const& range1, Range<math::Vector<Value, Size>> const& range2) noexcept
	{
		return renormalize (v, range1.min(), range1.max(), range2.min(), range2.max());
	}


template<class T>
	[[nodiscard]]
	constexpr int
	sgn (T x, std::false_type) noexcept
	{
		return T (0) < x;
	}


template<class T>
	[[nodiscard]]
	constexpr int
	sgn (T x, std::true_type) noexcept
	{
		return (T (0) < x) - (x < T (0));
	}


/**
 * Return signum (x) (-1, 0 or 1).
 */
template<class T>
	[[nodiscard]]
	constexpr int
	sgn (T x) noexcept
	{
		return sgn (x, std::is_signed<T>());
	}


template<class T = int>
	[[nodiscard]]
	constexpr T
	symmetric_round (auto s) noexcept
	{
		return static_cast<T> (sgn (s) * 0.5 + s);
	}


/**
 * For floats.
 * \param	n - dividend
 * \param	d - divisor
 */
template<class Number>
	[[nodiscard]]
	constexpr Number
	floored_mod (Number n, std::enable_if_t<std::is_floating_point_v<Number> || si::is_quantity_v<Number>, Number> d)
	{
		return n - (d * std::floor (n / d));
	}


/**
 * For integral types.
 * \param	n - dividend
 * \param	d - divisor
 */
template<class Number>
	[[nodiscard]]
	constexpr Number
	floored_mod (Number n, std::enable_if_t<std::is_integral_v<Number>, Number> d)
	{
		return (n % d) >= 0 ? (n % d) : (n % d) + std::abs (d);
	}


/**
 * For SI types.
 * \param	n - dividend
 * \param	d - divisor
 */
template<template<class, class> class Quantity, class Unit, class Value>
	[[nodiscard]]
	constexpr Quantity<Unit, Value>
	floored_mod (Quantity<Unit, Value> n, Quantity<Unit, Value> d)
	{
		return n - (d * std::floor (n / d));
	}


template<class Number>
	[[nodiscard]]
	constexpr Number
	floored_mod (Number n, Number min, Number max)
	{
		return floored_mod (n - min, max - min) + min;
	}


template<class Number>
	[[nodiscard]]
	constexpr Number
	floored_mod (Number n, Range<Number> range)
	{
		return floored_mod (n - range.min(), range.extent()) + range.min();
	}


template<class Value>
	constexpr void
	clamp (Value& value, Value min, Value max) noexcept
	{
		if (value < min)
			value = min;
		else if (value > max)
			value = max;
	}


template<class Value>
	constexpr void
	clamp (Value& value, Range<Value> range) noexcept
	{
		return range.min() <= range.max()
			? clamp (value, range.min(), range.max())
			: clamp (value, range.max(), range.min());
	}


template<class Value>
	[[nodiscard]]
	constexpr Value
	clamped (Value value, Value min, Value max) noexcept
	{
		return value < min
			? min
			: value > max
				? max
				: value;
	}


template<class Value>
	[[nodiscard]]
	constexpr Value
	clamped (Value value, Range<Value> range) noexcept
	{
		return range.min() <= range.max()
			? clamped (value, range.min(), range.max())
			: clamped (value, range.max(), range.min());
	}


template<class V>
	[[nodiscard]]
	constexpr V
	quantized (V value, std::size_t steps, Range<V> range)
	{
		Range<V> const steps_range { 0, steps };
		auto const r = std::round (renormalize (value, range, steps_range));
		auto const c = neutrino::clamped (r, steps_range);
		return renormalize (c, steps_range, range);
	}


template<class V>
	[[nodiscard]]
	constexpr V
	quantized (V value, V resolution)
	{
		return resolution * std::round (value / resolution);
	}


template<class Value>
	[[nodiscard]]
	constexpr Value
	magnetic_to_true (Value mag, Value declination)
	{
		return floored_mod (mag + declination, 360.0);
	}


[[nodiscard]]
constexpr Angle
magnetic_to_true (Angle mag, Angle declination)
{
	return floored_mod (mag + declination, 360_deg);
}


template<class Value>
	[[nodiscard]]
	constexpr Value
	true_to_magnetic (Value tru, Value declination)
	{
		return floored_mod (tru - declination, 360.0);
	}


[[nodiscard]]
constexpr Angle
true_to_magnetic (Angle tru, Angle declination)
{
	return floored_mod (tru - declination, 360_deg);
}


[[nodiscard]]
inline int
digit_from_ascii (char c)
{
	using namespace std::literals;

	if ('0' <= c && c <= '9')
		return c - '0';

	throw InvalidFormat ("non-numeric character '"s + c + "'");
}


/**
 * Calculate numeric integral over a range with step equal to delta.
 * Uses trapezoidal approximation.
 */
template<class Argument, class Callable>
	[[nodiscard]]
	constexpr auto
	integral (Callable function, Range<Argument> range, Argument delta)
	{
		using Value = std::result_of_t<Callable (Argument)>;

		auto sum = Argument() * Value();
		auto value_a = function (range.min());
		auto a = range.min();

		while (a < range.max() - delta)
		{
			auto b = a + delta;
			auto value_b = function (b);

			sum += (value_a + value_b) * delta * 0.5;
			value_a = value_b;
			a = b;
		}

		sum += (value_a + function (range.max())) * delta * 0.5;

		return sum;
	}


template<class T>
	[[nodiscard]]
	constexpr T
	static_pow (T value, uint64_t power)
	{
		T result = value;

		for (uint64_t i = 0; i < power - 1; ++i)
			result *= value;

		return result;
	}


template<class Iterator>
	[[nodiscard]]
	inline auto
	mean (Iterator begin, Iterator end)
	{
		if (begin == end)
			throw std::length_error ("can't compute mean() of zero-length sequence");

		using Value = std::remove_cvref_t<decltype (*std::declval<Iterator>())>;

		return std::accumulate (begin, end, Value{}) / std::distance (begin, end);
	}


/**
 * Compute median.
 * Allocates memory, but doesn't modify the sequence.
 */
template<class Iterator>
	[[nodiscard]]
	inline auto
	median (Iterator begin, Iterator end)
	{
		if (begin == end)
			throw std::length_error ("can't compute median() of zero-length sequence");

		using Value = std::remove_cvref_t<decltype (*std::declval<Iterator>())>;

		std::vector<Value> data (begin, end);
		auto const mid = data.size() / 2;
		std::nth_element (data.begin(), data.begin() + mid, data.end());

		if (data.size() % 2 == 0)
			return 0.5 * (data[mid - 1] + data[mid]);
		else
			return data[mid];
	}


/**
 * Compute median allowing range to get partially sorted during operation.
 */
template<class Iterator>
	[[nodiscard]]
	inline auto
	sort_and_median (Iterator begin, Iterator end)
	{
		if (begin == end)
			throw std::length_error ("can't compute sort_and_median() of zero-length sequence");

		auto const size = std::distance (begin, end);
		auto const mid = size / 2;
		std::nth_element (begin, begin + mid, end);

		if (size % 2 == 0)
			return 0.5 * (*std::next (begin, mid - 1) + *std::next (begin, mid));
		else
			return *std::next (begin, mid);
	}


template<class Iterator>
	[[nodiscard]]
	inline auto
	stddev (Iterator begin, Iterator end)
	{
		if (begin == end)
			throw std::length_error ("can't compute stddev() of zero-length sequence");

		using std::sqrt;
		using Value = std::remove_cvref_t<decltype (*std::declval<Iterator>())>;

		std::size_t count = 0;
		auto m = mean (begin, end);
		auto sum = std::accumulate (begin, end, Value{} * Value{}, [&](auto accumulated, auto value) {
			auto const diff = value - m;
			++count;
			return accumulated + diff * diff;
		});

		return sqrt (sum / (count - 1));
	}


/**
 * Round value so that result can be used as a maximum value on a chart or histogram.
 * Returns pair of Value and a number of helper lines for chart grid.
 */
template<class Value>
	[[nodiscard]]
	inline std::pair<Value, std::size_t>
	get_max_for_axis (Value const& value)
	{
		Value fac (1);

		while (Value (1e-6) <= fac && fac <= Value (1e+6))
		{
			if (value < 0.48 * fac)
				fac /= 10.0;
			else if (value < 0.8 * fac)
				return { 1.0 * fac, 10 };
			else if (value < 1.8 * fac)
				return { 2.0 * fac, 2 };
			else if (value < 2.8 * fac)
				return { 3.0 * fac, 3 };
			else if (value < 4.8 * fac)
				return { 5.0 * fac, 5 };
			else
				fac *= 10.0;
		};

		return { fac, 10 };
	}


/**
 * Return square of the argument.
 */
template<class Value>
	[[nodiscard]]
	constexpr decltype(std::declval<Value>() * std::declval<Value>())
	square (Value a)
	{
		return a * a;
	}


/**
 * Return cube of the argument.
 */
template<class Value>
	[[nodiscard]]
	constexpr decltype(std::declval<Value>() * std::declval<Value>() * std::declval<Value>())
	cube (Value a)
	{
		return a * a * a;
	}

} // namespace neutrino

#endif


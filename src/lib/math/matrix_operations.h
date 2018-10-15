/* vim:ts=4
 *
 * Copyleft 2012…2017  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef MATH__MATRIX_OPERATIONS_H__INCLUDED
#define MATH__MATRIX_OPERATIONS_H__INCLUDED

// Standard:
#include <algorithm>
#include <utility>

// Xefis:
#include <xefis/config/c++20.h>
#include <xefis/utility/types.h>

// Local:
#include "traits.h"


namespace math {

template<class Test, template<class, std::size_t, std::size_t> class Ref>
	struct is_matrix: public std::false_type
	{ };


template<template<class, std::size_t, std::size_t> class Ref, class Scalar, std::size_t Columns, std::size_t Rows>
	struct is_matrix<Ref<Scalar, Columns, Rows>, Ref>: public std::true_type
	{ };


template<class ScalarA, class ScalarB, std::size_t ARows, std::size_t Common, std::size_t BColumns>
	constexpr auto
	operator* (Matrix<ScalarA, Common, ARows> const& a,
			   Matrix<ScalarB, BColumns, Common> const& b)
	{
		using ResultScalar = decltype (ScalarA{} * ScalarB{});

		Matrix<ResultScalar, BColumns, ARows> result { uninitialized };

		for (std::size_t c = 0; c < BColumns; ++c)
		{
			for (std::size_t r = 0; r < ARows; ++r)
			{
				ResultScalar scalar{};

				for (std::size_t i = 0; i < Common; ++i)
					scalar += a (i, r) * b (c, i);

				result (c, r) = scalar;
			}
		}

		return result;
	}


template<class ScalarA, class ScalarB, std::size_t Columns, std::size_t Rows>
	constexpr auto
	operator* (Matrix<ScalarA, Columns, Rows> matrix,
			   ScalarB const& scalar)
	{
		Matrix<decltype (ScalarA{} * ScalarB{}), Columns, Rows> result;

		for (std::size_t i = 0; i < Columns * Rows; ++i)
			result.data()[i] = matrix.data()[i] * scalar;

		return result;
	}


template<class ScalarA, class ScalarB, std::size_t Columns, std::size_t Rows>
	constexpr auto
	operator* (ScalarA const& scalar,
			   Matrix<ScalarB, Columns, Rows> const& matrix)
	{
		return matrix * scalar;
	}


template<class ScalarA, class ScalarB, std::size_t Columns, std::size_t Rows>
	constexpr auto
	operator/ (Matrix<ScalarA, Columns, Rows> matrix,
			   ScalarB const& scalar)
	{
		Matrix<decltype (ScalarA{} / ScalarB{}), Columns, Rows> result;

		for (std::size_t i = 0; i < Columns * Rows; ++i)
			result.data()[i] = matrix.data()[i] / scalar;

		return result;
	}


template<class Scalar, std::size_t Columns, std::size_t Rows>
	constexpr Matrix<Scalar, Columns, Rows>
	operator+ (Matrix<Scalar, Columns, Rows> m) noexcept
	{
		return m;
	}


template<class Scalar, std::size_t Columns, std::size_t Rows>
	constexpr auto
	operator+ (Matrix<Scalar, Columns, Rows> a,
			   Matrix<Scalar, Columns, Rows> const& b) noexcept (noexcept (Scalar{} + Scalar{}))
	{
		return a += b;
	}


template<class Scalar, std::size_t Columns, std::size_t Rows>
	constexpr auto
	operator- (Matrix<Scalar, Columns, Rows> m) noexcept (noexcept (-Scalar{}))
	{
		for (std::size_t c = 0; c < Columns; ++c)
			for (std::size_t r = 0; r < Rows; ++r)
				m (c, r) = -m (c, r);

		return m;
	}


template<class Scalar, std::size_t Columns, std::size_t Rows>
	constexpr auto
	operator- (Matrix<Scalar, Columns, Rows> a,
			   Matrix<Scalar, Columns, Rows> const& b) noexcept (noexcept (Scalar{} - Scalar{}))
	{
		return a -= b;
	}


/**
 * Return cross product of two vectors.
 */
template<class ScalarA, class ScalarB, std::size_t Size>
	constexpr Vector<decltype (ScalarA{} * ScalarB{}), Size>
	cross_product (Vector<ScalarA, Size> const& a,
				   Vector<ScalarB, Size> const& b) noexcept (noexcept (ScalarA{} * ScalarB{} - ScalarA{} * ScalarB{}))
	{
		return {
			a[1] * b[2] - a[2] * b[1],
			a[2] * b[0] - a[0] * b[2],
			a[0] * b[1] - a[1] * b[0],
		};
	}



template<class Scalar, std::size_t Size>
	constexpr auto
	abs (Vector<Scalar, Size> const& v)
	{
		using std::sqrt;

		decltype (Scalar() * Scalar()) sum_of_squares (0);

		for (std::size_t i = 0; i < Size; ++i)
			sum_of_squares += v[i] * v[i];

		return sqrt (sum_of_squares);
	}


/**
 * Traits for Matrix<>
 */
template<class Scalar, std::size_t Columns, std::size_t Rows>
	struct Traits<Matrix<Scalar, Columns, Rows>>
	{
		typedef Matrix<Scalar, Columns, Rows> Value;

		static Value
		zero();

		static Value
		unit();

		static Value
		inversed (Value const&);
	};


template<class Scalar, std::size_t Columns, std::size_t Rows>
	inline auto
	Traits<Matrix<Scalar, Columns, Rows>>::zero() -> Value
	{
		return math::zero;
	}


template<class Scalar, std::size_t Columns, std::size_t Rows>
	inline auto
	Traits<Matrix<Scalar, Columns, Rows>>::unit() -> Value
	{
		return math::unit;
	}


template<class Scalar, std::size_t Columns, std::size_t Rows>
	inline auto
	Traits<Matrix<Scalar, Columns, Rows>>::inversed (Value const& v) -> Value
	{
		return v.inversed();
	}

} // namespace math

#endif


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

#ifndef NEUTRINO__MATH__MATRIX_OPERATIONS_H__INCLUDED
#define NEUTRINO__MATH__MATRIX_OPERATIONS_H__INCLUDED

// Standard:
#include <algorithm>
#include <utility>
#include <type_traits>

// Neutrino:
#include <neutrino/c++20.h>
#include <neutrino/types.h>

// Local:
#include "traits.h"


namespace neutrino::math {

template<class Test,
		 template<class, std::size_t, std::size_t, class, class> class Ref>
	struct is_matrix: public std::false_type
	{ };


template<template<class, std::size_t, std::size_t, class, class> class Ref,
		 class Scalar, std::size_t Columns, std::size_t Rows, class TargetFrame, class SourceFrame>
	struct is_matrix<Ref<Scalar, Columns, Rows, TargetFrame, SourceFrame>, Ref>: public std::true_type
	{ };


template<class ScalarA, class ScalarB, std::size_t ARows, std::size_t Common, std::size_t BColumns, class TargetFrame, class IntermediateFrame, class SourceFrame>
	[[nodiscard]]
	constexpr auto
	operator* (Matrix<ScalarA, Common, ARows, TargetFrame, IntermediateFrame> const& a,
			   Matrix<ScalarB, BColumns, Common, IntermediateFrame, SourceFrame> const& b)
	{
		using ResultScalar = decltype (ScalarA{} * ScalarB{});

		Matrix<ResultScalar, BColumns, ARows, TargetFrame, SourceFrame> result { uninitialized };

		for (std::size_t r = 0; r < ARows; ++r)
		{
			for (std::size_t c = 0; c < BColumns; ++c)
			{
				ResultScalar scalar{};

				for (std::size_t i = 0; i < Common; ++i)
					scalar += a (i, r) * b (c, i);

				result (c, r) = scalar;
			}
		}

		return result;
	}


template<class ScalarA, class ScalarB, std::size_t Columns, std::size_t Rows, class MatrixTargetFrame, class MatrixSourceFrame, class ScalarTargetFrame, class ScalarSourceFrame>
	[[nodiscard]]
	constexpr auto
	operator* (Matrix<ScalarA, 1, 1, ScalarTargetFrame, ScalarSourceFrame> const& scalar,
			   Matrix<ScalarB, Columns, Rows, MatrixTargetFrame, MatrixSourceFrame> const& matrix)
	{
		return matrix * scalar[0];
	}


template<class ScalarA, class ScalarB, std::size_t Columns, std::size_t Rows, class ScalarTargetFrame, class ScalarSourceFrame, class MatrixTargetFrame, class MatrixSourceFrame>
	[[nodiscard]]
	constexpr auto
	operator* (Matrix<ScalarB, Columns, Rows, MatrixTargetFrame, MatrixSourceFrame> const& matrix,
			   Matrix<ScalarA, 1, 1, ScalarTargetFrame, ScalarSourceFrame> const& scalar)
	{
		return matrix * scalar[0];
	}


template<class ScalarA, class ScalarB, std::size_t Columns, std::size_t Rows, class TargetFrame, class SourceFrame>
	[[nodiscard]]
	constexpr auto
	operator* (Matrix<ScalarA, Columns, Rows, TargetFrame, SourceFrame> matrix,
			   ScalarB const& scalar)
	{
		Matrix<decltype (ScalarA{} * ScalarB{}), Columns, Rows, TargetFrame, SourceFrame> result;

		for (std::size_t i = 0; i < Columns * Rows; ++i)
			result.array().data()[i] = matrix.array().data()[i] * scalar;

		return result;
	}


template<class ScalarA, class ScalarB, std::size_t Columns, std::size_t Rows, class TargetFrame, class SourceFrame>
	[[nodiscard]]
	constexpr auto
	operator* (ScalarA const& scalar,
			   Matrix<ScalarB, Columns, Rows, TargetFrame, SourceFrame> const& matrix)
	{
		return matrix * scalar;
	}


template<class ScalarA, class ScalarB, std::size_t Columns, std::size_t Rows, class TargetFrame, class SourceFrame>
	[[nodiscard]]
	constexpr auto
	operator/ (Matrix<ScalarA, Columns, Rows, TargetFrame, SourceFrame> matrix,
			   ScalarB const& scalar)
	{
		Matrix<decltype (ScalarA{} / ScalarB{}), Columns, Rows, TargetFrame, SourceFrame> result;

		for (std::size_t i = 0; i < Columns * Rows; ++i)
			result.array().data()[i] = matrix.array().data()[i] / scalar;

		return result;
	}


template<class Scalar, std::size_t Columns, std::size_t Rows, class TargetFrame, class SourceFrame>
	[[nodiscard]]
	constexpr Matrix<Scalar, Columns, Rows, TargetFrame, SourceFrame>
	operator+ (Matrix<Scalar, Columns, Rows, TargetFrame, SourceFrame> m) noexcept
	{
		return m;
	}


template<class Scalar, std::size_t Columns, std::size_t Rows, class TargetFrame, class SourceFrame>
	[[nodiscard]]
	constexpr auto
	operator+ (Matrix<Scalar, Columns, Rows, TargetFrame, SourceFrame> a,
			   Matrix<Scalar, Columns, Rows, TargetFrame, SourceFrame> const& b) noexcept (noexcept (Scalar{} + Scalar{}))
	{
		return a += b;
	}


template<class Scalar, std::size_t Columns, std::size_t Rows, class TargetFrame, class SourceFrame>
	[[nodiscard]]
	constexpr auto
	operator- (Matrix<Scalar, Columns, Rows, TargetFrame, SourceFrame> m) noexcept (noexcept (-Scalar{}))
	{
		for (std::size_t r = 0; r < Rows; ++r)
			for (std::size_t c = 0; c < Columns; ++c)
				m (c, r) = -m (c, r);

		return m;
	}


template<class Scalar, std::size_t Columns, std::size_t Rows, class TargetFrame, class SourceFrame>
	[[nodiscard]]
	constexpr auto
	operator- (Matrix<Scalar, Columns, Rows, TargetFrame, SourceFrame> a,
			   Matrix<Scalar, Columns, Rows, TargetFrame, SourceFrame> const& b) noexcept (noexcept (Scalar{} - Scalar{}))
	{
		return a -= b;
	}


/**
 * Return cross product of two vectors.
 */
template<class ScalarA, class ScalarB, std::size_t Size, class TargetFrame, class SourceFrame>
	[[nodiscard]]
	constexpr Vector<decltype (ScalarA{} * ScalarB{}), Size, TargetFrame, SourceFrame>
	cross_product (Vector<ScalarA, Size, TargetFrame, SourceFrame> const& a,
				   Vector<ScalarB, Size, TargetFrame, SourceFrame> const& b) noexcept (noexcept (ScalarA{} * ScalarB{} - ScalarA{} * ScalarB{}))
	{
		return {
			a[1] * b[2] - a[2] * b[1],
			a[2] * b[0] - a[0] * b[2],
			a[0] * b[1] - a[1] * b[0],
		};
	}


template<class Scalar, std::size_t Size, class TargetFrame, class SourceFrame>
	[[nodiscard]]
	constexpr auto
	abs (Vector<Scalar, Size, TargetFrame, SourceFrame> const& v)
	{
		using std::sqrt;

		decltype (Scalar() * Scalar()) sum_of_squares (0);

		for (std::size_t i = 0; i < Size; ++i)
			sum_of_squares += v[i] * v[i];

		return sqrt (sum_of_squares);
	}


/**
 * Return inversed matrix.
 */
template<class Scalar, std::size_t Columns, std::size_t Rows, class TargetFrame, class SourceFrame>
	[[nodiscard]]
	constexpr auto
	inv (Matrix<Scalar, Columns, Rows, TargetFrame, SourceFrame> const& matrix)
	{
		return matrix.inversed();
	}


/**
 * Return matrix with remapped scalar.
 */
template<class MapFunction, class Scalar, std::size_t Columns, std::size_t Rows, class TargetFrame, class SourceFrame>
	[[nodiscard]]
	constexpr auto
	map (Matrix<Scalar, Columns, Rows, TargetFrame, SourceFrame> const& matrix, MapFunction&& mapper)
	{
		using MappedType = std::invoke_result_t<MapFunction, Scalar>;

		Matrix<MappedType, Columns, Rows, TargetFrame, SourceFrame> result;

		for (std::size_t r = 0; r < Rows; ++r)
			for (std::size_t c = 0; c < Columns; ++c)
				result (c, r) = mapper (matrix (c, r));

		return result;
	}


/**
 * Traits for Matrix<>
 */
template<class Scalar, std::size_t Columns, std::size_t Rows, class TargetFrame, class SourceFrame>
	struct Traits<Matrix<Scalar, Columns, Rows, TargetFrame, SourceFrame>>
	{
		typedef Matrix<Scalar, Columns, Rows, TargetFrame, SourceFrame> Value;

		static Value
		zero();

		static Value
		unit();

		static Value
		inversed (Value const&);
	};


template<class Scalar, std::size_t Columns, std::size_t Rows, class TargetFrame, class SourceFrame>
	[[nodiscard]]
	inline auto
	Traits<Matrix<Scalar, Columns, Rows, TargetFrame, SourceFrame>>::zero() -> Value
	{
		return math::zero;
	}


template<class Scalar, std::size_t Columns, std::size_t Rows, class TargetFrame, class SourceFrame>
	[[nodiscard]]
	inline auto
	Traits<Matrix<Scalar, Columns, Rows, TargetFrame, SourceFrame>>::unit() -> Value
	{
		return math::unit;
	}


template<class Scalar, std::size_t Columns, std::size_t Rows, class TargetFrame, class SourceFrame>
	[[nodiscard]]
	inline auto
	Traits<Matrix<Scalar, Columns, Rows, TargetFrame, SourceFrame>>::inversed (Value const& v) -> Value
	{
		return v.inversed();
	}

} // namespace neutrino::math

#endif


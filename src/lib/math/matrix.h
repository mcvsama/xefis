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

#ifndef MATH__MATRIX_H__INCLUDED
#define MATH__MATRIX_H__INCLUDED

// Standard:
#include <algorithm>
#include <array>
#include <initializer_list>
#include <type_traits>
#include <utility>
#include <functional>
#include <stdexcept>


namespace math {

// used to call specific matrix constructor.
class ZeroMatrixType
{ };


// used to call specific matrix constructor.
class UnitaryMatrixType
{ };


// used to call specific matrix constructor.
class UninitializedMatrixType
{ };


static constexpr ZeroMatrixType				zero;
static constexpr UnitaryMatrixType			unit;
static constexpr UninitializedMatrixType	uninitialized;


/**
 * Thrown whey trying to inverse an inversible matrix.
 */
class NotInversible: public std::domain_error
{
  public:
	NotInversible():
		domain_error ("Matrix is not inversible")
	{ }
};


/**
 * Thrown on range errors.
 */
class OutOfRange: public std::out_of_range
{
  public:
	OutOfRange (std::size_t column, std::size_t row):
		out_of_range ("element [" + std::to_string (column) + ", " + std::to_string (row) + "] is out of bounds in the Matrix")
	{ }
};


/**
 * A matrix. Should be self explanatory.
 *
 * \param	pScalar
 *			Algebraic value type, probably a double or something.
 */
template<class pScalar, std::size_t pColumns, std::size_t pRows>
	class Matrix
	{
	  public:
		static constexpr std::size_t kColumns	= pColumns;
		static constexpr std::size_t kRows		= pRows;

		using Scalar			= pScalar;
		using InversedScalar	= decltype (1.0 / std::declval<pScalar>());
		using ColumnVector		= Matrix<Scalar, 1, pRows>;
		using InversedMatrix	= Matrix<InversedScalar, pColumns, pRows>;
		using TransposedMatrix	= Matrix<Scalar, pRows, pColumns>;

	  public:
		[[nodiscard]]
		static constexpr bool
		is_scalar();

		[[nodiscard]]
		static constexpr bool
		is_vector();

		[[nodiscard]]
		static constexpr bool
		is_square();

	  public:
		// Ctor. Same as using ZeroMatrixType.
		constexpr
		Matrix() noexcept;

		// Ctor. Copy constructor.
		constexpr
		Matrix (Matrix const&) noexcept;

		// Ctor. Alias for default constructor (initialized to zero).
		constexpr
		Matrix (ZeroMatrixType) noexcept;

		// Ctor. Initializes to identity matrix.
		constexpr
		Matrix (UnitaryMatrixType) noexcept;

		// Ctor. Doesn't initialize matrix at all.
		constexpr
		Matrix (UninitializedMatrixType) noexcept;

		// Ctor. Initializes from scalar. Only for 1x1 matrices.
		constexpr
		Matrix (Scalar) noexcept;

		// Ctor. Initializes from scalars initializer list.
		constexpr
		Matrix (std::initializer_list<Scalar> values) noexcept;

		// Ctor. Initializes from std::array.
		constexpr
		Matrix (std::array<Scalar, kColumns * kRows> values) noexcept;

		// Ctor. Initializes columns from series of vectors.
		constexpr
		Matrix (std::array<ColumnVector, kColumns> vectors) noexcept;

		// Copy operator
		constexpr Matrix&
		operator= (Matrix const&) noexcept (std::is_copy_assignable_v<Scalar>);

		// Move operator
		constexpr Matrix&
		operator= (Matrix&&) noexcept (std::is_move_assignable_v<Scalar>) = default;

		/**
		 * Equality operator
		 */
		[[nodiscard]]
		constexpr bool
		operator== (Matrix const&) const noexcept (noexcept (Scalar{} == Scalar{}));

		/**
		 * Difference operator
		 */
		[[nodiscard]]
		constexpr bool
		operator!= (Matrix const&) const noexcept (noexcept (Scalar{} != Scalar{}));

		/**
		 * Return pointer to the data stored in the matrix, row by row.
		 */
		[[nodiscard]]
		constexpr Scalar*
		data() noexcept;

		/**
		 * Return pointer to the data stored in the matrix, row by row.
		 */
		[[nodiscard]]
		constexpr Scalar const*
		data() const noexcept;

		/**
		 * If size is 1x1, then it should be convertible to scalar.
		 */
		template<class = std::enable_if_t<is_scalar()>>
			constexpr
			operator Scalar() const noexcept;

		/**
		 * Safe element accessor. Throws std::out_of_range when accessing elements outside matrix.
		 */
		[[nodiscard]]
		Scalar&
		at (std::size_t column, std::size_t row);

		/**
		 * Safe element accessor. Throws std::out_of_range when accessing elements outside matrix.
		 */
		[[nodiscard]]
		Scalar const&
		at (std::size_t column, std::size_t row) const;

		/**
		 * Fast element accessor. Doesn't perform range checks.
		 */
		[[nodiscard]]
		constexpr Scalar&
		operator() (std::size_t column, std::size_t row) noexcept;

		/**
		 * Fast element accessor. Doesn't perform range checks.
		 */
		[[nodiscard]]
		constexpr Scalar const&
		operator() (std::size_t column, std::size_t row) const noexcept;

		/**
		 * Vector access operator.
		 * Accesses only the first column of the matrix.
		 */
		[[nodiscard]]
		constexpr Scalar&
		operator[] (std::size_t index) noexcept;

		/**
		 * Vector access operator - const version.
		 */
		[[nodiscard]]
		constexpr Scalar const&
		operator[] (std::size_t index) const noexcept;

		/**
		 * Return given column as a vector.
		 */
		[[nodiscard]]
		constexpr Matrix<Scalar, 1, kRows>
		column (std::size_t index) const noexcept;

		/**
		 * Return inversed matrix.
		 * Throw NotInversible if determiant is 0.
		 */
		[[nodiscard]]
		constexpr InversedMatrix
		inversed() const;

		/**
		 * Return transposed matrix.
		 */
		[[nodiscard]]
		constexpr TransposedMatrix
		transposed() const noexcept;

		/**
		 * Alias for transposed().
		 */
		[[nodiscard]]
		constexpr TransposedMatrix
		operator~() const noexcept;

		/**
		 * Add another matrix to this one.
		 */
		constexpr Matrix&
		operator+= (Matrix const&) noexcept (noexcept (Scalar{} + Scalar{}));

		/**
		 * Subtract another matrix from this one.
		 */
		constexpr Matrix&
		operator-= (Matrix const&) noexcept (noexcept (Scalar{} - Scalar{}));

		/**
		 * Multiply this matrix by a scalar.
		 */
		constexpr Matrix&
		operator*= (Scalar const&) noexcept (noexcept (Scalar{} * Scalar{}));

		/**
		 * Multiply this matrix by another matrix.
		 */
		constexpr Matrix&
		operator*= (Matrix const&) noexcept (noexcept (Scalar{} * Scalar{}));

	  private:
		std::array<Scalar, kColumns * kRows> _data;
	};


template<class S, std::size_t N>
	using Vector = Matrix<S, 1, N>;


template<class S, std::size_t N>
	using SquareMatrix = Matrix<S, N, N>;


/*
 * Free functions
 */


template<class S, std::size_t C, std::size_t R>
	inline typename Matrix<S, C, R>::InversedMatrix
	gauss_inverse (Matrix<S, C, R> source)
	{
		static_assert (source.is_square(), "Matrix needs to be square");

		using InversedMatrix = typename Matrix<S, C, R>::InversedMatrix;

		InversedMatrix result = InversedMatrix (unit);

		// Make matrix triangular (with 0s under the diagonal and 1s
		// on the diagonal).
		for (std::size_t result_r = 0; result_r < source.kRows; ++result_r)
		{
			// Make coefficient at (result_r, result_r) == 1.0,
			// divide row at (result_r) by its diagonal coefficient:
			auto divider = source (result_r, result_r) / S (1);

			if (divider == 0.0)
				throw NotInversible();

			for (std::size_t c = 0; c < source.kColumns; ++c)
			{
				source (c, result_r) /= divider;
				// Mirror operation on result:
				result (c, result_r) /= divider;
			}

			// Make the column at (result_r, (result_r + 1)...) == 0.0:
			for (std::size_t r = result_r + 1; r < source.kRows; ++r)
			{
				// Zeroing element at column result_r in row r,
				// subtract k * row[result_r] from row r, where
				// k = at (result_r, r):
				auto k = source (result_r, r) / S (1);

				for (std::size_t c = 0; c < source.kColumns; ++c)
				{
					source (c, r) -= k * source (c, result_r);
					// Mirror:
					result (c, r) -= k * result (c, result_r);
				}
			}
		}

		// Use diagonal 1s multiplied by k (to be found) to reduce elements
		// of previous row to 0 (except the diagonal 1 on that row).
		for (std::size_t result_r = 0; result_r < source.kRows - 1; ++result_r)
		{
			for (std::size_t result_c = result_r + 1; result_c < source.kColumns; ++result_c)
			{
				auto k = source (result_c, result_r) / S (1);
				// We want element at (result_c, result_r) to be 0.
				for (std::size_t c = 0; c < source.kColumns; ++c)
				{
					source (c, result_r) -= k * source (c, result_c);
					// Mirror operation on result:
					result (c, result_r) -= k * result (c, result_c);
				}
			}
		}

		return result;
	}


/*
 * Implementation
 */


template<class S, std::size_t C, std::size_t R>
	constexpr
	Matrix<S, C, R>::Matrix() noexcept:
		Matrix (zero)
	{ }


template<class S, std::size_t C, std::size_t R>
	constexpr
	Matrix<S, C, R>::Matrix (Matrix const& other) noexcept:
		_data (other._data)
	{ }


template<class S, std::size_t C, std::size_t R>
	constexpr
	Matrix<S, C, R>::Matrix (ZeroMatrixType) noexcept
	{
		_data.fill (Scalar());
	}


template<class S, std::size_t C, std::size_t R>
	constexpr
	Matrix<S, C, R>::Matrix (UnitaryMatrixType) noexcept:
		Matrix (zero)
	{
		static_assert (is_square(), "Matrix needs to be square");

		for (std::size_t i = 0; i < std::min (kColumns, kRows); ++i)
			(*this)(i, i) = S (1);
	}


template<class S, std::size_t C, std::size_t R>
	constexpr
	Matrix<S, C, R>::Matrix (std::initializer_list<Scalar> initial_values) noexcept
	{
		std::copy (initial_values.begin(), initial_values.end(), _data.begin());
	}


template<class S, std::size_t C, std::size_t R>
	constexpr
	Matrix<S, C, R>::Matrix (std::array<Scalar, kColumns * kRows> initial_values) noexcept
	{
		_data = initial_values;
	}


template<class S, std::size_t C, std::size_t R>
	constexpr
	Matrix<S, C, R>::Matrix (std::array<ColumnVector, kColumns> vectors) noexcept
	{
		for (std::size_t c = 0; c < kColumns; ++c)
			for (std::size_t r = 0; r < kRows; ++r)
				at (c, r) = vectors[c][r];
	}


template<class S, std::size_t C, std::size_t R>
	constexpr
	Matrix<S, C, R>::Matrix (UninitializedMatrixType) noexcept
	{ }


template<class S, std::size_t C, std::size_t R>
	constexpr
	Matrix<S, C, R>::Matrix (Scalar scalar) noexcept:
		_data ({ scalar })
	{
		static_assert (is_scalar(), "must be a scalar (1x1 matrix) to use scalar constructor");
	}


template<class S, std::size_t C, std::size_t R>
	constexpr bool
	Matrix<S, C, R>::operator== (Matrix const& other) const noexcept (noexcept (Scalar{} == Scalar{}))
	{
		return _data == other._data;
	}


template<class S, std::size_t C, std::size_t R>
	constexpr bool
	Matrix<S, C, R>::operator!= (Matrix const& other) const noexcept (noexcept (Scalar{} != Scalar{}))
	{
		return _data != other._data;
	}


template<class S, std::size_t C, std::size_t R>
	constexpr auto
	Matrix<S, C, R>::data() noexcept -> Scalar*
	{
		return _data.data();
	}


template<class S, std::size_t C, std::size_t R>
	constexpr auto
	Matrix<S, C, R>::data() const noexcept -> Scalar const*
	{
		return _data.data();
	}


template<class S, std::size_t C, std::size_t R>
	template<class>
		constexpr
		Matrix<S, C, R>::operator Scalar() const noexcept
		{
			return at (0, 0);
		}


template<class S, std::size_t C, std::size_t R>
	inline auto
	Matrix<S, C, R>::at (std::size_t column, std::size_t row) -> Scalar&
	{
		if (column >= kColumns || row >= kRows)
			throw OutOfRange (column, row);
		return _data[row * kColumns + column];
	}


template<class S, std::size_t C, std::size_t R>
	inline auto
	Matrix<S, C, R>::at (std::size_t column, std::size_t row) const -> Scalar const&
	{
		return const_cast<Matrix&> (*this).at (column, row);
	}


template<class S, std::size_t C, std::size_t R>
	constexpr auto
	Matrix<S, C, R>::operator() (std::size_t column, std::size_t row) noexcept -> Scalar&
	{
		return _data[row * kColumns + column];
	}


template<class S, std::size_t C, std::size_t R>
	constexpr auto
	Matrix<S, C, R>::operator() (std::size_t column, std::size_t row) const noexcept -> Scalar const&
	{
		return _data[row * kColumns + column];
	}


template<class S, std::size_t C, std::size_t R>
	constexpr auto
	Matrix<S, C, R>::operator[] (std::size_t index) noexcept -> Scalar&
	{
		static_assert (is_vector(), "must be a vector to use []");
		return _data[index];
	}


template<class S, std::size_t C, std::size_t R>
	constexpr auto
	Matrix<S, C, R>::operator[] (std::size_t index) const noexcept -> Scalar const&
	{
		static_assert (is_vector(), "must be a vector to use []");
		return _data[index];
	}


template<class S, std::size_t C, std::size_t R>
	constexpr bool
	Matrix<S, C, R>::is_scalar()
	{
		return kColumns == 1 && kRows == 1;
	}


template<class S, std::size_t C, std::size_t R>
	constexpr bool
	Matrix<S, C, R>::is_vector()
	{
		return kColumns == 1;
	}


template<class S, std::size_t C, std::size_t R>
	constexpr bool
	Matrix<S, C, R>::is_square()
	{
		return kColumns == kRows;
	}


template<class S, std::size_t C, std::size_t R>
	constexpr auto
	Matrix<S, C, R>::column (std::size_t index) const noexcept -> Matrix<Scalar, 1, kRows>
	{
		Matrix<S, 1, R> result;

		for (std::size_t r = 0; r < kRows; ++r)
			result[r] = at (index, r);

		return result;
	}


template<class S, std::size_t C, std::size_t R>
	constexpr auto
	Matrix<S, C, R>::inversed() const -> InversedMatrix
	{
		static_assert (is_square(), "Matrix needs to be square");

		if constexpr (is_scalar())
		{
			return { 1.0 / (*this)(0, 0) };
		}
		else if constexpr (C == 2)
		{
			auto const& self = *this;
			auto scaler = 1.0 / (self (0, 0) * self (1, 1) - self (1, 0) * self (0, 1));
			return { { scaler * self (1, 1), scaler * -self (1, 0),
					   scaler * -self (0, 1), scaler * self (0, 0) } };
		}
		else
			return gauss_inverse (*this);
	}


template<class S, std::size_t C, std::size_t R>
	constexpr auto
	Matrix<S, C, R>::transposed() const noexcept -> TransposedMatrix
	{
		Matrix<Scalar, kRows, kColumns> result;

		for (std::size_t r = 0; r < kRows; ++r)
			for (std::size_t c = 0; c < kColumns; ++c)
				result (r, c) = (*this) (c, r);

		return result;
	}


template<class S, std::size_t C, std::size_t R>
	constexpr auto
	Matrix<S, C, R>::operator~() const noexcept -> TransposedMatrix
	{
		return transposed();
	}


template<class S, std::size_t C, std::size_t R>
	constexpr Matrix<S, C, R>&
	Matrix<S, C, R>::operator= (Matrix const& other) noexcept (std::is_copy_assignable_v<Scalar>)
	{
		std::copy (other._data.begin(), other._data.end(), _data.begin());
		return *this;
	}


template<class S, std::size_t C, std::size_t R>
	constexpr Matrix<S, C, R>&
	Matrix<S, C, R>::operator+= (Matrix const& other) noexcept (noexcept (Scalar{} + Scalar{}))
	{
		std::transform (_data.begin(), _data.end(), other._data.begin(), _data.begin(), std::plus<Scalar>());
		return *this;
	}


template<class S, std::size_t C, std::size_t R>
	constexpr Matrix<S, C, R>&
	Matrix<S, C, R>::operator-= (Matrix const& other) noexcept (noexcept (Scalar{} - Scalar{}))
	{
		std::transform (_data.begin(), _data.end(), other._data.begin(), _data.begin(), std::minus<Scalar>());
		return *this;
	}


template<class S, std::size_t C, std::size_t R>
	constexpr Matrix<S, C, R>&
	Matrix<S, C, R>::operator*= (Scalar const& scalar) noexcept (noexcept (Scalar{} * Scalar{}))
	{
		std::transform (_data.begin(), _data.end(), _data.begin(), std::bind (std::multiplies<Scalar>(), scalar, std::placeholders::_1));
		return *this;
	}


template<class S, std::size_t C, std::size_t R>
	constexpr Matrix<S, C, R>&
	Matrix<S, C, R>::operator*= (Matrix const& other) noexcept (noexcept (Scalar{} * Scalar{}))
	{
		static_assert (is_square(), "Matrix needs to be square");

		// Use global operator*():
		return *this = *this * other;
	}

} // namespace math


// Local:
#include "matrix_operations.h"

#endif


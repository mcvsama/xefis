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

#ifndef MATH__MATRIX_IMPLEMENTATION_H__INCLUDED
#define MATH__MATRIX_IMPLEMENTATION_H__INCLUDED

// Standard:
#include <algorithm>
#include <array>
#include <initializer_list>
#include <type_traits>
#include <utility>
#include <functional>
#include <stdexcept>


namespace math {

// Used to call required Matrix' ctor.
enum ZeroMatrixType
{
	ZeroMatrix = 0,
};


// Used to call required Matrix' ctor.
enum IdentityMatrixType
{
	IdentityMatrix = 0,
};


// Used to call required Matrix' ctor.
enum UninitializedMatrixType
{
	UninitializedMatrix = 0,
};


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

		typedef pScalar Scalar;

	  public:
		static constexpr bool
		is_scalar();

		static constexpr bool
		is_vector();

		static constexpr bool
		is_square();

	  public:
		// Ctor. Initializes matrix using default constructor of Scalar.
		constexpr Matrix() noexcept;

		// Ctor. Copy constructor.
		constexpr Matrix (Matrix const&) noexcept;

		// Ctor. Alias for default constructor (initialized to zero).
		Matrix (ZeroMatrixType) noexcept;

		// Ctor. Initializes to identity matrix.
		Matrix (IdentityMatrixType) noexcept;

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

		/**
		 * Equality operator
		 */
		constexpr bool
		operator== (Matrix const& other) const noexcept (noexcept (Scalar{} == Scalar{}));

		/**
		 * Difference operator
		 */
		constexpr bool
		operator!= (Matrix const& other) const noexcept (noexcept (Scalar{} != Scalar{}));

		/**
		 * Return pointer to the data stored in the matrix, row by row.
		 */
		constexpr Scalar*
		data() noexcept;

		/**
		 * Return pointer to the data stored in the matrix, row by row.
		 */
		constexpr Scalar const*
		data() const noexcept;

		/**
		 * Safe element accessor. Throws std::out_of_range when accessing elements outside matrix.
		 */
		Scalar&
		at (std::size_t column, std::size_t row);

		/**
		 * Safe element accessor. Throws std::out_of_range when accessing elements outside matrix.
		 */
		Scalar const&
		at (std::size_t column, std::size_t row) const;

		/**
		 * Fast element accessor. Doesn't perform range checks.
		 */
		constexpr Scalar&
		operator() (std::size_t column, std::size_t row) noexcept;

		/**
		 * Fast element accessor. Doesn't perform range checks.
		 */
		constexpr Scalar const&
		operator() (std::size_t column, std::size_t row) const noexcept;

		/**
		 * Vector access operator.
		 * Accesses only the first column of the matrix.
		 */
		Scalar&
		operator[] (std::size_t index) noexcept;

		/**
		 * Vector access operator - const version.
		 */
		Scalar const&
		operator[] (std::size_t index) const noexcept;

		/**
		 * Return inversed matrix.
		 * Throw NotInversible if determiant is 0.
		 */
		Matrix
		inversed() const;

		/**
		 * Return transposed matrix.
		 */
		Matrix<Scalar, kRows, kColumns>
		transposed() const noexcept;

		/**
		 * Alias for transposed().
		 */
		Matrix<Scalar, kRows, kColumns>
		operator~() const noexcept;

		/**
		 * Copy-assignment operator
		 */
		Matrix&
		operator= (Matrix const&) noexcept (noexcept (Scalar{} = Scalar{}));

		/**
		 * Add another matrix to this one.
		 */
		Matrix&
		operator+= (Matrix const&) noexcept (noexcept (Scalar{} + Scalar{}));

		/**
		 * Subtract another matrix from this one.
		 */
		Matrix&
		operator-= (Matrix const&) noexcept (noexcept (Scalar{} - Scalar{}));

		/**
		 * Multiply this matrix by a scalar.
		 */
		Matrix&
		operator*= (Scalar const&) noexcept (noexcept (Scalar{} * Scalar{}));

	  private:
		void
		gauss_inverse();

	  private:
		std::array<Scalar, kColumns * kRows> _data;
	};


template<class S, std::size_t N>
	using Vector = Matrix<S, 1, N>;


template<class S, std::size_t N>
	using SquareMatrix = Matrix<S, N, N>;


/*
 * Implementation
 */


template<class S, std::size_t C, std::size_t R>
	constexpr
	Matrix<S, C, R>::Matrix() noexcept:
		Matrix (ZeroMatrix)
	{ }


template<class S, std::size_t C, std::size_t R>
	constexpr
	Matrix<S, C, R>::Matrix (Matrix const& other) noexcept:
		_data (other._data)
	{ }


template<class S, std::size_t C, std::size_t R>
	inline
	Matrix<S, C, R>::Matrix (ZeroMatrixType) noexcept
	{
		_data.fill (Scalar());
	}


template<class S, std::size_t C, std::size_t R>
	inline
	Matrix<S, C, R>::Matrix (IdentityMatrixType) noexcept:
		Matrix (ZeroMatrix)
	{
		static_assert (is_square(), "Matrix needs to be square");

		for (std::size_t i = 0; i < std::min (kColumns, kRows); ++i)
			(*this) (i, i) = 1.0;
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
	inline auto
	Matrix<S, C, R>::operator[] (std::size_t index) noexcept -> Scalar&
	{
		static_assert (is_vector(), "must be a vector to use []");
		return _data[index];
	}


template<class S, std::size_t C, std::size_t R>
	inline auto
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
	inline Matrix<S, C, R>
	Matrix<S, C, R>::inversed() const
	{
		static_assert (is_square(), "Matrix needs to be square");

		auto self = *this;

		if (is_scalar())
			return { 1.0 / self (0, 0) };
		else if (C == 2)
		{
			Scalar scaler = 1.0 / (self (0, 0) * self (1, 1) - self (1, 0) * self (0, 1));
			return { { scaler * self (1, 1), scaler * -self (1, 0),
					   scaler * -self (0, 1), scaler * self (0, 0) } };
		}
		else
		{
			Matrix result = *this;
			result.gauss_inverse();
			return result;
		}
	}


template<class S, std::size_t C, std::size_t R>
	inline Matrix<typename Matrix<S, C, R>::Scalar, Matrix<S, C, R>::kRows, Matrix<S, C, R>::kColumns>
	Matrix<S, C, R>::transposed() const noexcept
	{
		Matrix<Scalar, kRows, kColumns> result;

		for (std::size_t r = 0; r < kRows; ++r)
			for (std::size_t c = 0; c < kColumns; ++c)
				result (c, r) = (*this) (r, c);

		return result;
	}


template<class S, std::size_t C, std::size_t R>
	inline Matrix<typename Matrix<S, C, R>::Scalar, Matrix<S, C, R>::kRows, Matrix<S, C, R>::kColumns>
	Matrix<S, C, R>::operator~() const noexcept
	{
		return transposed();
	}


template<class S, std::size_t C, std::size_t R>
	inline Matrix<S, C, R>&
	Matrix<S, C, R>::operator= (Matrix const& other) noexcept (noexcept (Scalar{} = Scalar{}))
	{
		std::copy (other._data.begin(), other._data.end(), _data.begin());
		return *this;
	}


template<class S, std::size_t C, std::size_t R>
	inline Matrix<S, C, R>&
	Matrix<S, C, R>::operator+= (Matrix const& other) noexcept (noexcept (Scalar{} + Scalar{}))
	{
		std::transform (_data.begin(), _data.end(), other._data.begin(), _data.begin(), std::plus<Scalar>());
		return *this;
	}


template<class S, std::size_t C, std::size_t R>
	inline Matrix<S, C, R>&
	Matrix<S, C, R>::operator-= (Matrix const& other) noexcept (noexcept (Scalar{} - Scalar{}))
	{
		std::transform (_data.begin(), _data.end(), other._data.begin(), _data.begin(), std::minus<Scalar>());
		return *this;
	}


template<class S, std::size_t C, std::size_t R>
	inline Matrix<S, C, R>&
	Matrix<S, C, R>::operator*= (Scalar const& scalar) noexcept (noexcept (Scalar{} * Scalar{}))
	{
		std::transform (_data.begin(), _data.end(), _data.begin(), std::bind (std::multiplies<Scalar>(), scalar, std::placeholders::_1));
		return *this;
	}


template<class S, std::size_t C, std::size_t R>
	inline void
	Matrix<S, C, R>::gauss_inverse()
	{
		static_assert (is_square(), "Matrix needs to be square");

		auto self = *this;
		Matrix result = IdentityMatrix;

		// Make matrix triangular (with 0s under the diagonal and 1s
		// on the diagonal).
		for (std::size_t result_r = 0; result_r < kRows; ++result_r)
		{
			// Make coefficient at (result_r, result_r) == 1.0,
			// divide row at (result_r) by its diagonal coefficient:
			auto divider = self (result_r, result_r);
			if (divider == 0.0)
				throw NotInversible();

			for (std::size_t c = 0; c < kColumns; ++c)
			{
				self (c, result_r) /= divider;
				// Mirror operation on result:
				result (c, result_r) /= divider;
			}

			// Make the column at (result_r, (result_r + 1)...) == 0.0:
			for (std::size_t r = result_r + 1; r < kRows; ++r)
			{
				// Zeroing element at column result_r in row r,
				// subtract k * row[result_r] from row r, where
				// k = at (result_r, r):
				auto k = self (result_r, r);

				for (std::size_t c = 0; c < kColumns; ++c)
				{
					self (c, r) -= k * self (c, result_r);
					// Mirror:
					result (c, r) -= k * result (c, result_r);
				}
			}
		}

		// Use diagonal 1s multiplied by k (to be found) to reduce elements
		// of previous row to 0 (except the diagonal 1 on that row).
		for (std::size_t result_r = 0; result_r < kRows - 1; ++result_r)
		{
			for (std::size_t result_c = result_r + 1; result_c < kColumns; ++result_c)
			{
				auto k = self (result_c, result_r);
				// We want element at (result_c, result_r) to be 0.
				for (std::size_t c = 0; c < kColumns; ++c)
				{
					self (c, result_r) -= k * self (c, result_c);
					// Mirror operation on result:
					result (c, result_r) -= k * result (c, result_c);
				}
			}
		}

		*this = result;
	}

} // namespace math

#endif


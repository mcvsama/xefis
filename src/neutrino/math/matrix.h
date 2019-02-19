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

#ifndef NEUTRINO__MATH__MATRIX_H__INCLUDED
#define NEUTRINO__MATH__MATRIX_H__INCLUDED

// Standard:
#include <algorithm>
#include <array>
#include <type_traits>
#include <utility>
#include <functional>
#include <stdexcept>
#include <type_traits>

// Neutrino:
#include <neutrino/c++20.h>


namespace neutrino::math {

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
 * \param	pTargetFrame
 *			A target frame of reference type tag.
 *			Matrix is supposed to transform frame of reference from pSourceFrame to pTargetFrame. This can be useful when using matrices as transformations for
 *			vector spaces. If used, Matrices with different type tags become incompatible, but certain operations are still defined, for example:
 *			  struct TargetFrame;
 *			  struct IntermediateFrame;
 *			  struct SourceFrame;
 *			  M<..., TargetFrame, SourceFrame> x = M<..., TargetFrame, IntermediateFrame>{} * M<..., IntermediateFrame, SourceFrame>{};
 * \param	pSourceFrame
 *			Source frame of reference.
 */
template<class pScalar, std::size_t pColumns, std::size_t pRows, class pTargetFrame = void, class pSourceFrame = pTargetFrame>
	class Matrix
	{
	  public:
		static constexpr std::size_t kColumns	= pColumns;
		static constexpr std::size_t kRows		= pRows;

		using Scalar			= pScalar;
		using InversedScalar	= decltype (1.0 / std::declval<pScalar>());
		using ColumnVector		= Matrix<Scalar, 1, pRows, pTargetFrame, void>;
		using InversedMatrix	= Matrix<InversedScalar, pColumns, pRows, pSourceFrame, pTargetFrame>;
		using TransposedMatrix	= Matrix<Scalar, pRows, pColumns, pSourceFrame, pTargetFrame>;
		using TargetFrame		= pTargetFrame;
		using SourceFrame		= pSourceFrame;

		template<std::size_t NewColumns, std::size_t NewRows>
			using Resized		= Matrix<Scalar, NewColumns, NewRows, pTargetFrame, pSourceFrame>;

		template<class NewScalar>
			using Retyped		= Matrix<NewScalar, pColumns, pRows, pTargetFrame, pSourceFrame>;

	  public:
		[[nodiscard]]
		static constexpr bool
		is_scalar()
			{ return kColumns == 1 && kRows == 1; }

		[[nodiscard]]
		static constexpr bool
		is_vector()
			{ return kColumns == 1; }

		[[nodiscard]]
		static constexpr bool
		is_square()
			{ return kColumns == kRows; }

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
		template<class T>
			requires (is_scalar() && std::is_convertible_v<std::remove_cvref_t<T>, Scalar>)
			constexpr
			Matrix (T scalar) noexcept:
				_data ({ scalar })
			{ }

		// Ctor. Initializes from std::array of scalars.
		explicit constexpr
		Matrix (std::array<Scalar, kColumns * kRows> values) noexcept;

		// Ctor. Initializes columns from std::array of vectors.
		explicit constexpr
		Matrix (std::array<ColumnVector, kColumns> vectors) noexcept;

		// Ctor. Initializes from scalars list.
		template<class ...Ts>
			requires ((std::is_convertible_v<std::remove_cvref_t<Ts>, Scalar> && ...) || (std::is_same_v<std::remove_cvref_t<Ts>, ColumnVector> && ...))
			constexpr
			Matrix (Ts&& ...values) noexcept
			{
				if constexpr ((std::is_convertible_v<std::remove_cvref_t<Ts>, Scalar> && ...))
				{
					static_assert (sizeof...(Ts) == kRows * kColumns, "Invalid number of scalar arguments");

					recursive_initialize_from_scalars (0, std::forward<Ts> (values)...);
				}
				else if constexpr ((std::is_same_v<std::remove_cvref_t<Ts>, ColumnVector> && ...))
				{
					static_assert (sizeof...(Ts) == kColumns, "Invalid number of vector arguments");

					recursive_initialize_from_vectors (0, std::forward<Ts> (values)...);
				}
			}

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
		operator== (Matrix const& other) const noexcept (noexcept (Scalar{} == Scalar{}))
			{ return _data == other._data; }

		/**
		 * Difference operator
		 */
		[[nodiscard]]
		constexpr bool
		operator!= (Matrix const& other) const noexcept (noexcept (Scalar{} != Scalar{}))
			{ return _data != other._data; }

		/**
		 * Array of data.
		 */
		[[nodiscard]]
		constexpr std::array<Scalar, kColumns * kRows>&
		array() noexcept
			{ return _data; }

		/**
		 * Array of data.
		 */
		[[nodiscard]]
		constexpr std::array<Scalar, kColumns * kRows> const&
		array() const noexcept
			{ return _data; }

		/**
		 * If size is 1x1, then it should be convertible to scalar.
		 */
		template<class = std::enable_if_t<is_scalar()>>
			constexpr
			operator Scalar() const noexcept
				{ return at (0, 0); }

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
		at (std::size_t column, std::size_t row) const
			{ return const_cast<Matrix&> (*this).at (column, row); }

		/**
		 * Fast element accessor. Doesn't perform range checks.
		 */
		[[nodiscard]]
		constexpr Scalar&
		operator() (std::size_t column, std::size_t row) noexcept
			{ return _data[row * kColumns + column]; }

		/**
		 * Fast element accessor. Doesn't perform range checks.
		 */
		[[nodiscard]]
		constexpr Scalar const&
		operator() (std::size_t column, std::size_t row) const noexcept
			{ return _data[row * kColumns + column]; }

		/**
		 * Vector access operator.
		 * Accesses only the first column of the matrix.
		 */
		template<class = std::enable_if_t<is_vector()>>
			[[nodiscard]]
			constexpr Scalar&
			operator[] (std::size_t index) noexcept
				{ return _data[index]; }

		/**
		 * Vector access operator - const version.
		 */
		template<class = std::enable_if_t<is_vector()>>
			[[nodiscard]]
			constexpr Scalar const&
			operator[] (std::size_t index) const noexcept
				{ return _data[index]; }

		/**
		 * Return given column as a vector.
		 */
		[[nodiscard]]
		constexpr ColumnVector
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
		operator~() const noexcept
			{ return transposed(); }

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
		template<class OtherScalar>
			requires (sizeof (Scalar{} *= OtherScalar{}) > 0)
			constexpr Matrix&
			operator*= (OtherScalar const& scalar) noexcept (noexcept (Scalar{} *= OtherScalar{}))
			{
				for (auto& d: _data)
					d *= scalar;

				return *this;
			}

		/**
		 * Multiply this matrix by another matrix.
		 */
		template<class OtherScalar>
			requires (sizeof (Scalar{} *= OtherScalar{}) > 0)
			constexpr Matrix&
			operator*= (Retyped<OtherScalar> const& other) noexcept (noexcept (Scalar{} *= OtherScalar{}))
			{
				static_assert (is_square(), "Matrix needs to be square");

				// Use global operator*():
				return *this = *this * other;
			}

	  private:
		template<class ...Ts>
			constexpr void
			recursive_initialize_from_scalars (std::size_t position, Scalar const& scalar, Ts&& ...rest) noexcept
			{
				_data[position] = scalar;

				if constexpr (sizeof...(rest) > 0)
					recursive_initialize_from_scalars (position + 1, std::forward<Ts> (rest)...);
			}

		template<class ...Ts>
			constexpr void
			recursive_initialize_from_vectors (std::size_t position, ColumnVector const& vector, Ts&& ...rest) noexcept
			{
				for (std::size_t r = 0; r < kRows; ++r)
					at (position, r) = vector[r];

				if constexpr (sizeof...(rest) > 0)
					recursive_initialize_from_vectors (position + 1, std::forward<Ts> (rest)...);
			}

	  private:
		std::array<Scalar, kColumns * kRows> _data;
	};


template<class S, std::size_t N, class TF = void, class SF = void>
	using Vector = Matrix<S, 1, N, TF, SF>;


template<class S, std::size_t N, class TF = void, class SF = void>
	using SquareMatrix = Matrix<S, N, N, TF, SF>;


/*
 * Implementation
 */


template<class S, std::size_t C, std::size_t R, class TF, class SF>
	constexpr
	Matrix<S, C, R, TF, SF>::Matrix() noexcept:
		Matrix (zero)
	{ }


template<class S, std::size_t C, std::size_t R, class TF, class SF>
	constexpr
	Matrix<S, C, R, TF, SF>::Matrix (Matrix const& other) noexcept:
		_data (other._data)
	{ }


template<class S, std::size_t C, std::size_t R, class TF, class SF>
	constexpr
	Matrix<S, C, R, TF, SF>::Matrix (ZeroMatrixType) noexcept
	{
		_data.fill (Scalar());
	}


template<class S, std::size_t C, std::size_t R, class TF, class SF>
	constexpr
	Matrix<S, C, R, TF, SF>::Matrix (UnitaryMatrixType) noexcept:
		Matrix (zero)
	{
		static_assert (is_square(), "Matrix has to be square");

		for (std::size_t i = 0; i < std::min (kColumns, kRows); ++i)
			(*this)(i, i) = S (1);
	}


template<class S, std::size_t C, std::size_t R, class TF, class SF>
	constexpr
	Matrix<S, C, R, TF, SF>::Matrix (UninitializedMatrixType) noexcept
	{ }


template<class S, std::size_t C, std::size_t R, class TF, class SF>
	constexpr
	Matrix<S, C, R, TF, SF>::Matrix (std::array<Scalar, kColumns * kRows> initial_values) noexcept
	{
		_data = initial_values;
	}


template<class S, std::size_t C, std::size_t R, class TF, class SF>
	constexpr
	Matrix<S, C, R, TF, SF>::Matrix (std::array<ColumnVector, kColumns> initial_vectors) noexcept
	{
		for (std::size_t r = 0; r < kRows; ++r)
			for (std::size_t c = 0; c < kColumns; ++c)
				at (c, r) = initial_vectors[c][r];
	}


template<class S, std::size_t C, std::size_t R, class TF, class SF>
	inline auto
	Matrix<S, C, R, TF, SF>::at (std::size_t column, std::size_t row) -> Scalar&
	{
		if (column >= kColumns || row >= kRows)
			throw OutOfRange (column, row);

		return _data[row * kColumns + column];
	}


template<class S, std::size_t C, std::size_t R, class TF, class SF>
	constexpr auto
	Matrix<S, C, R, TF, SF>::column (std::size_t index) const noexcept -> ColumnVector
	{
		ColumnVector result;

		for (std::size_t r = 0; r < kRows; ++r)
			result[r] = at (index, r);

		return result;
	}


template<class S, std::size_t C, std::size_t R, class TF, class SF>
	constexpr auto
	Matrix<S, C, R, TF, SF>::inversed() const -> InversedMatrix
	{
		static_assert (is_square(), "Matrix needs to be square");
		static_assert (kColumns <= 3, "Inversion of rank > 3 is not implemented.\n");

		if constexpr (is_scalar())
		{
			return { 1.0 / (*this)(0, 0) };
		}
		else if constexpr (kColumns == 2)
		{
			auto const& self = *this;
			auto const det = (self (0, 0) * self (1, 1) - self (1, 0) * self (0, 1));
			auto const scaler = 1.0 / det;

			return InversedMatrix {
				scaler * +self (1, 1), scaler * -self (1, 0),
				scaler * -self (0, 1), scaler * +self (0, 0),
			};
		}
		else if constexpr (kColumns == 3)
		{
			auto const& self = *this;
			auto const a = self (0, 0);
			auto const b = self (1, 0);
			auto const c = self (2, 0);
			auto const d = self (0, 1);
			auto const e = self (1, 1);
			auto const f = self (2, 1);
			auto const g = self (0, 2);
			auto const h = self (1, 2);
			auto const i = self (2, 2);
			auto const kA = +(e * i - f * h);
			auto const kB = -(d * i - f * g);
			auto const kC = +(d * h - e * g);
			auto const kD = -(b * i - c * h);
			auto const kE = +(a * i - c * g);
			auto const kF = -(a * h - b * g);
			auto const kG = +(b * f - c * e);
			auto const kH = -(a * f - c * d);
			auto const kI = +(a * e - b * d);
			auto const det = a * kA + b * kB + c * kC;
			auto const scaler = 1.0 / det;

			return InversedMatrix {
				scaler * kA, scaler * kD, scaler * kG,
				scaler * kB, scaler * kE, scaler * kH,
				scaler * kC, scaler * kF, scaler * kI,
			};
		}
	}


template<class S, std::size_t C, std::size_t R, class TF, class SF>
	constexpr auto
	Matrix<S, C, R, TF, SF>::transposed() const noexcept -> TransposedMatrix
	{
		Matrix<Scalar, kRows, kColumns, SF, TF> result;

		for (std::size_t r = 0; r < kRows; ++r)
			for (std::size_t c = 0; c < kColumns; ++c)
				result (r, c) = (*this) (c, r);

		return result;
	}


template<class S, std::size_t C, std::size_t R, class TF, class SF>
	constexpr Matrix<S, C, R, TF, SF>&
	Matrix<S, C, R, TF, SF>::operator= (Matrix const& other) noexcept (std::is_copy_assignable_v<Scalar>)
	{
		std::copy (other._data.begin(), other._data.end(), _data.begin());
		return *this;
	}


template<class S, std::size_t C, std::size_t R, class TF, class SF>
	constexpr Matrix<S, C, R, TF, SF>&
	Matrix<S, C, R, TF, SF>::operator+= (Matrix const& other) noexcept (noexcept (Scalar{} + Scalar{}))
	{
		std::transform (_data.begin(), _data.end(), other._data.begin(), _data.begin(), std::plus<Scalar>());
		return *this;
	}


template<class S, std::size_t C, std::size_t R, class TF, class SF>
	constexpr Matrix<S, C, R, TF, SF>&
	Matrix<S, C, R, TF, SF>::operator-= (Matrix const& other) noexcept (noexcept (Scalar{} - Scalar{}))
	{
		std::transform (_data.begin(), _data.end(), other._data.begin(), _data.begin(), std::minus<Scalar>());
		return *this;
	}


/*
 * Global functions
 */


template<class NewTF, class NewSF, class S, std::size_t C, std::size_t R, class TF, class SF>
	constexpr Matrix<S, C, R, NewTF, NewSF>&
	reframe (Matrix<S, C, R, TF, SF>& matrix)
	{
		return reinterpret_cast<Matrix<S, C, R, NewTF, NewSF>&> (matrix);
	}


template<class NewTF, class NewSF, class S, std::size_t C, std::size_t R, class TF, class SF>
	constexpr Matrix<S, C, R, NewTF, NewSF> const&
	reframe (Matrix<S, C, R, TF, SF> const& matrix)
	{
		return reinterpret_cast<Matrix<S, C, R, NewTF, NewSF> const&> (matrix);
	}

} // namespace neutrino::math


// Local:
#include "matrix_operations.h"

#endif


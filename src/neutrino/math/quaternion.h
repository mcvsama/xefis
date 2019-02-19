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

#ifndef NEUTRINO__MATH__QUATERNION_H__INCLUDED
#define NEUTRINO__MATH__QUATERNION_H__INCLUDED

// Standard:
#include <cmath>

// Local:
#include "matrix.h"


namespace neutrino::math {

template<class pScalar>
	class Quaternion
	{
	  public:
		using Scalar = pScalar;

	  public:
		// Ctor
		constexpr
		Quaternion() = default;

		// Copy ctor
		constexpr
		Quaternion (Quaternion const&) = default;

		// Move ctor
		constexpr
		Quaternion (Quaternion&&) = default;

		// Ctor
		constexpr
		Quaternion (Vector<Scalar, 3> const&);

		// Ctor
		// Constructs proper quaternion.
		constexpr
		Quaternion (Vector<Scalar, 4> const&);

		// Ctor
		constexpr
		Quaternion (std::array<Scalar, 4> const&);

		// Ctor
		constexpr
		Quaternion (Scalar w, Scalar x, Scalar y, Scalar z);

		// Ctor
		constexpr
		Quaternion (Scalar w, Vector<Scalar, 3> const&);

		// Copy operator
		constexpr Quaternion&
		operator= (Quaternion const&) noexcept (std::is_copy_assignable_v<Scalar>) = default;

		// Move operator
		constexpr Quaternion&
		operator= (Quaternion&&) noexcept (std::is_move_assignable_v<Scalar>) = default;

		/**
		 * Equality operator
		 */
		[[nodiscard]]
		constexpr bool
		operator== (Quaternion const&) const noexcept (noexcept (Scalar{} == Scalar{}));

		/**
		 * Difference operator
		 */
		[[nodiscard]]
		constexpr bool
		operator!= (Quaternion const&) const noexcept (noexcept (Scalar{} != Scalar{}));

		/**
		 * Return the rotation value.
		 */
		[[nodiscard]]
		Scalar&
		w() noexcept { return _components[0]; }

		/**
		 * Return the rotation value.
		 */
		[[nodiscard]]
		Scalar const&
		w() const noexcept { return _components[0]; }

		/**
		 * Return the x position.
		 */
		[[nodiscard]]
		Scalar&
		x() noexcept { return _components[1]; };

		/**
		 * Return the x position.
		 */
		[[nodiscard]]
		Scalar const&
		x() const noexcept { return _components[1]; }

		/**
		 * Return the y position.
		 */
		[[nodiscard]]
		Scalar&
		y() noexcept { return _components[2]; }

		/**
		 * Return the y position.
		 */
		[[nodiscard]]
		Scalar const&
		y() const noexcept { return _components[2]; };

		/**
		 * Return the z position.
		 */
		[[nodiscard]]
		Scalar&
		z() noexcept { return _components[3]; };

		/**
		 * Return the z position.
		 */
		[[nodiscard]]
		Scalar const&
		z() const noexcept { return _components[3]; };

		/**
		 * Return the array of components { w, x, y, z }.
		 */
		[[nodiscard]]
		constexpr std::array<Scalar, 4> const&
		components() const noexcept;

		/**
		 * Return the vector of imaginary components.
		 */
		[[nodiscard]]
		constexpr Vector<Scalar, 3>
		imag() const noexcept;

		/**
		 * Return quaternion's norm.
		 */
		[[nodiscard]]
		constexpr Scalar
		norm() const noexcept;

		/**
		 * Normalize this quaternion inline.
		 */
		constexpr void
		normalize();

		/**
		 * Return new, normalized quaternion.
		 */
		[[nodiscard]]
		constexpr Quaternion
		normalized() const;

		/**
		 * Conjugate this quaternion.
		 */
		constexpr void
		conjugate();

		/**
		 * Return new, conjugated quaternion.
		 */
		[[nodiscard]]
		constexpr Quaternion
		conjugated() const;

		/**
		 * Inverse this quaternion.
		 */
		constexpr void
		inverse();

		/**
		 * Return new, inversed quaternion.
		 */
		[[nodiscard]]
		constexpr Quaternion
		inversed() const;

		/**
		 * Alias for conjugated().
		 */
		[[nodiscard]]
		constexpr Quaternion
		operator~() const noexcept;

		/**
		 * Add another quaternion to this one.
		 */
		constexpr Quaternion&
		operator+= (Quaternion const&) noexcept (noexcept (Scalar{} + Scalar{}));

		/**
		 * Subtract another quaternion from this one.
		 */
		constexpr Quaternion&
		operator-= (Quaternion const&) noexcept (noexcept (Scalar{} - Scalar{}));

		/**
		 * Multiply this quaternion by a scalar.
		 */
		constexpr Quaternion&
		operator*= (Scalar const&) noexcept (noexcept (Scalar{} * Scalar{}));

		/**
		 * Multiply this quaternion by another quaternion.
		 */
		constexpr Quaternion&
		operator*= (Quaternion const&) noexcept (noexcept (Scalar{} * Scalar{}));

		/**
		 * Divide this quaternion by a scalar.
		 */
		constexpr Quaternion&
		operator/= (Scalar const&) noexcept (noexcept (Scalar{} / Scalar{}));

		/**
		 * Divide this quaternion by by another quaternion.
		 */
		constexpr Quaternion&
		operator/= (Quaternion const&) noexcept (noexcept (Scalar{} / Scalar{}));

	  private:
		std::array<Scalar, 4> _components;
	};


template<class S>
	constexpr
	Quaternion<S>::Quaternion (Vector<Scalar, 3> const& vector):
		_components ({ Scalar (0), vector[0], vector[1], vector[2] })
	{ }


template<class S>
	constexpr
	Quaternion<S>::Quaternion (Vector<Scalar, 4> const& vector):
		_components ({ vector[0], vector[1], vector[2], vector[3] })
	{ }


template<class S>
	constexpr
	Quaternion<S>::Quaternion (std::array<Scalar, 4> const& array):
		_components (array)
	{ }


template<class S>
	constexpr
	Quaternion<S>::Quaternion (Scalar w, Scalar x, Scalar y, Scalar z):
		_components { w, x, y, z }
	{ }


template<class S>
	constexpr
	Quaternion<S>::Quaternion (Scalar w, Vector<Scalar, 3> const& vector):
		_components { w, vector[0], vector[1], vector[2] }
	{ }


template<class S>
	constexpr bool
	Quaternion<S>::operator== (Quaternion const& other) const noexcept (noexcept (Scalar{} == Scalar{}))
	{
		return _components == other._components;
	}


template<class S>
	constexpr bool
	Quaternion<S>::operator!= (Quaternion const& other) const noexcept (noexcept (Scalar{} != Scalar{}))
	{
		return !(*this == other);
	}


template<class S>
	constexpr auto
	Quaternion<S>::components() const noexcept -> std::array<Scalar, 4> const&
	{
		return _components;
	}


template<class S>
	constexpr auto
	Quaternion<S>::imag() const noexcept -> Vector<Scalar, 3>
	{
		return Vector<Scalar, 3> { x(), y(), z() };
	}


template<class S>
	constexpr auto
	Quaternion<S>::norm() const noexcept -> Scalar
	{
		Scalar sum { 0 };

		for (auto c: _components)
			sum += c * c;

		return std::sqrt (sum);
	}


template<class S>
	constexpr void
	Quaternion<S>::normalize()
	{
		*this /= norm();
	}


template<class S>
	constexpr Quaternion<S>
	Quaternion<S>::normalized() const
	{
		return Quaternion<S> (*this) / norm();
	}


template<class S>
	constexpr void
	Quaternion<S>::conjugate()
	{
		x() = -x();
		y() = -y();
		z() = -z();
	}


template<class S>
	constexpr Quaternion<S>
	Quaternion<S>::conjugated() const
	{
		Quaternion copy (*this);
		copy.conjugate();
		return copy;
	}


template<class S>
	constexpr void
	Quaternion<S>::inverse()
	{
		Scalar sum { 0 };

		for (auto c: _components)
			sum += c * c;

		conjugate();
		*this /= sum;
	}


template<class S>
	constexpr Quaternion<S>
	Quaternion<S>::inversed() const
	{
		Quaternion copy (*this);
		copy.inverse();
		return copy;
	}


template<class S>
	constexpr Quaternion<S>
	Quaternion<S>::operator~() const noexcept
	{
		return conjugated();
	}


template<class S>
	constexpr Quaternion<S>&
	Quaternion<S>::operator+= (Quaternion const& other) noexcept (noexcept (Scalar{} + Scalar{}))
	{
		for (std::size_t i = 0; i < _components.size(); ++i)
			_components[i] += other._components[i];

		return *this;
	}


template<class S>
	constexpr Quaternion<S>&
	Quaternion<S>::operator-= (Quaternion const& other) noexcept (noexcept (Scalar{} - Scalar{}))
	{
		for (std::size_t i = 0; i < _components.size(); ++i)
			_components[i] -= other._components[i];

		return *this;
	}


template<class S>
	constexpr Quaternion<S>&
	Quaternion<S>::operator*= (Scalar const& scalar) noexcept (noexcept (Scalar{} * Scalar{}))
	{
		for (std::size_t i = 0; i < _components.size(); ++i)
			_components[i] *= scalar;

		return *this;
	}


template<class S>
	constexpr Quaternion<S>&
	Quaternion<S>::operator*= (Quaternion const& other) noexcept (noexcept (Scalar{} * Scalar{}))
	{
		auto const w1 = _components[0];	auto const w2 = other._components[0];
		auto const x1 = _components[1];	auto const x2 = other._components[1];
		auto const y1 = _components[2];	auto const y2 = other._components[2];
		auto const z1 = _components[3];	auto const z2 = other._components[3];

		_components[0] = w1 * w2 - x1 * x2 - y1 * y2 - z1 * z2;
		_components[1] = w1 * x2 + x1 * w2 + y1 * z2 - z1 * y2;
		_components[2] = w1 * y2 - x1 * z2 + y1 * w2 + z1 * x2;
		_components[3] = w1 * z2 + x1 * y2 - y1 * x2 + z1 * w2;

		return *this;
	}


template<class S>
	constexpr Quaternion<S>&
	Quaternion<S>::operator/= (Scalar const& scalar) noexcept (noexcept (Scalar{} / Scalar{}))
	{
		for (std::size_t i = 0; i < _components.size(); ++i)
			_components[i] /= scalar;

		return *this;
	}


template<class S>
	constexpr Quaternion<S>&
	Quaternion<S>::operator/= (Quaternion const& other) noexcept (noexcept (Scalar{} / Scalar{}))
	{
		return (*this) *= other.inversed();
	}

} // namespace neutrino::math


#include "quaternion_operations.h"

#endif


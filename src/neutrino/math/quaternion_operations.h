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

#ifndef NEUTRINO__MATH__QUATERNION_OPERATIONS_H__INCLUDED
#define NEUTRINO__MATH__QUATERNION_OPERATIONS_H__INCLUDED

// Standard:
#include <cmath>

// Local:
#include "matrix.h"


namespace neutrino::math {

template<class S>
	constexpr Quaternion<S>
	operator+ (Quaternion<S> q) noexcept
	{
		return q;
	}


template<class S>
	constexpr auto
	operator+ (Quaternion<S> a, Quaternion<S> const& b) noexcept
	{
		return a += b;
	}


template<class S>
	constexpr Quaternion<S>
	operator- (Quaternion<S> q) noexcept
	{
		return Quaternion (-q.w(), -q.x(), -q.y(), -q.z());
	}


template<class S>
	constexpr auto
	operator- (Quaternion<S> a, Quaternion<S> const& b) noexcept
	{
		return a -= b;
	}


template<class S>
	constexpr auto
	operator* (Quaternion<S> q, typename Quaternion<S>::Scalar const& scalar) noexcept
	{
		return q *= scalar;
	}


template<class S>
	constexpr auto
	operator* (typename Quaternion<S>::Scalar const& scalar, Quaternion<S> q) noexcept
	{
		return q *= scalar;
	}


template<class S>
	constexpr auto
	operator* (Quaternion<S> a, Quaternion<S> const& b) noexcept
	{
		return a *= b;
	}


template<class S>
	constexpr auto
	operator/ (Quaternion<S> q, typename Quaternion<S>::Scalar const& scalar)
	{
		return q /= scalar;
	}


template<class S>
	constexpr auto
	operator/ (typename Quaternion<S>::Scalar const& scalar, Quaternion<S> const& q)
	{
		return scalar * q.inverted();
	}


template<class S>
	constexpr auto
	operator/ (Quaternion<S> a, Quaternion<S> const& b)
	{
		return a /= b;
	}

} // namespace neutrino::math

#endif


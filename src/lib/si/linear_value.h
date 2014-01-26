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

#ifndef SI__LINEAR_VALUE_H__INCLUDED
#define SI__LINEAR_VALUE_H__INCLUDED

// Standard:
#include <cstddef>
#include <limits>
#include <type_traits>

// Boost:
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

// Xefis:
#include <xefis/config/exception.h>

// Local:
#include "value.h"


namespace SI {

/**
 * Implementer should add constructors from specific value types,
 * for example Angle would have constructors that take Degrees and Radians.
 *
 * Use: class Angle: public LinearValue<double, Angle>.
 */
template<class tValueType, class tDerived>
	class LinearValue:
		public TypedValue<tValueType>
	{
	  public:
		typedef tValueType					ValueType;
		typedef SI::TypedValue<tValueType>	TypedValue;
		typedef tDerived					Derived;

	  protected:
		explicit constexpr
		LinearValue (ValueType);

		constexpr
		LinearValue() noexcept = default;

		explicit constexpr
		LinearValue (LinearValue const&) noexcept = default;

	  public:
		static constexpr Derived
		from_internal (ValueType internal);

		/*
		 * Unary operators
		 */

		constexpr Derived
		operator+() const noexcept;

		constexpr Derived
		operator-() const noexcept;

		/*
		 * Binary operators
		 */

		constexpr bool
		operator< (Derived) const noexcept;

		constexpr bool
		operator> (Derived) const noexcept;

		constexpr bool
		operator<= (Derived) const noexcept;

		constexpr bool
		operator>= (Derived) const noexcept;

		constexpr bool
		operator== (Derived) const noexcept;

		constexpr bool
		operator!= (Derived) const noexcept;

		constexpr Derived
		operator+ (Derived) const noexcept;

		constexpr Derived
		operator- (Derived) const noexcept;

		constexpr Derived
		operator* (ValueType) const noexcept;

		constexpr ValueType
		operator/ (Derived) const noexcept;

		constexpr Derived
		operator/ (ValueType) const noexcept;

		/*
		 * Self modifiers
		 */

		Derived&
		operator+= (Derived) noexcept;

		Derived&
		operator-= (Derived) noexcept;

		Derived&
		operator*= (ValueType) noexcept;

		Derived&
		operator/= (ValueType) noexcept;
	};


inline
UnparsableValue::UnparsableValue (std::string const& message):
	Exception (message)
{ }


inline
UnsupportedUnit::UnsupportedUnit (std::string const& message):
	Exception (message)
{ }


template<class V, class T>
	inline constexpr
	LinearValue<V, T>::LinearValue (ValueType value):
		TypedValue (value)
	{ }


template<class V, class T>
	inline constexpr typename LinearValue<V, T>::Derived
	LinearValue<V, T>::from_internal (ValueType internal)
	{
		return Derived (internal);
	}


template<class V, class T>
	inline constexpr typename LinearValue<V, T>::Derived
	LinearValue<V, T>::operator+() const noexcept
	{
		return Derived (+this->internal());
	}


template<class V, class T>
	inline constexpr typename LinearValue<V, T>::Derived
	LinearValue<V, T>::operator-() const noexcept
	{
		return Derived (-this->internal());
	}


template<class V, class T>
	inline constexpr bool
	LinearValue<V, T>::operator< (Derived other) const noexcept
	{
		return this->internal() < other.internal();
	}


template<class V, class T>
	inline constexpr bool
	LinearValue<V, T>::operator> (Derived other) const noexcept
	{
		return this->internal() > other.internal();
	}


template<class V, class T>
	inline constexpr bool
	LinearValue<V, T>::operator<= (Derived other) const noexcept
	{
		return this->internal() <= other.internal();
	}


template<class V, class T>
	inline constexpr bool
	LinearValue<V, T>::operator>= (Derived other) const noexcept
	{
		return this->internal() >= other.internal();
	}


template<class V, class T>
	inline constexpr bool
	LinearValue<V, T>::operator== (Derived other) const noexcept
	{
		return this->internal() == other.internal();
	}


template<class V, class T>
	inline constexpr bool
	LinearValue<V, T>::operator!= (Derived other) const noexcept
	{
		return this->internal() != other.internal();
	}


template<class V, class T>
	inline constexpr typename LinearValue<V, T>::Derived
	LinearValue<V, T>::operator+ (Derived other) const noexcept
	{
		return Derived (this->internal() + other.internal());
	}


template<class V, class T>
	inline constexpr typename LinearValue<V, T>::Derived
	LinearValue<V, T>::operator- (Derived other) const noexcept
	{
		return Derived (this->internal() - other.internal());
	}


template<class V, class T>
	inline constexpr typename LinearValue<V, T>::Derived
	LinearValue<V, T>::operator* (ValueType factor) const noexcept
	{
		return Derived (this->internal() * factor);
	}


template<class V, class T>
	inline constexpr typename LinearValue<V, T>::ValueType
	LinearValue<V, T>::operator/ (Derived other) const noexcept
	{
		return this->internal() / other.internal();
	}


template<class V, class T>
	inline constexpr typename LinearValue<V, T>::Derived
	LinearValue<V, T>::operator/ (ValueType factor) const noexcept
	{
		return Derived (this->internal() / factor);
	}


template<class V, class T>
	inline typename LinearValue<V, T>::Derived&
	LinearValue<V, T>::operator+= (Derived other) noexcept
	{
		this->internal() += other.internal();
		return *static_cast<Derived*> (this);
	}


template<class V, class T>
	inline typename LinearValue<V, T>::Derived&
	LinearValue<V, T>::operator-= (Derived other) noexcept
	{
		this->internal() -= other.internal();
		return *static_cast<Derived*> (this);
	}


template<class V, class T>
	inline typename LinearValue<V, T>::Derived&
	LinearValue<V, T>::operator*= (ValueType factor) noexcept
	{
		this->internal() *= factor;
		return *static_cast<Derived*> (this);
	}


template<class V, class T>
	inline typename LinearValue<V, T>::Derived&
	LinearValue<V, T>::operator/= (ValueType factor) noexcept
	{
		this->internal() /= factor;
		return *static_cast<Derived*> (this);
	}


/*
 * Global functions
 */


template<class V, class T>
	inline constexpr typename LinearValue<V, T>::Derived
	operator* (typename LinearValue<V, T>::ValueType a, LinearValue<V, T> const& b) noexcept
	{
		return b * a;
	}

} // namespace SI


namespace std {

template<class D, class = typename std::enable_if<std::is_base_of<SI::LinearValue<typename D::ValueType, D>, D>::value>::type>
	inline D
	abs (D const& a)
	{
		return D::from_internal (std::abs (a.internal()));
	}

} // namespace std

#endif


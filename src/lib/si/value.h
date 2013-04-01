/* vim:ts=4
 *
 * Copyleft 2012…2013  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef SI__VALUE_H__INCLUDED
#define SI__VALUE_H__INCLUDED

// Standard:
#include <cstddef>
#include <limits>


namespace SI {

/**
 * Implementer should add constructors from specific value types,
 * for example Angle would have constructors that take Degrees and Radians.
 *
 * Use: class Angle: public Value<double, Angle>.
 */
template<class tValueType, class tDerived>
	class Value
	{
	  public:
		typedef tValueType	ValueType;
		typedef tDerived	Derived;

	  protected:
		constexpr explicit
		Value (ValueType);

	  public:
		constexpr
		Value() noexcept = default;

		explicit constexpr
		Value (Value const&) noexcept = default;

		/**
		 * Access internal representation.
		 */
		ValueType&
		internal() noexcept;

		/**
		 * Access internal representation (const version).
		 */
		constexpr ValueType const&
		internal() const noexcept;

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

	  protected:
		ValueType&
		value() noexcept;

		constexpr ValueType
		value() const noexcept;

	  private:
		ValueType _value = ValueType();
	};


template<class V, class T>
	inline constexpr
	Value<V, T>::Value (ValueType value):
		_value (value)
	{ }


template<class V, class T>
	inline typename Value<V, T>::ValueType&
	Value<V, T>::internal() noexcept
	{
		return _value;
	}


template<class V, class T>
	inline constexpr typename Value<V, T>::ValueType const&
	Value<V, T>::internal() const noexcept
	{
		return _value;
	}


template<class V, class T>
	inline constexpr typename Value<V, T>::Derived
	Value<V, T>::operator+() const noexcept
	{
		return Derived (+value());
	}


template<class V, class T>
	inline constexpr typename Value<V, T>::Derived
	Value<V, T>::operator-() const noexcept
	{
		return Derived (-value());
	}


template<class V, class T>
	inline constexpr bool
	Value<V, T>::operator< (Derived other) const noexcept
	{
		return value() < other.value();
	}


template<class V, class T>
	inline constexpr bool
	Value<V, T>::operator> (Derived other) const noexcept
	{
		return value() > other.value();
	}


template<class V, class T>
	inline constexpr bool
	Value<V, T>::operator<= (Derived other) const noexcept
	{
		return value() <= other.value();
	}


template<class V, class T>
	inline constexpr bool
	Value<V, T>::operator>= (Derived other) const noexcept
	{
		return value() >= other.value();
	}


template<class V, class T>
	inline constexpr bool
	Value<V, T>::operator== (Derived other) const noexcept
	{
		return value() == other.value();
	}


template<class V, class T>
	inline constexpr typename Value<V, T>::Derived
	Value<V, T>::operator+ (Derived other) const noexcept
	{
		return Derived (value() + other.value());
	}


template<class V, class T>
	inline constexpr typename Value<V, T>::Derived
	Value<V, T>::operator- (Derived other) const noexcept
	{
		return Derived (value() - other.value());
	}


template<class V, class T>
	inline constexpr typename Value<V, T>::Derived
	Value<V, T>::operator* (ValueType factor) const noexcept
	{
		return Derived (value() * factor);
	}


template<class V, class T>
	inline constexpr typename Value<V, T>::ValueType
	Value<V, T>::operator/ (Derived other) const noexcept
	{
		return value() / other.value();
	}


template<class V, class T>
	inline constexpr typename Value<V, T>::Derived
	Value<V, T>::operator/ (ValueType factor) const noexcept
	{
		return Derived (value() / factor);
	}


template<class V, class T>
	inline typename Value<V, T>::Derived&
	Value<V, T>::operator+= (Derived other) noexcept
	{
		value() += other.value();
		return *static_cast<Derived*> (this);
	}


template<class V, class T>
	inline typename Value<V, T>::Derived&
	Value<V, T>::operator-= (Derived other) noexcept
	{
		value() -= other.value();
		return *static_cast<Derived*> (this);
	}


template<class V, class T>
	inline typename Value<V, T>::Derived&
	Value<V, T>::operator*= (ValueType factor) noexcept
	{
		value() *= factor;
		return *static_cast<Derived*> (this);
	}


template<class V, class T>
	inline typename Value<V, T>::Derived&
	Value<V, T>::operator/= (ValueType factor) noexcept
	{
		value() /= factor;
		return *static_cast<Derived*> (this);
	}


template<class V, class T>
	inline typename Value<V, T>::ValueType&
	Value<V, T>::value() noexcept
	{
		return _value;
	}


template<class V, class T>
	inline constexpr typename Value<V, T>::ValueType
	Value<V, T>::value() const noexcept
	{
		return _value;
	}


/*
 * Global functions
 */


template<class V, class T>
	inline constexpr typename Value<V, T>::Derived
	operator* (typename Value<V, T>::ValueType a, Value<V, T> const& b) noexcept
	{
		return b * a;
	}

} // namespace SI

#endif


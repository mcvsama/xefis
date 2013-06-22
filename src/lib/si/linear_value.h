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

#ifndef SI__LINEAR_VALUE_H__INCLUDED
#define SI__LINEAR_VALUE_H__INCLUDED

// Standard:
#include <cstddef>
#include <limits>
#include <string>

// Boost:
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

// Xefis:
#include <xefis/config/exception.h>

// Local:
#include "value.h"


namespace SI {

class UnparsableValue: public Xefis::Exception
{
  public:
	UnparsableValue (std::string const& message);
};


class UnsupportedUnit: public Xefis::Exception
{
  public:
	UnsupportedUnit (std::string const& message);
};


/**
 * Implementer should add constructors from specific value types,
 * for example Angle would have constructors that take Degrees and Radians.
 *
 * Use: class Angle: public Value<double, Angle>.
 */
template<class tValueType, class tDerived>
	class LinearValue: public Value
	{
	  public:
		typedef tValueType	ValueType;
		typedef tDerived	Derived;

	  protected:
		constexpr explicit
		LinearValue (ValueType);

	  public:
		constexpr
		LinearValue() noexcept = default;

		explicit constexpr
		LinearValue (LinearValue const&) noexcept = default;

		/**
		 * List supported units.
		 */
		virtual std::vector<std::string> const&
		supported_units() const = 0;

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

		/**
		 * Parse value from string (eg. 1.0 kt).
		 * Return *this.
		 */
		virtual Derived&
		parse (std::string const&) = 0;

		/**
		 * Output string with value and unit.
		 */
		virtual std::string
		stringify() const = 0;

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

	  protected:
		ValueType&
		value() noexcept;

		constexpr ValueType
		value() const noexcept;

	  protected:
		std::pair<ValueType, std::string>
		generic_parse (std::string const&) const;

	  private:
		ValueType _value = ValueType();
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
		_value (value)
	{ }


template<class V, class T>
	inline typename LinearValue<V, T>::ValueType&
	LinearValue<V, T>::internal() noexcept
	{
		return _value;
	}


template<class V, class T>
	inline constexpr typename LinearValue<V, T>::ValueType const&
	LinearValue<V, T>::internal() const noexcept
	{
		return _value;
	}


template<class V, class T>
	inline constexpr typename LinearValue<V, T>::Derived
	LinearValue<V, T>::operator+() const noexcept
	{
		return Derived (+value());
	}


template<class V, class T>
	inline constexpr typename LinearValue<V, T>::Derived
	LinearValue<V, T>::operator-() const noexcept
	{
		return Derived (-value());
	}


template<class V, class T>
	inline constexpr bool
	LinearValue<V, T>::operator< (Derived other) const noexcept
	{
		return value() < other.value();
	}


template<class V, class T>
	inline constexpr bool
	LinearValue<V, T>::operator> (Derived other) const noexcept
	{
		return value() > other.value();
	}


template<class V, class T>
	inline constexpr bool
	LinearValue<V, T>::operator<= (Derived other) const noexcept
	{
		return value() <= other.value();
	}


template<class V, class T>
	inline constexpr bool
	LinearValue<V, T>::operator>= (Derived other) const noexcept
	{
		return value() >= other.value();
	}


template<class V, class T>
	inline constexpr bool
	LinearValue<V, T>::operator== (Derived other) const noexcept
	{
		return value() == other.value();
	}


template<class V, class T>
	inline constexpr bool
	LinearValue<V, T>::operator!= (Derived other) const noexcept
	{
		return value() != other.value();
	}


template<class V, class T>
	inline constexpr typename LinearValue<V, T>::Derived
	LinearValue<V, T>::operator+ (Derived other) const noexcept
	{
		return Derived (value() + other.value());
	}


template<class V, class T>
	inline constexpr typename LinearValue<V, T>::Derived
	LinearValue<V, T>::operator- (Derived other) const noexcept
	{
		return Derived (value() - other.value());
	}


template<class V, class T>
	inline constexpr typename LinearValue<V, T>::Derived
	LinearValue<V, T>::operator* (ValueType factor) const noexcept
	{
		return Derived (value() * factor);
	}


template<class V, class T>
	inline constexpr typename LinearValue<V, T>::ValueType
	LinearValue<V, T>::operator/ (Derived other) const noexcept
	{
		return value() / other.value();
	}


template<class V, class T>
	inline constexpr typename LinearValue<V, T>::Derived
	LinearValue<V, T>::operator/ (ValueType factor) const noexcept
	{
		return Derived (value() / factor);
	}


template<class V, class T>
	inline typename LinearValue<V, T>::Derived&
	LinearValue<V, T>::operator+= (Derived other) noexcept
	{
		value() += other.value();
		return *static_cast<Derived*> (this);
	}


template<class V, class T>
	inline typename LinearValue<V, T>::Derived&
	LinearValue<V, T>::operator-= (Derived other) noexcept
	{
		value() -= other.value();
		return *static_cast<Derived*> (this);
	}


template<class V, class T>
	inline typename LinearValue<V, T>::Derived&
	LinearValue<V, T>::operator*= (ValueType factor) noexcept
	{
		value() *= factor;
		return *static_cast<Derived*> (this);
	}


template<class V, class T>
	inline typename LinearValue<V, T>::Derived&
	LinearValue<V, T>::operator/= (ValueType factor) noexcept
	{
		value() /= factor;
		return *static_cast<Derived*> (this);
	}


template<class V, class T>
	inline typename LinearValue<V, T>::ValueType&
	LinearValue<V, T>::value() noexcept
	{
		return _value;
	}


template<class V, class T>
	inline constexpr typename LinearValue<V, T>::ValueType
	LinearValue<V, T>::value() const noexcept
	{
		return _value;
	}


template<class V, class T>
	inline std::pair<typename LinearValue<V, T>::ValueType, std::string>
	LinearValue<V, T>::generic_parse (std::string const& str) const
	{
		ValueType result = 0.0;
		std::string unit;

		int p = str.find_first_of (' ');
		if (p == -1)
			throw UnparsableValue (std::string ("error while parsing: ") + str);

		try {
			result = boost::lexical_cast<ValueType> (str.substr (0, p));
			unit = boost::to_lower_copy (str.substr (p + 1));
		}
		catch (...)
		{
			throw UnparsableValue (std::string ("error while parsing: ") + str);
		}

		if (std::find (supported_units().begin(), supported_units().end(), unit) == supported_units().end())
			throw UnsupportedUnit (std::string ("error while parsing: ") + str);

		return { result, unit };
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

template<class V, class T>
	std::string to_string (SI::LinearValue<V, T> const& value)
	{
		return value.stringify();
	}

} // namespace std

#endif


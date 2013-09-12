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
#include <string>


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


class Value
{
  public:
	/**
	 * List supported units.
	 */
	virtual std::vector<std::string> const&
	supported_units() const = 0;

	/**
	 * Parse value from string (eg. 1.0 kt).
	 */
	virtual void
	parse (std::string const&) = 0;

	/**
	 * Output string with value and unit.
	 */
	virtual std::string
	stringify() const = 0;

	/**
	 * Return float value in given units.
	 */
	virtual double
	floatize (std::string unit) const = 0;
};


template<class tValueType>
	class TypedValue: public Value
	{
	  public:
		typedef tValueType	ValueType;

	  protected:
		constexpr explicit
		TypedValue (ValueType);

		constexpr
		TypedValue() noexcept = default;

		explicit constexpr
		TypedValue (TypedValue const&) noexcept = default;

	  public:
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

	  protected:
		std::pair<ValueType, std::string>
		generic_parse (std::string const&) const;

	  private:
		ValueType _value = ValueType();
	};


template<class V>
	inline constexpr
	TypedValue<V>::TypedValue (ValueType value):
		_value (value)
	{ }


template<class V>
	inline typename TypedValue<V>::ValueType&
	TypedValue<V>::internal() noexcept
	{
		return _value;
	}


template<class V>
	inline constexpr typename TypedValue<V>::ValueType const&
	TypedValue<V>::internal() const noexcept
	{
		return _value;
	}


template<class V>
	inline std::pair<typename TypedValue<V>::ValueType, std::string>
	TypedValue<V>::generic_parse (std::string const& str) const
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

		if (std::find (this->supported_units().begin(), this->supported_units().end(), unit) == this->supported_units().end())
			throw UnsupportedUnit (std::string ("error while parsing: ") + str);

		return { result, unit };
	}

} // namespace std


namespace std {

template<class V>
	std::string
	to_string (SI::TypedValue<V> const& value)
	{
		return value.stringify();
	}

} // namespace std

#endif


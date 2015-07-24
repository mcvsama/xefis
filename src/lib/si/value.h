/* vim:ts=4
 *
 * Copyleft 2012…2015  Michał Gawron
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
#include <vector>
#include <string>
#include <type_traits>

// Boost:
#include <boost/endian/conversion.hpp>

// Xefis:
#include <xefis/config/exception.h>


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


typedef std::vector<uint8_t> Blob;


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
	 * Parse value from blob.
	 */
	virtual void
	parse_blob (Blob const&) = 0;

	/**
	 * Output string with value and unit.
	 */
	virtual std::string
	stringify() const = 0;

	/**
	 * Output binary blob representing value.
	 */
	virtual Blob
	binarify() const = 0;

	/**
	 * Return float value in given units.
	 */
	virtual double
	floatize (std::string unit) const = 0;
};


static_assert (std::is_literal_type<Value>::value, "Base class Value must be a literal type");


template<class tValueType>
	class TypedValue: public Value
	{
	  public:
		typedef tValueType	ValueType;

	  protected:
		explicit constexpr
		TypedValue (ValueType);

		constexpr
		TypedValue() noexcept = default;

		explicit constexpr
		TypedValue (TypedValue const&) noexcept = default;

	  public:
		virtual ValueType
		si_units() const noexcept = 0;

		/**
		 * Set from SI units.
		 */
		virtual void
		set_si_units (ValueType) = 0;

		// Value API
		void
		parse_blob (Blob const&) override;

		// Value API
		Blob
		binarify() const override;

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
	inline void
	TypedValue<V>::parse_blob (Blob const& blob)
	{
		if (blob.size() == sizeof (ValueType))
		{
			ValueType parsed;
			uint8_t const* begin = blob.data();
			uint8_t const* end = blob.data() + blob.size();
			uint8_t* destination = reinterpret_cast<uint8_t*> (&parsed);
			std::copy (begin, end, destination);
			boost::endian::little_to_native (parsed);
			internal() = parsed;
		}
		else
			throw UnparsableValue ("wrong size of binary data");
	}


template<class V>
	inline Blob
	TypedValue<V>::binarify() const
	{
		Blob result (sizeof (ValueType));
		ValueType copy = internal();
		boost::endian::native_to_little (copy);
		uint8_t const* begin = reinterpret_cast<uint8_t const*> (&copy);
		uint8_t const* end = begin + sizeof (ValueType);
		uint8_t* destination = &*result.begin();
		std::copy (begin, end, destination);
		return result;
	}


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

		std::size_t p = str.find_first_of (' ');
		if (p == std::string::npos)
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


template<class V>
	std::string
	to_string (SI::TypedValue<V> const& value)
	{
		return value.stringify();
	}

} // namespace SI

#endif


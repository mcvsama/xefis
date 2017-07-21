/* vim:ts=4
 *
 * Copyleft 2012…2016  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__CORE__V2__PROPERTY_DIGITIZER_H__INCLUDED
#define XEFIS__CORE__V2__PROPERTY_DIGITIZER_H__INCLUDED

// Standard:
#include <cstddef>
#include <optional>
#include <type_traits>

// Lib:
#include <boost/lexical_cast.hpp>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/property.h>
#include <xefis/core/v2/property_utils.h>


namespace v2 {

/**
 * Returns numerical value for a Property.
 */
class PropertyDigitizer
{
	static constexpr const char kDefaultNilValue[] = u8"∅";

  public:
	/**
	 * Converts value of a property from and to std::string.
	 */
	class BasicConverter
	{
	  public:
		virtual std::optional<double>
		to_numeric() const = 0;

		virtual void
		from_numeric (std::optional<double>) = 0;
	};

	template<class Value>
		class ArithmeticConverter: public BasicConverter
		{
		  public:
			// Ctor
			explicit
			ArithmeticConverter (Property<Value>& property):
				_property (property)
			{ }

			std::optional<double>
			to_numeric() const override
			{
				return _property.get_optional();
			}

			void
			from_numeric (std::optional<double> value) override
			{
				_property = value;
			}

		  private:
			Property<Value>& _property;
		};

	template<class Quantity, class DesiredUnit>
		class SIQuantityConverter: public BasicConverter
		{
		  public:
			// Ctor
			explicit
			SIQuantityConverter (Property<Quantity>& property):
				_property (property)
			{ }

			std::optional<double>
			to_numeric() const override
			{
				if (_property)
					return _property->template quantity<DesiredUnit>();
				else
					return std::nullopt;
			}

			void
			from_numeric (std::optional<double> value) override
			{
				if (value)
					_property = DesiredUnit (value);
				else
					_property = {};
			}

		  private:
			Property<Quantity>& _property;
		};

  public:
	// Ctor
	template<class Value,
			 class = std::enable_if_t<std::is_arithmetic<Value>::value>>
		explicit
		PropertyDigitizer (Property<Value>&);

	// Ctor
	template<class Quantity, class DesiredUnit,
			 class = std::enable_if_t<is_quantity<Quantity>::value>>
		explicit
		PropertyDigitizer (Property<Quantity>&);

	// Ctor
	PropertyDigitizer (PropertyDigitizer const& other) = default;

	PropertyDigitizer const&
	operator= (PropertyDigitizer const& other);

	/**
	 * Return property reference.
	 */
	BasicProperty&
	property() noexcept;

	/**
	 * Return property reference.
	 */
	BasicProperty const&
	property() const noexcept;

	/**
	 * Return the converter object.
	 */
	BasicConverter const&
	converter() const noexcept;

	/**
	 * Convert value to string.
	 */
	std::optional<double>
	to_numeric() const;

	/**
	 * Parse string and set property value.
	 */
	void
	from_numeric (std::optional<double>);

  private:
	Shared<BasicConverter>	_converter;
	BasicProperty&			_property;
};


template<class Value,
		 class = std::enable_if_t<std::is_arithmetic<Value>::value>>
	inline
	PropertyDigitizer::PropertyDigitizer (Property<Value>& property):
		_converter (std::make_shared<ArithmeticConverter<Value>> (property)),
		_property (property)
	{ }


template<class Quantity, class DesiredUnit,
		 class = std::enable_if_t<is_quantity<Quantity>::value>>
	inline
	PropertyDigitizer::PropertyDigitizer (Property<Quantity>& property):
		_converter (std::make_shared<SIQuantityConverter<Quantity, DesiredUnit>> (property)),
		_property (property)
{ }


inline BasicProperty&
PropertyDigitizer::property() noexcept
{
	return _property;
}


inline BasicProperty const&
PropertyDigitizer::property() const noexcept
{
	return _property;
}


inline PropertyDigitizer::BasicConverter const&
PropertyDigitizer::converter() const noexcept
{
	return *_converter;
}


inline std::optional<double>
PropertyDigitizer::to_numeric() const
{
	return _converter->to_numeric();
}


inline void
PropertyDigitizer::from_numeric (std::optional<double> v)
{
	_converter->from_numeric (v);
}


inline PropertyDigitizer const&
PropertyDigitizer::operator= (PropertyDigitizer const& other)
{
	_converter = other._converter;
	return *this;
}

} // namespace v2

#endif


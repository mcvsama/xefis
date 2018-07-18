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

#ifndef XEFIS__CORE__PROPERTY_STRING_CONVERTER_H__INCLUDED
#define XEFIS__CORE__PROPERTY_STRING_CONVERTER_H__INCLUDED

// Standard:
#include <cstddef>

// Lib:
#include <boost/lexical_cast.hpp>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/property_converter.h>


namespace xf {

class BasicProperty;

template<class Value>
	class Property;


/**
 * Formats Property value according to configuration.
 * Specifically extracts numeric values in configured units from Properties holding SI values.
 */
class PropertyStringConverter
{
  public:
	/**
	 * Converts value of a property from and to std::string.
	 */
	class BasicConverter
	{
	  public:
		// Ctor
		explicit
		BasicConverter (PropertyConversionSettings const&);

		// Dtor
		virtual
		~BasicConverter() = default;

		void
		set_settings (PropertyConversionSettings const&);

		PropertyConversionSettings const&
		settings() const noexcept;

		virtual std::string
		to_string() const = 0;

		virtual void
		from_string (std::string const&) = 0;

	  private:
		PropertyConversionSettings _settings;
	};

	class StringConverter: public BasicConverter
	{
	  public:
		// Ctor
		explicit
		StringConverter (Property<std::string>& property, PropertyConversionSettings const&);

		std::string
		to_string() const override;

		void
		from_string (std::string const&) override;

	  private:
		Property<std::string>& _property;
	};

	class BoolConverter: public BasicConverter
	{
	  public:
		// Ctor
		explicit
		BoolConverter (Property<bool>& property, PropertyConversionSettings const&);

		std::string
		to_string() const override;

		void
		from_string (std::string const&) override;

	  private:
		Property<bool>& _property;
	};

	template<class Value>
		class FormatConverter: public BasicConverter
		{
		  public:
			// Ctor
			explicit
			FormatConverter (Property<Value>& property, PropertyConversionSettings const&);

			std::string
			to_string() const override;

			void
			from_string (std::string const&) override;

		  private:
			Property<Value>& _property;
		};

	template<class Quantity>
		class SIQuantityConverter: public BasicConverter
		{
		  public:
			// Ctor
			explicit
			SIQuantityConverter (Property<Quantity>& property, PropertyConversionSettings const&);

			std::string
			to_string() const override;

			void
			from_string (std::string const&) override;

		  private:
			Property<Quantity>& _property;
		};

	template<class Value>
		class GenericToStringConverter: public BasicConverter
		{
		  public:
			// Ctor
			explicit
			GenericToStringConverter (Property<Value>& property, PropertyConversionSettings const&);

			std::string
			to_string() const override;

			void
			from_string (std::string const&) override;

		  private:
			Property<Value>& _property;
		};

  private:
	/**
	 * Select SIQuantityConverter or GenericToStringConverter depending on Value type.
	 */
	template<class Value>
		using SIOrGenericConverter = std::conditional_t<si::is_quantity_v<Value>, SIQuantityConverter<Value>, GenericToStringConverter<Value>>;

  public:
	// Ctor
	explicit
	PropertyStringConverter (Property<std::string>&, PropertyConversionSettings const& = {});

	// Ctor
	explicit
	PropertyStringConverter (Property<bool>&, PropertyConversionSettings const& = {});

	// Ctor
	explicit
	PropertyStringConverter (Property<int64_t>&, PropertyConversionSettings const& = {});

	// Ctor
	explicit
	PropertyStringConverter (Property<double>&, PropertyConversionSettings const& = {});

	// Ctor
	template<class Value>
		explicit
		PropertyStringConverter (Property<Value>&, PropertyConversionSettings const& = {});

	// Ctor
	PropertyStringConverter (PropertyStringConverter const& other) = default;

	PropertyStringConverter const&
	operator= (PropertyStringConverter const& other);

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
	std::string
	to_string() const;

	/**
	 * Parse string and set property value.
	 */
	void
	from_string (std::string const&);

  private:
	Shared<BasicConverter>	_converter;
	BasicProperty&			_property;
};

} // namespace xf


#include "property_string_converter.tcc"

#endif


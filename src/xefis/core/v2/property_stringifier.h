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

#ifndef XEFIS__CORE__V2__PROPERTY_STRINGIFIER_H__INCLUDED
#define XEFIS__CORE__V2__PROPERTY_STRINGIFIER_H__INCLUDED

// Standard:
#include <cstddef>

// Lib:
#include <boost/lexical_cast.hpp>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/property.h>
#include <xefis/core/v2/property_utils.h>


namespace v2 {

/**
 * Formats Property value according to configuration.
 * Specifically extracts numeric values in configured units from Properties holding SI values.
 */
class PropertyStringifier
{
	static constexpr const char kDefaultNilValue[] = u8"∅";

  public:
	/**
	 * Converts value of a property from and to std::string.
	 */
	class BasicConverter
	{
	  public:
		virtual std::string
		to_string() const = 0;

		virtual void
		from_string (std::string const&) = 0;
	};

	class StringConverter: public BasicConverter
	{
	  public:
		// Ctor
		explicit
		StringConverter (Property<std::string>& property, std::string const& nil_value):
			_property (property),
			_nil_value (nil_value)
		{ }

		std::string
		to_string() const override
		{
			return _property ? *_property : _nil_value;
		}

		void
		from_string (std::string const& s) override
		{
			_property = s;
		}

	  private:
		Property<std::string>&	_property;
		std::string				_nil_value;
	};

	class BoolConverter: public BasicConverter
	{
	  public:
		// Ctor
		explicit
		BoolConverter (Property<bool>& property, std::string const& true_value, std::string const& false_value, std::string const& nil_value):
			_property (property),
			_true_value (true_value),
			_false_value (false_value),
			_nil_value (nil_value)
		{ }

		std::string
		to_string() const override
		{
			if (_property)
				return *_property ? _true_value : _false_value;
			else
				return _nil_value;
		}

		void
		from_string (std::string const& s) override
		{
			_property = s == _true_value;
		}

		std::string const&
		true_value() const noexcept
		{
			return _true_value;
		}

		std::string const&
		false_value() const noexcept
		{
			return _false_value;
		}

		std::string const&
		nil_value() const noexcept
		{
			return _nil_value;
		}

	  private:
		Property<bool>&	_property;
		std::string		_true_value;
		std::string		_false_value;
		std::string		_nil_value;
	};

	template<class Value>
		class FormatConverter: public BasicConverter
		{
		  public:
			// Ctor
			explicit
			FormatConverter (Property<Value>& property, boost::format const& format, std::string const& nil_value):
				_property (property),
				_format (format),
				_nil_value (nil_value)
			{ }

			std::string
			to_string() const override
			{
				auto format = _format;

				if (_property)
					return (format % *_property).str();
				else
					return _nil_value;
			}

			void
			from_string (std::string const& s) override
			{
				_property = boost::lexical_cast<Value> (s);
			}

		  private:
			Property<Value>&	_property;
			boost::format		_format;
			std::string			_nil_value;
		};

	template<class Quantity, class DesiredUnit>
		class SIQuantityConverter: public BasicConverter
		{
		  public:
			// Ctor
			explicit
			SIQuantityConverter (Property<Quantity>& property, boost::format const& format, std::string const& nil_value):
				_property (property),
				_format (format),
				_nil_value (nil_value)
			{ }

			std::string
			to_string() const override
			{
				using si::to_string;

				auto format = _format;

				if (_property)
					return (format % to_string (si::Quantity<DesiredUnit> (*_property))).str();
				else
					return _nil_value;
			}

			void
			from_string (std::string const& s) override
			{
				si::parse (s, _property);
			}

		  private:
			Property<Quantity>&			_property;
			boost::format				_format;
			std::string					_nil_value;
		};

  public:
	// Ctor
	// TODO add option { WithUnit, NoUnit }
	explicit
	PropertyStringifier (Property<std::string>&, std::string const& nil_value = kDefaultNilValue);

	// Ctor
	explicit
	PropertyStringifier (Property<bool>&, std::string const& true_value = "true", std::string const& false_value = "false", std::string const& nil_value = kDefaultNilValue);

	// Ctor
	explicit
	PropertyStringifier (Property<int64_t>&, boost::format const&, std::string const& nil_value = kDefaultNilValue);

	// Ctor
	explicit
	PropertyStringifier (Property<double>&, boost::format const&, std::string const& nil_value = kDefaultNilValue);

	// Ctor
	template<class Quantity, class DesiredUnit,
			 class = std::enable_if_t<is_quantity<Quantity>::value>>
		explicit
		PropertyStringifier (Property<Quantity>&, boost::format const&, std::string const& nil_value = kDefaultNilValue);

	// Ctor
	PropertyStringifier (PropertyStringifier const& other) = default;

	PropertyStringifier const&
	operator= (PropertyStringifier const& other);

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


inline
PropertyStringifier::PropertyStringifier (Property<std::string>& property, std::string const& nil_value):
	_converter (std::make_shared<StringConverter> (property, nil_value)),
	_property (property)
{ }


inline
PropertyStringifier::PropertyStringifier (Property<bool>& property, std::string const& true_value, std::string const& false_value, std::string const& nil_value):
	_converter (std::make_shared<BoolConverter> (property, true_value, false_value, nil_value)),
	_property (property)
{ }


inline
PropertyStringifier::PropertyStringifier (Property<int64_t>& property, boost::format const& format, std::string const& nil_value):
	_converter (std::make_shared<FormatConverter<int64_t>> (property, format, nil_value)),
	_property (property)
{ }


inline
PropertyStringifier::PropertyStringifier (Property<double>& property, boost::format const& format, std::string const& nil_value):
	_converter (std::make_shared<FormatConverter<double>> (property, format, nil_value)),
	_property (property)
{ }


template<class Quantity, class DesiredUnit,
		 class = std::enable_if_t<is_quantity<Quantity>::value>>
	inline
	PropertyStringifier::PropertyStringifier (Property<Quantity>& property, boost::format const& format, std::string const& nil_value):
		_converter (std::make_shared<SIQuantityConverter<Quantity, DesiredUnit>> (property, format, nil_value)),
		_property (property)
{ }


inline BasicProperty&
PropertyStringifier::property() noexcept
{
	return _property;
}


inline BasicProperty const&
PropertyStringifier::property() const noexcept
{
	return _property;
}


inline PropertyStringifier::BasicConverter const&
PropertyStringifier::converter() const noexcept
{
	return *_converter;
}


inline std::string
PropertyStringifier::to_string() const
{
	return _converter->to_string();
}


inline void
PropertyStringifier::from_string (std::string const& s)
{
	_converter->from_string (s);
}


inline PropertyStringifier const&
PropertyStringifier::operator= (PropertyStringifier const& other)
{
	_converter = other._converter;
	return *this;
}

} // namespace v2

#endif


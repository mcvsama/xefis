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

#ifndef XEFIS__CORE__PROPERTY_STRING_CONVERTER_TCC__INCLUDED
#define XEFIS__CORE__PROPERTY_STRING_CONVERTER_TCC__INCLUDED

// Standard:
#include <cstddef>

// Boost:
#include <boost/lexical_cast.hpp>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/property_utils.h>


namespace xf {

/**
 * This enables-back ADL on to_string() function from within PropertyStringConverter,
 * which also has to_string() method. ADL doesn't work from within there since the method
 * name is the same.
 */
template<class T>
	inline std::string
	to_string_proxy (T&& t)
	{
		using std::to_string;

		return std::string (to_string (std::forward<T> (t)));
	}


/**
 * Parse fundamental types.
 */
template<class T,
		 class = std::enable_if_t<std::is_arithmetic_v<T>>>
	inline void
	parse (std::string_view const& str, T& result)
	{
		result = boost::lexical_cast<T> (str);
	}


inline
PropertyStringConverter::BasicConverter::BasicConverter (PropertyConversionSettings const& settings):
	_settings (settings)
{ }


inline void
PropertyStringConverter::BasicConverter::set_settings (PropertyConversionSettings const& settings)
{
	_settings = settings;
}


inline PropertyConversionSettings const&
PropertyStringConverter::BasicConverter::settings() const noexcept
{
	return _settings;
}


template<class V>
	inline
	PropertyStringConverter::FormatConverter<V>::FormatConverter (Property<V>& property, PropertyConversionSettings const& settings):
		BasicConverter (settings),
		_property (property)
	{ }


template<class V>
	inline std::string
	PropertyStringConverter::FormatConverter<V>::to_string() const
	{
		if (_property)
			return (boost::format (settings().numeric_format) % *_property).str();
		else
			return settings().nil_value;
	}


template<class V>
	inline void
	PropertyStringConverter::FormatConverter<V>::from_string (std::string const& /*s*/)
	{
		// TODO _property << ConstantSource (boost::lexical_cast<Value> (s));
	}


template<class Q>
	inline
	PropertyStringConverter::SIQuantityConverter<Q>::SIQuantityConverter (Property<Q>& property, PropertyConversionSettings const& settings):
		BasicConverter (settings),
		_property (property)
	{ }


template<class Q>
	inline std::string
	PropertyStringConverter::SIQuantityConverter<Q>::to_string() const
	{
		using si::to_string;

		if (_property)
			return (boost::format (settings().numeric_format) % to_string (*_property)).str();
		else
			return settings().nil_value;
	}


template<class Q>
	inline void
	PropertyStringConverter::SIQuantityConverter<Q>::from_string (std::string const& /*s*/)
	{
		//si::parse (s, _property);
	}


template<class Value>
	inline
	PropertyStringConverter::GenericToStringConverter<Value>::GenericToStringConverter (Property<Value>& property, PropertyConversionSettings const& settings):
		BasicConverter (settings),
		_property (property)
	{ }


template<class Value>
	inline std::string
	PropertyStringConverter::GenericToStringConverter<Value>::to_string() const
	{
		if (_property)
			return to_string_proxy (*_property);
		else
			return settings().nil_value;
	}


template<class Value>
	inline void
	PropertyStringConverter::GenericToStringConverter<Value>::from_string (std::string const& str)
	{
		Value value;
		parse (str, value);
		// TODO _property << ConstantSource (value);
	}


inline
PropertyStringConverter::PropertyStringConverter (Property<std::string>& property, PropertyConversionSettings const& settings):
	_converter (std::make_shared<StringConverter> (property, settings)),
	_property (property)
{ }


inline
PropertyStringConverter::PropertyStringConverter (Property<bool>& property, PropertyConversionSettings const& settings):
	_converter (std::make_shared<BoolConverter> (property, settings)),
	_property (property)
{ }


inline
PropertyStringConverter::PropertyStringConverter (Property<int64_t>& property, PropertyConversionSettings const& settings):
	_converter (std::make_shared<FormatConverter<int64_t>> (property, settings)),
	_property (property)
{ }


inline
PropertyStringConverter::PropertyStringConverter (Property<double>& property, PropertyConversionSettings const& settings):
	_converter (std::make_shared<FormatConverter<double>> (property, settings)),
	_property (property)
{ }


template<class Value>
	inline
	PropertyStringConverter::PropertyStringConverter (Property<Value>& property, PropertyConversionSettings const& settings):
		_converter (std::make_shared<SIOrGenericConverter<Value>> (property, settings)),
		_property (property)
	{ }


inline BasicProperty&
PropertyStringConverter::property() noexcept
{
	return _property;
}


inline BasicProperty const&
PropertyStringConverter::property() const noexcept
{
	return _property;
}


inline PropertyStringConverter::BasicConverter const&
PropertyStringConverter::converter() const noexcept
{
	return *_converter;
}


inline std::string
PropertyStringConverter::to_string() const
{
	return _converter->to_string();
}


inline void
PropertyStringConverter::from_string (std::string const& s)
{
	_converter->from_string (s);
}


inline PropertyStringConverter const&
PropertyStringConverter::operator= (PropertyStringConverter const& other)
{
	_converter = other._converter;
	return *this;
}

} // namespace xf

#endif


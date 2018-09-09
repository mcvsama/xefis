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

#ifndef XEFIS__CORE__PROPERTY_TCC__INCLUDED
#define XEFIS__CORE__PROPERTY_TCC__INCLUDED

// Standard:
#include <cstddef>
#include <cstdint>
#include <optional>
#include <variant>


namespace xf {

/*
 * NilProperty
 */


inline
NilProperty::NilProperty (PropertyPath const& path):
	Exception ("tried to read a nil property " + path.string())
{ }


/*
 * BasicProperty
 */


inline
BasicProperty::BasicProperty (std::string_view const& path):
	_path (path)
{ }


inline
BasicProperty::BasicProperty (ModuleIO* owner, std::string_view const& path):
	_owner (owner),
	_path (path)
{ }


inline ModuleIO*
BasicProperty::io() const noexcept
{
	return _owner;
}


inline si::Time
BasicProperty::modification_timestamp() const noexcept
{
	return _modification_timestamp;
}


inline si::Time
BasicProperty::modification_age() const noexcept
{
	return TimeHelper::now() - modification_timestamp();
}


inline si::Time
BasicProperty::valid_timestamp() const noexcept
{
	return _valid_timestamp;
}


inline si::Time
BasicProperty::valid_age() const noexcept
{
	return TimeHelper::now() - valid_timestamp();
}


inline PropertyPath const&
BasicProperty::path() const noexcept
{
	return _path;
}


inline BasicProperty::Serial
BasicProperty::serial() const noexcept
{
	return _serial;
}


/*
 * Property
 */


template<class V>
	inline typename Property<V>::Value const&
	Property<V>::get() const
	{
		if (_value)
			return *_value;
		else if (_fallback_value)
			return *_fallback_value;
		else
			throw NilProperty (path());
	}


template<class V>
	inline typename Property<V>::Value const&
	Property<V>::operator*() const
	{
		return get();
	}


template<class V>
	inline std::optional<typename Property<V>::Value>
	Property<V>::get_optional() const
	{
		if (_value)
			return _value;
		else
			return _fallback_value;
	}


template<class V>
	inline typename Property<V>::Value
	Property<V>::value_or (Value fallback) const
	{
		if (_value)
			return *_value;
		else if (_fallback_value)
			return *_fallback_value;
		else
			return fallback;
	}


template<class V>
	inline typename Property<V>::Value const*
	Property<V>::operator->() const
	{
		return &get();
	}


template<class V>
	inline void
	Property<V>::set_fallback (std::optional<Value> fallback_value)
	{
		if (_fallback_value != fallback_value)
		{
			_modification_timestamp = TimeHelper::now();
			_valid_timestamp = _modification_timestamp;
			_fallback_value = fallback_value;
			++_serial;
		}
	}


template<class V>
	inline bool
	Property<V>::is_nil() const noexcept
	{
		return !_value && !_fallback_value;
	}


template<class V>
	inline bool
	Property<V>::valid() const noexcept
	{
		return !is_nil();
	}


template<class V>
	inline void
	Property<V>::protected_set_value (Value value)
	{
		if (!_value || *_value != value)
		{
			_modification_timestamp = TimeHelper::now();
			_valid_timestamp = _modification_timestamp;
			_value = value;
			++_serial;
		}
	}


template<class V>
	inline void
	Property<V>::protected_set (std::optional<Value> value)
	{
		if (value)
			protected_set_value (*value);
		else
			protected_set_nil();
	}


template<class V>
	inline void
	Property<V>::protected_set (Property<Value> const& value)
	{
		if (value)
			protected_set_value (*value);
		else
			protected_set_nil();
	}


template<class V>
	inline void
	Property<V>::protected_set_nil()
	{
		if (_value)
		{
			_modification_timestamp = TimeHelper::now();
			_value.reset();
			++_serial;
		}
	}

} // namespace xf

#endif


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

#ifndef XEFIS__CORE__V2__SETTING_H__INCLUDED
#define XEFIS__CORE__V2__SETTING_H__INCLUDED

// Standard:
#include <cstddef>
#include <cstdint>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/property_utils.h>
#include <xefis/core/v2/module.h>
#include <xefis/utility/time.h>
#include <xefis/utility/time_helper.h>
#include <xefis/utility/blob.h>


namespace v2 {

class Module;


/**
 * Common base class for all Setting<*> types.
 */
class BasicSetting
{
  public:
	/**
	 * Tag for creating a setting that doesn't not need to be set by user explicitly, but doesn't necessarily
	 * have any default value.
	 */
	enum OptionalTag { Optional };

  public:
	/**
	 * Store pointer to owning module.
	 */
	explicit
	BasicSetting (Module* owner);

	/**
	 * Return owning module.
	 */
	Module*
	owner() const noexcept;

	/**
	 * Return true if setting has a value.
	 */
	virtual
	operator bool() const noexcept = 0;

  private:
	Module* _owner;
};


/**
 * Wrapper for setting variables.
 * Allows run-timer checking if required settings have been configured.
 */
template<class pValue>
	class Setting: public BasicSetting
	{
	  public:
		typedef pValue Value;

	  public:
		/**
		 * Create a setting object that requires explicit setting of a value.
		 */
		explicit
		Setting (Module* owner);

		/**
		 * Creates a setting object that has an initial value.
		 */
		explicit
		Setting (Module* owner, Value&& initial_value);

		/**
		 * Creates a setting that doesn't have and doesn't require any value.
		 */
		explicit
		Setting (Module* owner, OptionalTag);

		/**
		 * Copy-assignment operator.
		 */
		Setting const&
		operator= (Value&& new_value);

		/**
		 * Return true if setting is required to have a value.
		 */
		bool
		required() const noexcept;

		/**
		 * Read the setting value.
		 */
		Value&
		operator*() noexcept;

		/**
		 * Read the setting value.
		 */
		Value const&
		operator*() const noexcept;

		/**
		 * Read the setting value.
		 */
		Value const*
		operator->() const noexcept;

		// BasicSetting API
		operator bool() const noexcept override;

	  private:
		::Optional<Value>	_value;
		bool				_required;
	};


inline BasicSetting::BasicSetting (Module* owner):
	_owner (owner)
{ }


inline Module*
BasicSetting::owner() const noexcept
{
	return _owner;
}


template<class V>
	inline
	Setting<V>::Setting (Module* owner):
		BasicSetting (owner)
	{ }


template<class V>
	inline
	Setting<V>::Setting (Module* owner, Value&& initial_value):
		BasicSetting (owner),
		_value (std::forward<Value> (initial_value)),
		_required (true)
	{ }


template<class V>
	inline
	Setting<V>::Setting (Module* owner, OptionalTag):
		BasicSetting (owner),
		_required (false)
	{ }


template<class V>
	inline Setting<V> const&
	Setting<V>::operator= (Value&& value)
	{
		_value = std::forward<Value> (value);
		return *this;
	}


template<class V>
	inline bool
	Setting<V>::required() const noexcept
	{
		return _required;
	}


template<class V>
	inline auto
	Setting<V>::operator*() noexcept -> Value&
	{
		return *_value;
	}


template<class V>
	inline auto
	Setting<V>::operator*() const noexcept -> Value const&
	{
		return *_value;
	}


template<class V>
	inline auto
	Setting<V>::operator->() const noexcept -> Value const*
	{
		return &_value.get();
	}


template<class V>
	inline
	Setting<V>::operator bool() const noexcept
	{
		return !!_value;
	}

} // namespace v2

#endif


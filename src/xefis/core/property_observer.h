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

#ifndef XEFIS__CORE__PROPERTY_OBSERVER_H__INCLUDED
#define XEFIS__CORE__PROPERTY_OBSERVER_H__INCLUDED

// Standard:
#include <cstddef>
#include <functional>
#include <list>

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "property.h"


namespace Xefis {

/**
 * Observes a set of properties, and checks if their values have changed.
 * If they did, calls registered callback function.
 */
class PropertyObserver
{
  public:
	class Object
	{
		friend class PropertyObserver;

	  public:
		// Ctor:
		Object (GenericProperty*);

		// Ctor:
		Object (PropertyObserver*);

		PropertyNode::Serial
		remote_serial() const noexcept;

	  private:
		GenericProperty*		_property		= nullptr;
		PropertyObserver*		_observer		= nullptr;
		PropertyNode::Serial	_saved_serial	= 0;
	};

	typedef std::list<Object>		ObjectsList;

  public:
	typedef PropertyNode::Serial	Serial;
	typedef std::function<void()>	Callback;

  public:
	/**
	 * Add property to be tracked.
	 * Property is added by reference, so the property object
	 * must live as long as the PropertyObserver.
	 */
	void
	observe (GenericProperty& property);

	/**
	 * Add another PropertyObserver to observe.
	 */
	void
	observe (PropertyObserver& observer);

	/**
	 * Add list of properties to be tracked.
	 */
	void
	observe (std::initializer_list<Object> list);

	/**
	 * Setup callback function.
	 */
	void
	set_callback (Callback callback);

	/**
	 * Signal data update, so the observer will do
	 * its checks.
	 */
	void
	data_updated (Time update_time);

	/**
	 * Return serial value.
	 * It's incremented every time the callback function
	 * is called.
	 */
	Serial
	serial() const noexcept;

	/**
	 * Return last update time.
	 */
	Time
	update_time() const noexcept;

	/**
	 * Return time delta since last update and
	 * call to the callback function.
	 */
	Time
	update_dt() const noexcept;

  private:
	ObjectsList		_objects;
	Callback		_callback;
	Serial			_serial			= 0;
	Time			_update_time	= 0_s;
	Time			_update_dt		= 0_s;
};


inline
PropertyObserver::Object::Object (GenericProperty* property):
	_property (property)
{ }


inline
PropertyObserver::Object::Object (PropertyObserver* observer):
	_observer (observer)
{ }


inline PropertyNode::Serial
PropertyObserver::Object::remote_serial() const noexcept
{
	if (_property)
		return _property->serial();
	return _observer->serial();
}


inline PropertyObserver::Serial
PropertyObserver::serial() const noexcept
{
	return _serial;
}


inline Time
PropertyObserver::update_time() const noexcept
{
	return _update_time;
}


inline Time
PropertyObserver::update_dt() const noexcept
{
	return _update_dt;
}

} // namespace Xefis

#endif


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

#ifndef XEFIS__CORE__V2__PROPERTY_OBSERVER_H__INCLUDED
#define XEFIS__CORE__V2__PROPERTY_OBSERVER_H__INCLUDED

// Standard:
#include <cstddef>
#include <functional>
#include <list>

// Boost:
#include <boost/variant.hpp>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/property.h>
#include <xefis/utility/smoother.h>


namespace v2 {

/**
 * Observes a set of properties, and checks if their values have changed.
 * If they did, calls registered callback function.
 */
class PropertyObserver
{
  public:
	/**
	 * Encapsulates object to be observed: a property or another observer.
	 */
	class Object
	{
		friend class PropertyObserver;

	  public:
		// Ctor
		Object (BasicProperty*);

		// Ctor
		Object (PropertyObserver*);

		PropertyNode::Serial
		remote_serial() const noexcept;

	  private:
		boost::variant<BasicProperty*, PropertyObserver*>	_observable;
		PropertyNode::Serial								_saved_serial	= 0;
	};

	typedef std::list<Object>			ObjectsList;
	typedef std::list<SmootherBase*>	SmoothersList;

  public:
	typedef PropertyNode::Serial		Serial;
	typedef std::function<void()>		Callback;

  public:
	/**
	 * Add property to be observed.
	 * When property's value changes (that is the fresh() method returns true), the callback function is called.
	 *
	 * Property is held by reference, so the property object must live as long as the PropertyObserver.
	 */
	void
	observe (BasicProperty& property);

	/**
	 * Add another PropertyObserver to observe.
	 * Similarly to observing property, if the other observer fires its callback function, then this observer
	 * will fire its own.
	 *
	 * The other observer is held by reference, and it must live as long as this observer lives.
	 */
	void
	observe (PropertyObserver& observer);

	/**
	 * Add list of properties to be tracked.
	 * Same as calling observe (GenericProperty&) for each of the objects in list, in sequence.
	 */
	void
	observe (std::initializer_list<Object> list);

	/**
	 * Setup callback function.
	 * This function will be called when one of observed properties is changed or observers is fired.
	 */
	void
	set_callback (Callback callback) noexcept;

	/**
	 * Set minimum time-delta accumulation before firing the callback function.
	 * To avoid aliasing, it's good to make sure that the observed data doesn't contain high-frequency value changes.
	 * Default is 0_s.
	 */
	void
	set_minimum_dt (Time) noexcept;

	/**
	 * Signal data update, so the observer will do its checks.
	 */
	void
	process (si::Time update_time);

	/**
	 * Return serial value.
	 * It's incremented every time the callback function is called.
	 */
	Serial
	serial() const noexcept;

	/**
	 * Return last update time.
	 * This is the time of the last fire of the callback function.
	 */
	Time
	update_time() const noexcept;

	/**
	 * Return time delta since last fire of the callback function.
	 */
	Time
	update_dt() const noexcept;

	/**
	 * Register smoother with this observer.
	 * Several smoothers can be registered. The longest smoothing time from those smoothers is collected every
	 * time this Observer is updated (process(...) is called). Then, for that period of time, observer
	 * will fire callback function several times. Frequency of fires is controlled by set_smoothing_frequency(...).
	 *
	 * Smoother is held by reference, so it must live as long as this object lives.
	 */
	void
	add_depending_smoother (SmootherBase& smoother);

	/**
	 * Register smoothers with this overver.
	 * Convenience method.
	 */
	void
	add_depending_smoothers (std::initializer_list<SmootherBase*> list);

	/**
	 * Tells the property observer to do a callback on next occasion, regardless of other conditions, but takes into
	 * consideration minimum dt set with set_minimum_dt().
	 */
	void
	touch() noexcept;

  private:
	/**
	 * Find longest smoothing time from all registered smoothers.
	 * Return 0_s, if no smoothers are registered.
	 */
	Time
	longest_smoothing_time() noexcept;

  private:
	ObjectsList		_objects;
	SmoothersList	_smoothers;
	Callback		_callback;
	Serial			_serial						= 0;
	// Time of last change of observed property:
	Time			_obs_update_time			= 0_s;
	// Time of last firing of the callback function:
	Time			_fire_time					= 0_s;
	Time			_fire_dt					= 0_s;
	Time			_accumulated_dt				= 0_s;
	Time			_minimum_dt					= 0_s;
	Time			_longest_smoother			= 0_s;
	bool			_recompute_longest_smoother	= false;
	// Set to true, when observed property is updated, but
	// _minimum_dt prevented firing the callback.
	bool			_need_callback				= false;
	bool			_last_recompute				= false;
	bool			_touch						= false;
};


inline
PropertyObserver::Object::Object (BasicProperty* property):
	_observable (property)
{ }


inline
PropertyObserver::Object::Object (PropertyObserver* observer):
	_observable (observer)
{ }


inline PropertyNode::Serial
PropertyObserver::Object::remote_serial() const noexcept
{
	struct SerialGetter: public boost::static_visitor<BasicProperty::Serial>
	{
		BasicProperty::Serial
		operator() (BasicProperty* property) const
		{
			return property->serial();
		}

		BasicProperty::Serial
		operator() (PropertyObserver* observer) const
		{
			return observer->serial();
		}
	};

	return boost::apply_visitor (SerialGetter(), _observable);
}


inline PropertyObserver::Serial
PropertyObserver::serial() const noexcept
{
	return _serial;
}


inline Time
PropertyObserver::update_time() const noexcept
{
	return _fire_time;
}


inline Time
PropertyObserver::update_dt() const noexcept
{
	return _fire_dt;
}


inline void
PropertyObserver::touch() noexcept
{
	_touch = true;
}

} // namespace v2

#endif


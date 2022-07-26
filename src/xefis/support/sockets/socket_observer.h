/* vim:ts=4
 *
 * Copyleft 2021  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__SOCKETS__MODULE_SOCKET_OBSERVER_H__INCLUDED
#define XEFIS__SUPPORT__SOCKETS__MODULE_SOCKET_OBSERVER_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/sockets/basic_socket.h>
#include <xefis/utility/smoother.h>

// Neutrino:
#include <neutrino/variant.h>

// Standard:
#include <cstddef>
#include <functional>
#include <variant>
#include <list>


namespace xf {

/**
 * Observes a set of sockets, and checks if their values have changed.
 * If they did, calls registered callback function.
 */
class SocketObserver
{
  public:
	/**
	 * Encapsulates object to be observed: a module socket or another observer.
	 */
	class Object
	{
		friend class SocketObserver;

	  public:
		// Ctor
		Object (BasicSocket const*);

		// Ctor
		Object (SocketObserver*);

		BasicSocket::Serial
		remote_serial() const noexcept;

	  private:
		std::variant<BasicSocket const*, SocketObserver*>	_observable;
		BasicSocket::Serial									_saved_serial	= 0;
	};

	typedef std::vector<Object>			ObjectsList;
	typedef std::vector<SmootherBase*>	SmoothersList;

  public:
	typedef BasicSocket::Serial			Serial;
	typedef std::function<void()>		Callback;

  public:
	/**
	 * Add socket to be observed.
	 * When socket's value changes the callback function is called.
	 *
	 * ModuleSocket is held by reference, so the socket object must live as long as the SocketObserver.
	 */
	void
	observe (BasicSocket const& socket);

	/**
	 * Add another SocketObserver to observe.
	 * Similarly to observing a socket, if the other observer fires its callback function, then this observer
	 * will fire its own.
	 *
	 * The other observer is held by reference, and it must live as long as this observer lives.
	 */
	void
	observe (SocketObserver& observer);

	/**
	 * Add list of sockets to be tracked.
	 * Same as calling observe (GenericModuleSocket&) for each of the objects in list, in sequence.
	 */
	void
	observe (std::initializer_list<Object> list);

	/**
	 * Add list of sockets to be tracked, from a sequence.
	 */
	template<class Iterator>
		void
		observe (Iterator begin, Iterator end);

	/**
	 * Setup callback function.
	 * This function will be called when one of observed sockets is changed or observers is fired.
	 */
	void
	set_callback (Callback callback) noexcept;

	/**
	 * Set minimum time-delta accumulation before firing the callback function.
	 * To avoid aliasing, it's good to make sure that the observed data doesn't contain high-frequency value changes.
	 * Default is 0_s.
	 */
	void
	set_minimum_dt (si::Time) noexcept;

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
	si::Time
	update_time() const noexcept;

	/**
	 * Return time delta since last fire of the callback function.
	 */
	si::Time
	update_dt() const noexcept;

	/**
	 * Register smoother with this observer.
	 * SocketObserver will fire callbacks more than once after last socket change to accommodate for period of time greater or equal to the smoothing-time
	 * of the longest-smoothing Smoother. This is to ensure that smoothers continue to work and smooth data even after single-event socket change occurs.
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
	 * Tells the socket observer to do a callback on next occasion, regardless of other conditions, but takes into
	 * consideration minimum dt set with set_minimum_dt().
	 */
	void
	touch() noexcept;

  private:
	/**
	 * Find longest smoothing time from all registered smoothers.
	 * Return 0_s, if no smoothers are registered.
	 */
	si::Time
	longest_smoothing_time() noexcept;

  private:
	ObjectsList				_objects;
	SmoothersList			_smoothers;
	Callback				_callback;
	Serial					_serial						{ 0 };
	// Time of last change of observed socket:
	si::Time				_obs_update_time			{ 0_s };
	// Time of last firing of the callback function:
	si::Time				_fire_time					= 0_s;
	si::Time				_fire_dt					= 0_s;
	si::Time				_accumulated_dt				= 0_s;
	si::Time				_minimum_dt					= 0_s;
	std::optional<si::Time>	_longest_smoothing_time;
	// Set to true, when observed socket is updated, but
	// _minimum_dt prevented firing the callback.
	bool					_need_callback				{ false };
	bool					_additional_recompute		{ false };
	bool					_touch						{ false };
};


inline
SocketObserver::Object::Object (BasicSocket const* socket):
	_observable (socket)
{ }


inline
SocketObserver::Object::Object (SocketObserver* observer):
	_observable (observer)
{ }


inline BasicSocket::Serial
SocketObserver::Object::remote_serial() const noexcept
{
	return std::visit ([](auto const& observable) noexcept {
		return observable->serial();
	}, _observable);
}


template<class Iterator>
	void
	SocketObserver::observe (Iterator begin, Iterator end)
	{
		for (Iterator p = begin; p != end; ++p)
			observe (*p);
	}


inline SocketObserver::Serial
SocketObserver::serial() const noexcept
{
	return _serial;
}


inline si::Time
SocketObserver::update_time() const noexcept
{
	return _fire_time;
}


inline si::Time
SocketObserver::update_dt() const noexcept
{
	return _fire_dt;
}


inline void
SocketObserver::touch() noexcept
{
	_touch = true;
}

} // namespace xf

#endif


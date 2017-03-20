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

#ifndef XEFIS__UTILITY__SYNCHRONIZED_H__INCLUDED
#define XEFIS__UTILITY__SYNCHRONIZED_H__INCLUDED

// Standard:
#include <cstddef>
#include <mutex>


namespace xf {

/**
 * RAII-style safe lock. You need a token to access the resource, and if token exists, it guarantees
 * that the resource is locked.
 *
 * \param	pValue
 *			Type of balue to be protected by lock.
 * \param	pMutex
 *			One of standard locks (eg. std::mutex, etc) that can be dealt with with std::unique_lock<pMutex>.
 */
template<class pValue, class pMutex>
	class Synchronized
	{
	  public:
		using Value	= pValue;
		using Mutex	= pMutex;

		/**
		 * This object allows you to access the resource protected by Synchronized.
		 * As long as it exists, the lock is held.
		 */
		class UniqueAccessor
		{
			friend class Synchronized;

		  private:
			// Ctor
			explicit
			UniqueAccessor (Synchronized&);

		  public:
			// Move ctor
			UniqueAccessor (UniqueAccessor&&);

			// Move operator
			UniqueAccessor&
			operator= (UniqueAccessor&&) noexcept;

			/**
			 * Access the value by reference.
			 */
			Value&
			operator*() noexcept;

			/**
			 * Access the value by reference.
			 */
			Value const&
			operator*() const noexcept;

			/**
			 * Access the value by pointer.
			 */
			Value*
			operator->() noexcept;

			/**
			 * Access the value by pointer.
			 */
			Value const*
			operator->() const noexcept;

			/**
			 * Unlock the mutex and deassociate this Accessor from a Synchronized object.
			 * After calling this function, calling dereference operators is undefined-behaviour.
			 */
			void
			unlock() noexcept;

		  private:
			Value*					_value;
			std::unique_lock<Mutex>	_lock;
		};

	  public:
		// Ctor
		template<class ...Arg>
			Synchronized (Arg ...args);

		/**
		 * Return unique access token.
		 */
		UniqueAccessor
		unique_accessor();

	  private:
		Value	_value;
		Mutex	_mutex;
	};


template<class V, class M>
	Synchronized<V, M>::UniqueAccessor::UniqueAccessor (Synchronized& synchronized):
		_value (&synchronized._value),
		_lock (synchronized._mutex)
	{ }


template<class V, class M>
	inline
	Synchronized<V, M>::UniqueAccessor::UniqueAccessor (UniqueAccessor&& other):
		_value (other._value),
		_lock (std::move (other._lock))
	{ }


template<class V, class M>
	inline auto
	Synchronized<V, M>::UniqueAccessor::operator= (UniqueAccessor&& other) noexcept -> UniqueAccessor&
	{
		_value = &other;
		_lock = std::move (other._lock);
		return *this;
	}


template<class V, class M>
	inline auto
	Synchronized<V, M>::UniqueAccessor::operator*() noexcept -> Value&
	{
		return *_value;
	}


template<class V, class M>
	inline auto
	Synchronized<V, M>::UniqueAccessor::operator*() const noexcept -> Value const&
	{
		return *_value;
	}


template<class V, class M>
	inline auto
	Synchronized<V, M>::UniqueAccessor::operator->() noexcept -> Value*
	{
		return _value;
	}


template<class V, class M>
	inline auto
	Synchronized<V, M>::UniqueAccessor::operator->() const noexcept -> Value const*
	{
		return _value;
	}


template<class V, class M>
	inline void
	Synchronized<V, M>::UniqueAccessor::unlock() noexcept
	{
		_value = nullptr;
		_lock.unlock();
	}


template<class V, class M>
	template<class ...Arg>
		inline
		Synchronized<V, M>::Synchronized (Arg ...args):
			_value (std::forward<Arg> (args)...)
		{ }


template<class V, class M>
	inline auto
	Synchronized<V, M>::unique_accessor() -> UniqueAccessor
	{
		return UniqueAccessor (*this);
	}

} // namespace xf

#endif


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

#ifndef NEUTRINO__SYNCHRONIZED_H__INCLUDED
#define NEUTRINO__SYNCHRONIZED_H__INCLUDED

// Standard:
#include <cstddef>
#include <mutex>
#include <type_traits>


namespace neutrino {

template<class V, class M>
	class Synchronized;


/**
 * This object allows you to access the resource protected by Synchronized.
 * As long as it exists, the lock is held.
 *
 * You don't create it yourself, instead you use
 * Synchronized<T>::lock() method.
 */
template<class pValue, class pMutex>
	class UniqueAccessor
	{
		template<class V, class M>
			friend class Synchronized;

	  public:
		using Value	= pValue;
		using Mutex	= pMutex;

	  private:
		/**
		 * Constructor for UniqueAccessor for non-const values.
		 */
		template<class = std::enable_if_t<!std::is_const_v<Value>>>
			explicit
			UniqueAccessor (Synchronized<Value, Mutex>&);

		/**
		 * Constructor for UniqueAccessor for const values.
		 */
		template<class = std::enable_if_t<std::is_const_v<Value>>>
			explicit
			UniqueAccessor (Synchronized<std::remove_const_t<Value>, Mutex> const&);

	  public:
		// Move ctor
		UniqueAccessor (UniqueAccessor&&) = default;

		// Move operator
		UniqueAccessor&
		operator= (UniqueAccessor&&) noexcept = default;

		/**
		 * Access the value by reference.
		 */
		template<class = std::enable_if_t<!std::is_const_v<Value>>>
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
		template<class = std::enable_if_t<!std::is_const_v<Value>>>
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


/**
 * RAII-style safe lock. You need a token to access the resource, and if token exists, it guarantees
 * that the resource is locked.
 *
 * \param	pValue
 *			Type of balue to be protected by lock.
 * \param	pMutex
 *			One of standard locks (eg. std::mutex, etc) that can be dealt with with std::unique_lock<pMutex>.
 */
template<class pValue, class pMutex = std::mutex>
	class Synchronized
	{
		template<class V, class M>
			friend class UniqueAccessor;

	  public:
		using Value	= pValue;
		using Mutex	= pMutex;

	  public:
		// Ctor
		template<class ...Arg>
			Synchronized (Arg ...args) noexcept (noexcept (Value (std::forward<Arg> (args)...))):
				_value (std::forward<Arg> (args)...)
			{ }

		// Copy ctor
		Synchronized (Synchronized const&);

		// Move ctor
		Synchronized (Synchronized&&) = delete;

		// Copy operator
		Synchronized&
		operator= (Synchronized const&);

		// Move ctor
		Synchronized&
		operator= (Synchronized&&) = delete;

		/**
		 * Return unique access token.
		 */
		UniqueAccessor<Value, Mutex>
		lock();

		/**
		 * Return const unique access token.
		 */
		UniqueAccessor<Value const, Mutex>
		lock() const;

	  private:
		Value			_value;
		Mutex mutable	_mutex;
	};


template<class V, class M>
	template<class>
		inline
		UniqueAccessor<V, M>::UniqueAccessor (Synchronized<V, M>& synchronized):
			_value (&synchronized._value),
			_lock (synchronized._mutex)
		{ }


template<class V, class M>
	template<class>
		inline
		UniqueAccessor<V, M>::UniqueAccessor (Synchronized<std::remove_const_t<V>, M> const& synchronized):
			_value (&synchronized._value),
			_lock (synchronized._mutex)
		{ }


template<class V, class M>
	template<class>
		inline auto
		UniqueAccessor<V, M>::operator*() noexcept -> Value&
		{
			return *_value;
		}


template<class V, class M>
	inline auto
	UniqueAccessor<V, M>::operator*() const noexcept -> Value const&
	{
		return *_value;
	}


template<class V, class M>
	template<class>
		inline auto
		UniqueAccessor<V, M>::operator->() noexcept -> Value*
		{
			return _value;
		}


template<class V, class M>
	inline auto
	UniqueAccessor<V, M>::operator->() const noexcept -> Value const*
	{
		return _value;
	}


template<class V, class M>
	inline void
	UniqueAccessor<V, M>::unlock() noexcept
	{
		_value = nullptr;
		_lock.unlock();
	}


template<class V, class M>
	inline
	Synchronized<V, M>::Synchronized (Synchronized const& other):
		_value (*other.lock())
	{ }


template<class V, class M>
	inline Synchronized<V, M>&
	Synchronized<V, M>::operator= (Synchronized const& other)
	{
		*lock() = *other.lock();
		return *this;
	}


template<class V, class M>
	inline auto
	Synchronized<V, M>::lock() -> UniqueAccessor<Value, Mutex>
	{
		return UniqueAccessor<Value, Mutex> (*this);
	}


template<class V, class M>
	inline auto
	Synchronized<V, M>::lock() const -> UniqueAccessor<Value const, Mutex>
	{
		return UniqueAccessor<Value const, Mutex> (*this);
	}

} // namespace neutrino

#endif


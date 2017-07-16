/* vim:ts=4
 *
 * Copyleft 2008…2013  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__UTILITY__MUTEX_H__INCLUDED
#define XEFIS__UTILITY__MUTEX_H__INCLUDED

// Standard:
#include <cstddef>
#include <stdexcept>

// System:
#include <pthread.h>
#include <errno.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/noncopyable.h>


namespace xf {

/**
 * MutexPermissionException
 */
class MutexPermissionException: public std::runtime_error
{
  public:
	explicit
	MutexPermissionException (const char* what, const char* details):
		std::runtime_error ((what + ": "_str + details).c_str())
	{ }
};


/**
 * OO-oriented mutex.
 */
class Mutex: private Noncopyable
{
  public:
	/**
	 * Normal mutexes can be locked only once.
	 *
	 * Recursive mutexec can be locked many times
	 * by the same thread (and then they must be
	 * unlocked the same number of times).
	 */
	enum MutexType { Normal, Recursive };

	/**
	 * Lock representation for RAII-way locking. Instead of doing m.lock()
	 * and m.unlock() or m.synchronize ([] { … }) you can just create instance
	 * of below class and it will acquire the lock. It's released when the Lock
	 * object is destructed.
	 *
	 * Beware of the behavior when exceptions are thrown: the lock is released
	 * when exception is handled and stack is unwinded.
	 */
	class Lock: public Noncopyable
	{
	  public:
		// Ctor. Acquire lock.
		explicit
		Lock (Mutex const& mutex);

		// Move ctor:
		Lock (Lock&& other);

		// Dtor. Release lock.
		~Lock();

		/**
		 * Release lock explicitly.
		 */
		void
		release();

	  private:
		Mutex const&	_mutex;
		bool			_owns_lock;
	};

	/**
	 * Lock representation for RAII-way locking.
	 * Similar to Lock, but can fail acquiring the lock.
	 */
	class TryLock: public Noncopyable
	{
	  public:
		// Ctor. Try to acquire lock.
		explicit
		TryLock (Mutex const& mutex);

		// Move ctor:
		TryLock (TryLock&& other);

		// Dtor. Release lock if it was acquired.
		~TryLock();

		/**
		 * Return true if lock has been acquired.
		 */
		bool
		acquired() const;

		/**
		 * Release lock explicitly.
		 */
		void
		release();

	  private:
		Mutex const&	_mutex;
		bool			_owns_lock;
	};

  public:
	/**
	 * \param	mutex_kind
	 *			Type of mutex (Normal or Recursive).
	 */
	explicit
	Mutex (MutexType = Normal) noexcept;

	~Mutex() noexcept;

	/**
	 * Locks mutex or waits to be released and locks.
	 */
	void
	lock() const noexcept;

	/**
	 * Tries to lock mutex. Returns true if mutex was
	 * locked successfully, and false if it was not.
	 */
	bool
	try_lock() const noexcept;

	/**
	 * Unlocks mutex. If calling thread is not the thread
	 * that locked mutex, std::invalid_argument is thrown.
	 *
	 * \throws	std::invalid_argument
	 *			When calling thread does not own the mutex.
	 */
	void
	unlock() const;

	/**
	 * Unlocks and locks mutex again.
	 */
	void
	yield() const;

	/**
	 * Locks mutex and returns Lock object for this mutex.
	 */
	Lock
	acquire_lock() const;

	/**
	 * Lock, execute function, and unlock.
	 */
	template<class Callback>
		void
		synchronize (Callback function) const noexcept (noexcept (function()));

	/**
	 * Helper for lock-safe copying some value (eg. for returning):
	 * Like: return graph()->safe_copy (_some_value);
	 */
	template<class Type>
		Type
		safe_copy (Type const& value) const noexcept (noexcept (Type::operator= (value)));

	/**
	 * Helper for unlocking and returning given value.
	 */
	template<class Type>
		Type const&
		unlock_and_return (Type const& value) const;

	/**
	 * Helper for unlocking and throwing given object.
	 */
	template<class Type>
		void
		unlock_and_throw (Type const& value) const;

  private:
	::pthread_mutex_t mutable _mutex;
};


class RecursiveMutex: public Mutex
{
  public:
	RecursiveMutex() noexcept;
};


inline
Mutex::Lock::Lock (Mutex const& mutex):
	_mutex (mutex),
	_owns_lock (true)
{
	_mutex.lock();
}


inline
Mutex::Lock::Lock (Lock&& other):
	_mutex (other._mutex),
	_owns_lock (other._owns_lock)
{
	other._owns_lock = false;
}


inline
Mutex::Lock::~Lock()
{
	release();
}


inline void
Mutex::Lock::release()
{
	if (_owns_lock)
	{
		_mutex.unlock();
		_owns_lock = false;
	}
}


inline
Mutex::TryLock::TryLock (Mutex const& mutex):
	_mutex (mutex)
{
	_owns_lock = _mutex.try_lock();
}


inline
Mutex::TryLock::TryLock (TryLock&& other):
	_mutex (other._mutex),
	_owns_lock (other._owns_lock)
{
	other._owns_lock = false;
}


inline
Mutex::TryLock::~TryLock()
{
	release();
}


inline bool
Mutex::TryLock::acquired() const
{
	return _owns_lock;
}


inline void
Mutex::TryLock::release()
{
	if (_owns_lock)
	{
		_mutex.unlock();
		_owns_lock = false;
	}
}


inline void
Mutex::lock() const noexcept
{
	::pthread_mutex_lock (&_mutex);
}


inline bool
Mutex::try_lock() const noexcept
{
	switch (::pthread_mutex_trylock (&_mutex))
	{
		case EBUSY:
			return false;

		case 0:
			return true;
	}
	return false;
}


inline void
Mutex::unlock() const
{
	switch (::pthread_mutex_unlock (&_mutex))
	{
		case EPERM:
			throw MutexPermissionException ("the calling thread does not own the mutex", __func__);
	}
}


inline void
Mutex::yield() const
{
	unlock();
	lock();
}


inline Mutex::Lock
Mutex::acquire_lock() const
{
	return Lock (*this);
}


template<class Callback>
	inline void
	Mutex::synchronize (Callback function) const noexcept (noexcept (function()))
	{
		lock();
		if constexpr (noexcept (function()))
		{
			function();
			unlock();
		}
		else
		{
			try {
				function();
				unlock();
			}
			catch (...)
			{
				unlock();
				throw;
			}
		}
	}


template<class Type>
	inline Type
	Mutex::safe_copy (Type const& value) const noexcept (noexcept (Type::operator= (value)))
	{
		lock();
		Type copy = value;
		unlock();
		return copy;
	}


template<class Type>
	inline Type const&
	Mutex::unlock_and_return (Type const& value) const
	{
		unlock();
		return value;
	}


template<class Type>
	inline void
	Mutex::unlock_and_throw (Type const& value) const
	{
		unlock();
		throw value;
	}

} // namespace xf

#endif


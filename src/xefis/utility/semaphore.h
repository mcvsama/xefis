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

#ifndef XEFIS__UTILITY__SEMAPHORE_H__INCLUDED
#define XEFIS__UTILITY__SEMAPHORE_H__INCLUDED

// Standard:
#include <cstddef>

// System:
#include <semaphore.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/noncopyable.h>


/**
 * OO-oriented semaphore.
 */
class Semaphore: private Noncopyable
{
  public:
	explicit
	Semaphore (int value = 0) noexcept;

	~Semaphore() noexcept;

	/**
	 * Reset semaphore to initial value.
	 * No thead should wait on semaphore at the moment
	 * of the call.
	 */
	void
	reset() noexcept;

	/**
	 * Return semaphore value. If there are
	 * threads waiting on a semaphore, it will still
	 * return 0 instead of negative number.
	 */
	int
	value() const noexcept;

	/**
	 * Locks semaphore.
	 */
	void
	wait() const noexcept;

	/**
	 * Tries to lock semaphore. Returns true if semaphore was
	 * locked successfully, false otherwise.
	 */
	bool
	try_wait() const noexcept;

	/**
	 * Unlocks semaphore.
	 */
	void
	post() const noexcept;

  private:
	::sem_t mutable	_semaphore;
	int				_initial_value;
};

#endif


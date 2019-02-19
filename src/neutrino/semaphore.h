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

#ifndef NEUTRINO__SEMAPHORE_H__INCLUDED
#define NEUTRINO__SEMAPHORE_H__INCLUDED

// Standard:
#include <condition_variable>
#include <cstddef>
#include <mutex>

// Neutrino:
#include <neutrino/noncopyable.h>


namespace neutrino {

/**
 * Simple semaphore using standard mutex and condition_variable.
 */
class Semaphore: private Noncopyable
{
  public:
	explicit
	Semaphore (std::size_t initial_count = 0);

	void
	notify (std::size_t how_many = 1);

	void
	wait();

	bool
	try_wait();

  private:
	std::mutex				_mutex;
	std::condition_variable	_condition;
	std::size_t				_count;
};


inline
Semaphore::Semaphore (std::size_t initial_count):
	_count (initial_count)
{ }


inline void
Semaphore::notify (std::size_t how_many)
{
	std::unique_lock lock (_mutex);
	_count += how_many;

	for (std::size_t i = 0; i < how_many; ++i)
		_condition.notify_one();
}


inline void
Semaphore::wait()
{
	std::unique_lock lock (_mutex);

	// Handle spurious wake-ups:
	while (_count == 0)
		_condition.wait (lock);

	--_count;
}


inline bool
Semaphore::try_wait()
{
	std::unique_lock lock (_mutex);

	if (_count > 0)
	{
		--_count;
		return true;
	}
	else
		return false;
}

} // namespace neutrino

#endif


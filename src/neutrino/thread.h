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

#ifndef NEUTRINO__THREAD_H__INCLUDED
#define NEUTRINO__THREAD_H__INCLUDED

// Standard:
#include <cstddef>
#include <thread>

// Neutrino:
#include <neutrino/exception.h>


namespace neutrino {

/**
 * Available schedulers for threads.
 */
enum class ThreadScheduler: int
{
	FIFO	= SCHED_FIFO,
	RR		= SCHED_RR,
	Other	= SCHED_OTHER
};


/**
 * Thrown from thread scheduling config function.
 */
class SchedulerException: public Exception
{
  public:
	using Exception::Exception;
};


/**
 * Set scheduling policy/parameter for thread.
 * Has to be called when thread is running.
 */
void
set (std::thread& thread, ThreadScheduler scheduler, int priority);

} // namespace neutrino

#endif


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

// Standard:
#include <cstddef>
#include <stdexcept>

// System:
#include <pthread.h>
#include <errno.h>

// Local:
#include "mutex.h"


Mutex::Mutex (MutexType kind) noexcept
{
	static ::pthread_mutex_t initializer = PTHREAD_MUTEX_INITIALIZER;
	static ::pthread_mutex_t recursive_initializer = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

	switch (kind)
	{
		case Normal:
			_mutex = initializer;
			break;

		case Recursive:
			_mutex = recursive_initializer;
			break;
	}
}


Mutex::~Mutex() noexcept
{
	::pthread_mutex_destroy (&_mutex);
}


RecursiveMutex::RecursiveMutex() noexcept:
	Mutex (Mutex::Recursive)
{ }


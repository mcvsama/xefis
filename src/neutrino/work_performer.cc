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
#include <utility>

// Neutrino:
#include <neutrino/thread.h>

// Local:
#include "work_performer.h"


namespace neutrino {

WorkPerformer::WorkPerformer (std::size_t threads_number, Logger const& logger):
	_logger (logger.with_scope ("<work performer>"))
{
	_logger << "Creating WorkPerformer" << std::endl;

	for (std::size_t i = 0; i < threads_number; ++i)
		_threads.emplace_back (std::move (std::thread (&WorkPerformer::thread, this)));
}


WorkPerformer::~WorkPerformer()
{
	_logger << "Destroying WorkPerformer" << std::endl;

	_terminating = true;
	_tasks_semaphore.notify (_threads.size());

	for (auto& thread: _threads)
		thread.join();
}


void
WorkPerformer::set (ThreadScheduler scheduler, int priority)
{
	for (auto& thread: _threads)
		neutrino::set (thread, scheduler, priority);
}


std::size_t
WorkPerformer::queued_tasks() const noexcept
{
	return _tasks.lock()->size();
}


void
WorkPerformer::thread()
{
	while (!_terminating)
	{
		_tasks_semaphore.wait();
		std::unique_ptr<AbstractTask> task;

		{
			auto tasks = _tasks.lock();

			if (!tasks->empty())
			{
				task = std::move (tasks->front());
				tasks->pop();
			}
		}

		if (task)
			(*task)();
	}
}

} // namespace neutrino


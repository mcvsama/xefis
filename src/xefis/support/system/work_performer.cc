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

// Local:
#include "work_performer.h"


namespace xf {

WorkPerformer::Performer::Performer (WorkPerformer* work_performer, unsigned int thread_id) noexcept:
	_work_performer (work_performer),
	_thread_id (thread_id)
{
	// 128k-words stack (512kB on 32-bit, 1MB on 64-bit system) should be sufficient for most operations:
	set_stack_size (128 * sizeof (size_t) * 1024);
}


void
WorkPerformer::Performer::run()
{
	Unit* unit = 0;
	while ((unit = _work_performer->take_unit()))
	{
		unit->_is_ready.store (false);
		unit->_thread_id = _thread_id;
		unit->execute();
		unit->_is_ready.store (true);
		unit->_wait_sem.post();
	}
}


WorkPerformer::WorkPerformer (unsigned int threads_number)
{
	_logger.set_prefix ("<work performer>");
	_logger << "Creating WorkPerformer" << std::endl;

	threads_number = std::max (1u, threads_number);

	for (unsigned int i = 0; i < threads_number; ++i)
	{
		Shared<Performer> p = std::make_shared<Performer> (this, i);
		_performers.push_back (p);
		p->start();
	}
}


WorkPerformer::~WorkPerformer()
{
	_logger << "Destroying WorkPerformer" << std::endl;

	for (decltype (_performers.size()) i = 0; i < _performers.size(); ++i)
		_queue_semaphore.post();
	for (auto p: _performers)
		p->wait();
}


void
WorkPerformer::add (Unit* unit)
{
	_queue_mutex.synchronize ([&] {
		unit->added_to_queue();
		_queue.push (unit);
	});
	_queue_semaphore.post();
}


void
WorkPerformer::set_sched (Thread::SchedType sched_type, int priority) const noexcept
{
	for (auto p: _performers)
		p->set_sched (sched_type, priority);
}


WorkPerformer::Unit*
WorkPerformer::take_unit()
{
	_queue_semaphore.wait();
	Mutex::Lock lock (_queue_mutex);

	if (_queue.empty())
		return 0;
	else
	{
		Unit* u = _queue.front();
		_queue.pop();
		return u;
	}
}

} // namespace xf


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

#ifndef NEUTRINO__WORK_PERFORMER_H__INCLUDED
#define NEUTRINO__WORK_PERFORMER_H__INCLUDED

// Standard:
#include <cstddef>
#include <functional>
#include <future>
#include <optional>
#include <queue>
#include <tuple>
#include <vector>

// Neutrino:
#include <neutrino/logger.h>
#include <neutrino/noncopyable.h>
#include <neutrino/semaphore.h>
#include <neutrino/synchronized.h>
#include <neutrino/thread.h>


namespace neutrino {

/**
 * WorkPerformer queues packaged tasks and executes them in the context of separate threads. It's a good
 * solution for CPU-intensive tasks.
 *
 * The threads are waiting for execution packaged tasks even if nothing is scheduled, so no time is lost
 * for creating new threads. Also avoids creating too many threads at the same time.
 *
 * It's not a good solution for packaged tasks that do IO, since they will block execution units (threads).
 */
class WorkPerformer: private Noncopyable
{
  public:
	// Ctor
	explicit
	WorkPerformer (std::size_t threads_number, Logger const&);

	// Dtor
	~WorkPerformer();

	/**
	 * Set scheduling parameter for all threads.
	 */
	void
	set (ThreadScheduler, int priority);

	/**
	 * Return number of threads created.
	 */
	std::size_t
	threads_number() const noexcept;

	/**
	 * Return number of tasks which haven't started execution.
	 */
	std::size_t
	queued_tasks() const noexcept;

	/**
	 * Submit new task to execute.
	 */
	template<class Result, class ...Args>
		std::future<Result>
		submit (std::packaged_task<Result (Args...)>&&, Args&&...);

	/**
	 * Submit new task to execute.
	 */
	template<class Function, class ...Args>
		std::future<std::invoke_result_t<Function&&, Args&&...>>
		submit (Function&& function, Args&&...);

  private:
	/**
	 * Type-erasing container for tasks to execute.
	 */
	struct AbstractTask
	{
		virtual
		~AbstractTask() = default;

		virtual void
		operator()() = 0;
	};

	using TaskQueue = std::queue<std::unique_ptr<AbstractTask>>;

  private:
	/**
	 * Function executed by all threads.
	 * Waits for new tasks or exits when _terminating is true.
	 */
	void
	thread();

  private:
	Logger							_logger;
	std::atomic<bool>				_terminating { false };
	Synchronized<TaskQueue> mutable	_tasks;
	Semaphore						_tasks_semaphore;
	std::vector<std::thread>		_threads;
};


inline std::size_t
WorkPerformer::threads_number() const noexcept
{
	return _threads.size();
}


template<class Result, class ...Args>
	inline std::future<Result>
	WorkPerformer::submit (std::packaged_task<Result (Args...)>&& task, Args&&... args)
	{
		using PackagedTask = std::packaged_task<Result (Args...)>;

		struct ConcreteTask: public AbstractTask
		{
			explicit
			ConcreteTask (PackagedTask&& pt, Args&&... args):
				packaged_task (std::move (pt)),
				args (std::forward<Args> (args)...)
			{ }

			void
			operator()() override
			{
				std::apply (packaged_task, std::forward<std::tuple<Args...>> (args));
			}

			PackagedTask		packaged_task;
			std::tuple<Args...>	args;
		};

		std::future<Result> future = task.get_future();
		_tasks.lock()->emplace (std::make_unique<ConcreteTask> (std::move (task), std::forward<Args> (args)...));
		_tasks_semaphore.notify();
		return future;
	}


template<class Function, class ...Args>
	inline std::future<std::invoke_result_t<Function&&, Args&&...>>
	WorkPerformer::submit (Function&& function, Args&&... args)
	{
		return submit (std::packaged_task<std::invoke_result_t<Function, Args...> (Args...)> (std::forward<Function> (function)),
					   std::forward<Args> (args)...);
	}

} // namespace neutrino

#endif


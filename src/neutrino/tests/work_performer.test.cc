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
#include <thread>
#include <type_traits>

// Neutrino:
#include <neutrino/work_performer.h>

// Neutrino:
#include <neutrino/test/test.h>


namespace neutrino::test {
namespace {

using namespace std::chrono_literals;

Logger g_null_logger;


RuntimeTest t1 ("neutrino::WorkPerformer: execute 100'000 non-trivial tasks", []{
	constexpr int kTasks = 100'000;
	constexpr int kResult = 1337;

	auto delayed_return = [] (int thing_to_return) noexcept {
		for (volatile int i = 0; i < 1'000; ++i)
			continue;

		return thing_to_return;
	};

	std::optional<WorkPerformer> wp (std::in_place, 8, g_null_logger);
	std::vector<std::future<int>> futures;

	for (std::size_t i = 0; i < kTasks; ++i)
		futures.emplace_back (wp->submit (delayed_return, kResult));

	// Wait for all tasks to finish:
	while (wp->queued_tasks() > 0)
		std::this_thread::sleep_for (0.1s);

	// Destroy the work performer:
	wp.reset();

	bool all_are_correct = true;

	for (auto& future: futures)
		if (future.get() != kResult)
			all_are_correct = false;

	test_asserts::verify ("tasks are executed and return values correctly", all_are_correct);
});


RuntimeTest t2 ("neutrino::WorkPerformer abandons not-started tasks when destructed", []{
	auto sleeper = [] (auto time) {
		std::this_thread::sleep_for (time);
		return 0;
	};

	std::optional<WorkPerformer> wp (std::in_place, 1, g_null_logger);
	auto v1 = wp->submit (sleeper, 0.01s);
	auto v2 = wp->submit (sleeper, 0.3s);

	wp.reset();

	try {
		v2.get();
		test_asserts::verify ("exception is thrown", false);
	}
	catch (std::future_error& e)
	{
		test_asserts::verify ("promise is broken", e.code() == std::future_errc::broken_promise);
	}
});

} // namespace
} // namespace neutrino::test


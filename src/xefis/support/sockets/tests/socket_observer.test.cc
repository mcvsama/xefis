/* vim:ts=4
 *
 * Copyleft 2008…2018  Michał Gawron
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

// Neutrino:
#include <neutrino/test/auto_test.h>

// Xefis:
#include <xefis/core/cycle.h>
#include <xefis/core/module_io.h>
#include <xefis/core/sockets/module_out.h>
#include <xefis/core/sockets/socket.h>
#include <xefis/support/sockets/socket_observer.h>


namespace xf::test {
namespace {

using TestedType = int64_t;

constexpr TestedType kValue1 = 5;
constexpr TestedType kValue2 = -100;

xf::Logger g_null_logger;


class TestCycle: public Cycle
{
  public:
	explicit
	TestCycle():
		Cycle (1, 0_s, 1_s, 1_s, g_null_logger)
	{ }

	TestCycle&
	operator+= (si::Time dt)
	{
		Cycle::operator= (Cycle (number() + 1, update_time() + dt, dt, dt, g_null_logger));
		return *this;
	}
};


template<class T>
	struct TestEnvironment
	{
	  private:
		std::unique_ptr<ModuleIO>	io		{ std::make_unique<ModuleIO>() };

	  public:
		ModuleOut<T>				out		{ io.get(), "out" };
		ModuleIn<T>					in		{ io.get(), "in" };
		Module<ModuleIO>			module	{ std::move (io) };
		TestCycle					cycle;
		SocketObserver				observer;
		std::optional<T>			result;
		size_t						calls	{ 0 };

	  public:
		explicit
		TestEnvironment()
		{
			in << out;
			observer.observe (in);
			observer.set_callback ([this] {
				result = in.get_optional();
				++calls;
			});
		}
	};


AutoTest t1 ("xf::SocketObserver noticing changes", []{
	TestEnvironment<TestedType> env;

	env.out = kValue1;
	env.in.fetch (env.cycle += 1_s);
	env.observer.process (env.cycle.update_time());
	test_asserts::verify ("SocketObserver calls callback on change to non-nil", env.result && (*env.result == kValue1));
	test_asserts::verify ("SocketObserver calls callback once", env.calls == 1);

	env.out = xf::nil;
	env.in.fetch (env.cycle += 1_s);
	env.observer.process (env.cycle.update_time());
	test_asserts::verify ("SocketObserver calls callback on change to nil", !env.result);
	test_asserts::verify ("SocketObserver calls callback twice", env.calls == 2);
});


AutoTest t2 ("xf::SocketObserver set_minimum_dt()", []{
	TestEnvironment<TestedType> env;

	env.observer.set_minimum_dt (5_s);
	env.out = kValue2;
	env.in.fetch (env.cycle += 1_s);
	env.observer.process (env.cycle.update_time());
	test_asserts::verify ("SocketObserver waits minimum_dt before firing (no fire)", !env.result);

	env.in.fetch (env.cycle += 4.01_s);
	env.observer.process (env.cycle.update_time());
	test_asserts::verify ("SocketObserver waits minimum_dt before firing (fire)", env.result && (*env.result == kValue2));
});


AutoTest t3 ("xf::SocketObserver serial()", []{
	TestEnvironment<TestedType> env;

	auto serial = env.observer.serial();
	env.out = kValue1;
	env.in.fetch (env.cycle += 1_s);
	test_asserts::verify ("SocketObserver serial() doesn't change before calling process()", env.observer.serial() == serial);
	env.observer.process (env.cycle.update_time());
	test_asserts::verify ("SocketObserver serial() gets updated after calling process()", env.observer.serial() > serial);

	serial = env.observer.serial();
	env.out = xf::nil;
	env.in.fetch (env.cycle += 1_s);
	env.observer.process (env.cycle.update_time());
	test_asserts::verify ("SocketObserver serial() gets updated after calling process() even for nil values", env.observer.serial() > serial);
});


AutoTest t4 ("xf::SocketObserver update_time()", []{
	TestEnvironment<TestedType> env;

	auto ut = (env.cycle += 1_s).update_time();
	env.out = kValue1;
	env.in.fetch (env.cycle);
	env.observer.process (env.cycle.update_time());
	test_asserts::verify ("SocketObserver update_time() returns last time of actually firing a callback (1)", env.observer.update_time() == ut);

	env.in.fetch (env.cycle += 1_s);
	env.observer.process (env.cycle.update_time());
	test_asserts::verify ("SocketObserver update_time() returns last time of actually firing a callback (2)", env.observer.update_time() == ut);

	ut = (env.cycle += 1_s).update_time();
	env.out = kValue2;
	env.in.fetch (env.cycle);
	env.observer.process (env.cycle.update_time());
	test_asserts::verify ("SocketObserver update_time() returns last time of actually firing a callback (3)", env.observer.update_time() == ut);
});


AutoTest t5 ("xf::SocketObserver touch()", []{
	TestEnvironment<TestedType> env;

	env.out = kValue1;
	env.in.fetch (env.cycle += 1_s);
	env.observer.process (env.cycle.update_time());
	test_asserts::verify ("SocketObserver calls callback once", env.calls == 1);

	env.in.fetch (env.cycle += 1_s);
	env.observer.process (env.cycle.update_time());
	test_asserts::verify ("SocketObserver doesn't call callback on no change", env.calls == 1);

	env.observer.touch();
	env.in.fetch (env.cycle += 1_s);
	env.observer.process (env.cycle.update_time());
	test_asserts::verify ("SocketObserver calls callback after touch()", env.calls == 2);
});


AutoTest t6 ("xf::SocketObserver depending smoothers", []{
	TestEnvironment<TestedType> env;
	xf::Smoother<TestedType> smoother { 5_s };

	env.observer.add_depending_smoother (smoother);
	env.out = kValue1;

	// Verify that callback gets called multiple times even if socket value doesn't change, to make sure that Smoother can continue to properly smooth and
	// output the data.
	for (size_t i = 0; i < 10; ++i)
	{
		env.in.fetch (env.cycle += 1_s);
		env.observer.process (env.cycle.update_time());
	}

	// Expect total 7 calls, 1 for value change, 5 for 5 seconds of smoothing time plus one additional
	// to ensure smoother has finished.
	test_asserts::verify ("callback was called 5 times after last socket change", env.calls == 7);
});


AutoTest t7 ("xf::SocketObserver observing other observers", []{
	// TODO
});

} // namespace
} // namespace xf::test


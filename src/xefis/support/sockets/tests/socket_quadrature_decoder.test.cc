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
#include <tuple>

// Boost:
#include <boost/range/adaptor/indexed.hpp>

// Neutrino:
#include <neutrino/test/auto_test.h>

// Xefis:
#include <xefis/core/module_io.h>
#include <xefis/core/sockets/module_socket.h>
#include <xefis/core/sockets/tests/test_cycle.h>
#include <xefis/support/sockets/socket_quadrature_counter.h>
#include <xefis/support/sockets/socket_quadrature_decoder.h>


namespace xf::test {
namespace {

using Integer = int16_t;


template<class T>
	inline std::string
	to_string (std::optional<T> o)
	{
		if (o)
			return std::to_string (*o);
		else
			return "std::nullopt";
	}


AutoTest t1 ("SocketQuadratureDecoder + SocketQuadratureCounter", []{
	// SocketQuadratureDecoder will be tested by testing SocketQuadratureCounter, so no need for separate tests.

	// Tuple is <input_a, input_b, delta, callback_expected>:
	std::vector<std::tuple<std::optional<bool>, std::optional<bool>, std::optional<Integer>, bool>> const test_steps {
		// Count up:
		{ false,        false,         0,           false },
		{ false,        true,         +1,           true },
		{ true,         true,         +1,           true },
		{ true,         false,        +1,           true },
		{ false,        false,        +1,           true },
		{ false,        true,         +1,           true },
		// Count down:
		{ false,        false,        -1,           true },
		{ true,         false,        -1,           true },
		{ true,         true,         -1,           true },
		{ false,        true,         -1,           true },
		{ false,        false,        -1,           true },
		// Count down below 0:
		{ true,         false,        -1,           true },
		{ true,         true,         -1,           true },
		{ false,        true,         -1,           true },
		// When both inputs are changed, result should not change:
		{ true,         false,        std::nullopt, true },
		{ false,        true,         std::nullopt, true },
		{ true,         false,        std::nullopt, true },
		// Works again:
		{ true,         true,         -1,           true },
		{ true,         false,        +1,           true },
		{ false,        false,        +1,           true },
		{ true,         false,        -1,           true },
		// Missing values:
		{ std::nullopt, false,        std::nullopt, true },
		{ std::nullopt, std::nullopt, std::nullopt, true },
		{ false,        std::nullopt, std::nullopt, true },
		{ true,         std::nullopt, std::nullopt, true },
		// Works again:
		{ true,         false,         0,           false },
		{ true,         true,         -1,           true },
		{ false,        true,         -1,           true },
		{ true,         true,         +1,           true },
	};

	constexpr auto initial_value = 5;

	std::optional<std::tuple<std::optional<Integer>, Integer>> callback_result;
	auto callback = [&callback_result] (auto delta, Integer total) {
		callback_result = { delta, total };
	};
	ModuleIO io;
	ModuleOut<bool> socket_a (&io, "line-a");
	ModuleOut<bool> socket_b (&io, "line-b");
	SocketQuadratureCounter<Integer> decoder (socket_a, socket_b, initial_value, callback);
	auto expected_total = initial_value;

	for (auto const& step: test_steps | boost::adaptors::indexed (0))
	{
		auto const expected_delta = std::get<2> (step.value());
		auto const callback_expected = std::get<3> (step.value());

		callback_result.reset();
		socket_a = std::get<0> (step.value());
		socket_b = std::get<1> (step.value());
		expected_total += expected_delta.value_or (0);
		decoder.process();

		std::string const on_step = "on step " + std::to_string (step.index());

		test_asserts::verify (on_step + " counter delta is " + std::to_string (expected_total) +
							  " (is " + std::to_string (decoder.value()) + ")",
							  decoder.value() == expected_total);

		if (callback_expected)
		{
			test_asserts::verify (on_step + " callback was called", !!callback_result);

			auto const& delta = std::get<0> (*callback_result);
			auto const& total = std::get<1> (*callback_result);

			test_asserts::verify (on_step + " callback was called with correct delta value " + to_string (expected_delta) +
								  " (is " + to_string (delta) + ")",
								  delta == expected_delta);
			test_asserts::verify (on_step + " callback was called with correct total value " + std::to_string (expected_total) +
								  " (is " + std::to_string (total) + ")",
								  total == expected_total);
		}
		else
			test_asserts::verify (on_step + " callback was not called", !callback_result);
	}
});

} // namespace
} // namespace xf::test


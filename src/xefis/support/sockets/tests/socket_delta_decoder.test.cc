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

// Neutrino:
#include <neutrino/test/auto_test.h>

// Xefis:
#include <xefis/core/module_io.h>
#include <xefis/core/sockets/module_socket.h>
#include <xefis/support/sockets/socket_delta_decoder.h>


namespace xf::test {
namespace {

AutoTest t1 ("SocketDeltaDecoder", []{
	using Integer = int16_t;
	using Callback = std::function<void (std::optional<Integer>)>;

	ModuleIO io;
	ModuleOut<Integer> socket (&io, "output");
	Callback verifications_callback;
	SocketDeltaDecoder<Integer> decoder (socket, [&verifications_callback] (auto const delta) {
		verifications_callback (delta);
	}, 5);

	auto verify = [&] (bool callback_should_be_called, Callback verifications_code = [](auto){}) {
		bool executed = false;
		verifications_callback = [&] (auto delta) {
			verifications_code (delta);
			executed = true;
		};

		decoder.process();

		if (callback_should_be_called)
			test_asserts::verify ("decoder callback was called", executed);
		else
			test_asserts::verify ("decoder callback was not called", !executed);

		verifications_callback = [](auto) {};
	};

	socket = 6;
	verify (true, [](auto const delta) {
		test_asserts::verify ("delta is +1", delta && *delta == 1);
	});

	socket = 4;
	verify (true, [](auto const delta) {
		test_asserts::verify ("delta is -2", delta && *delta == -2);
	});

	verify (false);

	socket = xf::nil;
	verify (true, [](auto const delta) {
		test_asserts::verify ("delta is std::nullopt", !delta);
	});

	socket = 2;
	verify (true, [](auto const delta) {
		test_asserts::verify ("delta is -2", delta && *delta == -2);
	});

	socket = 4;
	decoder.call_action (10);
	verify (true, [](auto const delta) {
		test_asserts::verify ("delta is +2 after call_action (10)", delta && *delta == 2);
	});
});

} // namespace
} // namespace xf::test


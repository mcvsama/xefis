/* vim:ts=4
 *
 * Copyleft 2026  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

// Xefis:
#include <xefis/core/cycle.h>
#include <xefis/core/sockets/module_socket.h>
#include <xefis/core/sockets/tests/test_cycle.h>
#include <xefis/core/sockets/tests/test_enum.h>
#include <xefis/test/test_processing_loop.h>

// Neutrino:
#include <neutrino/demangle.h>
#include <neutrino/test/auto_test.h>

// Standard:
#include <cstddef>
#include <memory>
#include <string>


namespace xf::test {
namespace {

namespace test_asserts = nu::test_asserts;

using namespace nu::si::units;


template<class Lambda>
	std::function<void()>
	for_all_types (Lambda lambda)
	{
		return [lambda] {
			lambda.template operator()<bool> (false);
			lambda.template operator()<bool> (true);
			lambda.template operator()<int8_t> (-5);
			lambda.template operator()<int8_t> (0);
			lambda.template operator()<int8_t> (+5);
			lambda.template operator()<int16_t> (-5);
			lambda.template operator()<int16_t> (0);
			lambda.template operator()<int16_t> (-5);
			lambda.template operator()<int32_t> (-5);
			lambda.template operator()<int32_t> (0);
			lambda.template operator()<int32_t> (+5);
			lambda.template operator()<int64_t> (-5);
			lambda.template operator()<int64_t> (0);
			lambda.template operator()<int64_t> (+5);
			lambda.template operator()<uint8_t> (0);
			lambda.template operator()<uint8_t> (5);
			lambda.template operator()<uint16_t> (0);
			lambda.template operator()<uint16_t> (5);
			lambda.template operator()<uint32_t> (0);
			lambda.template operator()<uint32_t> (5);
			lambda.template operator()<uint64_t> (0);
			lambda.template operator()<uint64_t> (5);
			lambda.template operator()<float16_t> (0.125f16);
			lambda.template operator()<float32_t> (0.125f);
			lambda.template operator()<float64_t> (0.125);
			lambda.template operator()<float128_t> (0.125L);
			lambda.template operator()<std::string> ("");
			lambda.template operator()<std::string> ("v");
			lambda.template operator()<std::string> ("value");
			lambda.template operator()<si::Length> (1.15_m);
			lambda.template operator()<TestEnum> (TestEnum::Value1);
			lambda.template operator()<TestEnum> (TestEnum::Value2);
			lambda.template operator()<TestEnumWithNil> (TestEnumWithNil::Value1);
			lambda.template operator()<TestEnumWithNil> (TestEnumWithNil::Value2);
		};
	}


template<class T>
	std::string
	desc_type (std::string str)
	{
		return std::string (str) + " <" + nu::demangle (typeid (T).name()) + ">";
	}


nu::AutoTest t1 ("xf::SocketTraits", for_all_types ([](auto value) {
	using Value = decltype (value);
	auto traits = SocketTraits<Value>();

	// Test non-nil values.

	auto loop = TestProcessingLoop (0.1_s);
	auto module = Module (loop);
	auto original_socket = ModuleOut<Value> (&module, "original");

	{
		auto reconstructed_socket = ModuleOut<Value> (&module, "reconstructed");
		original_socket = value;
		auto const blob = traits.to_blob (original_socket);
		traits.from_blob (reconstructed_socket, blob);

		test_asserts::verify (desc_type<Value> ("blob is not empty"), !blob.empty());
		test_asserts::verify_equal (desc_type<Value> ("to_blob() works with from_blob()"), original_socket, reconstructed_socket);
		test_asserts::verify_equal (desc_type<Value> ("to_blob() works with from_blob() (dereferenced)"), *original_socket, *reconstructed_socket);
	}

	// Test nil values.

	{
		auto reconstructed_socket = ModuleOut<Value> (&module, "reconstructed");
		original_socket = xf::nil;
		auto const blob = traits.to_blob (original_socket);
		traits.from_blob (reconstructed_socket, blob);

		test_asserts::verify (desc_type<Value> ("blob is not empty"), !blob.empty());
		test_asserts::verify_equal (desc_type<Value> ("to_blob() works with from_blob()"), original_socket, reconstructed_socket);
		test_asserts::verify (desc_type<Value> ("original_socket is nil"), original_socket.is_nil());
		test_asserts::verify (desc_type<Value> ("reconstructed_socket is nil"), reconstructed_socket.is_nil());
	}
}));

} // namespace
} // namespace xf::test

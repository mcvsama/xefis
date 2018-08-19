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
#include <string>

// Xefis:
#include <xefis/test/test.h>
#include <xefis/utility/blob.h>
#include <xefis/utility/demangle.h>


namespace xf::test {
namespace {

template<class Value>
	void
	test_serialization (Value value)
	{
		using namespace std::literals;

		Blob serialized;
		value_to_blob (value, serialized);
		Value deserialized;
		blob_to_value (serialized, deserialized);
		test_asserts::verify ("serialization of "s + demangle (typeid (Value).name()) + " works", value == deserialized);

		if constexpr (std::is_same<Value, bool>())
		{
			if (!value)
				serialized[0] = 0xff;
			else
				serialized[0] = 0;
		}
		else
			serialized[0] = 0xff;

		blob_to_value (serialized, deserialized);
		test_asserts::verify ("deserialization of broken "s + demangle (typeid (Value).name()) + " works", value != deserialized);
	}


template<class Value>
	void
	test_size (Value value, size_t expected_size)
	{
		using namespace std::literals;

		Blob blob;
		value_to_blob (value, blob);
		test_asserts::verify ("size of serialized "s + demangle(typeid (Value).name()) + " is " + std::to_string (expected_size),
							  blob.size() == expected_size);
	}


RuntimeTest t1 ("blob: value_to_blob", []{
	enum class TestEnum {
		Value1, Value2, Value3,
	};

	test_serialization<bool> (false);
	test_serialization<bool> (true);
	test_serialization<int8_t> (-5);
	test_serialization<int16_t> (-5114);
	test_serialization<int32_t> (-559340);
	test_serialization<int64_t> (-503293402432);
	test_serialization<uint8_t> (5);
	test_serialization<uint16_t> (5114);
	test_serialization<uint32_t> (559340);
	test_serialization<uint64_t> (503293402432);
	test_serialization<float16_t> (0.15_half);
	test_serialization<float32_t> (0.152534f);
	test_serialization<float64_t> (0.15253452890394);
	test_serialization<float128_t> (0.152534503492039402890394L);
	test_serialization<si::Length> (1.15_m);
	test_serialization<std::string> ("random string");
	test_serialization<TestEnum> (TestEnum::Value1);
	test_serialization<TestEnum> (TestEnum::Value3);
});


RuntimeTest t2 ("blob: little-endianess of serialized int", []{
	Blob result;
	value_to_blob<uint32_t> (0x44332211, result);

	test_asserts::verify ("byte[3] is 0x44", result[3] == 0x44);
	test_asserts::verify ("byte[2] is 0x33", result[2] == 0x33);
	test_asserts::verify ("byte[1] is 0x22", result[1] == 0x22);
	test_asserts::verify ("byte[0] is 0x11", result[0] == 0x11);
	test_asserts::verify ("to_hex_string works correctly", to_hex_string (result) == "11:22:33:44");
});


RuntimeTest t3 ("blob: test sizes of serialized data", []{
	enum class TestEnum8: uint8_t {
		Value,
	};

	enum class TestEnum32: uint32_t {
		Value,
	};

	test_size<bool> (false, 1);
	test_size<int8_t> (0, 1);
	test_size<int16_t> (0, 2);
	test_size<int32_t> (0, 4);
	test_size<int64_t> (0, 8);
	test_size<uint8_t> (0, 1);
	test_size<uint16_t> (0, 2);
	test_size<uint32_t> (0, 4);
	test_size<uint64_t> (0, 8);
	test_size<float16_t> (0.0_half, 2);
	test_size<float32_t> (0.0f, 4);
	test_size<float64_t> (0.0, 8);
	test_size<float128_t> (0.0L, 16);
	test_size<si::Length> (0_m, 8);
	test_size<std::string> ("random string", 13);
	test_size<TestEnum8> (TestEnum8::Value, 1);
	test_size<TestEnum32> (TestEnum32::Value, 4);
});

} // namespace
} // namespace xf::test


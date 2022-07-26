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

// Xefis:
#include <xefis/core/cycle.h>
#include <xefis/core/sockets/module_socket.h>
#include <xefis/core/sockets/tests/test_cycle.h>

// Neutrino:
#include <neutrino/demangle.h>
#include <neutrino/test/auto_test.h>

// Standard:
#include <cstddef>
#include <memory>
#include <sstream>
#include <string>
#include <type_traits>


namespace xf::test {
namespace {

using namespace si::units;
using namespace std::literals;

inline namespace test_enum {

enum class TestEnum
{
	Value1,
	Value2,
};


enum class TestEnumWithNil
{
	Value1,
	Value2,
	xf_nil_value,
};


constexpr std::string_view
to_string (TestEnum value)
{
	switch (value)
	{
		case TestEnum::Value1:	return "Value1";
		case TestEnum::Value2:	return "Value2";
	}

	return "";
}


void
parse (std::string_view const& str, TestEnum& value)
{
	if (str == "Value1")
		value = TestEnum::Value1;
	else if (str == "Value2")
		value = TestEnum::Value2;
	else
		throw Exception ("invalid enum string \"" + std::string (str) + "\"");
}


constexpr std::string_view
to_string (TestEnumWithNil value)
{
	switch (value)
	{
		case TestEnumWithNil::Value1:		return "Value1";
		case TestEnumWithNil::Value2:		return "Value2";
		case TestEnumWithNil::xf_nil_value:	return "";
	}

	return "";
}


void
parse (std::string_view const& str, TestEnumWithNil& value)
{
	if (str == "Value1")
		value = TestEnumWithNil::Value1;
	else if (str == "Value2")
		value = TestEnumWithNil::Value2;
	else if (str == "")
		value = TestEnumWithNil::xf_nil_value;
	else
		throw Exception ("invalid enum string \"" + std::string (str) + "\"");
}

} // namespace test_enum


xf::Logger g_null_logger;


template<class T>
	struct TestEnvironment
	{
	  public:
		Module						module;
		ModuleOut<T>				out		{ &module, "out" };
		ModuleIn<T>					mid		{ &module, "mid" };
		ModuleIn<T>					in		{ &module, "in" };
		TestCycle					cycle;
	};


template<class Lambda>
	std::function<void()>
	for_all_types (Lambda lambda)
	{
		return [lambda] {
			lambda.template operator()<bool, bool> (true, false);
			lambda.template operator()<int8_t, int8_t> (120, -5);
			lambda.template operator()<int16_t, int16_t> (1337, -5);
			lambda.template operator()<int32_t, int32_t> (1337, -5);
			lambda.template operator()<int64_t, int64_t> (1337, -5);
			lambda.template operator()<uint8_t, uint8_t> (133, 5);
			lambda.template operator()<uint16_t, uint16_t> (1337, 5);
			lambda.template operator()<uint32_t, uint32_t> (1337, 5);
			lambda.template operator()<uint64_t, uint64_t> (1337, 5);
			lambda.template operator()<float16_t, float16_t> (0.125_half, 0.0_half);
			lambda.template operator()<float32_t, float32_t> (0.125f, 0.0f);
			lambda.template operator()<float64_t, float64_t> (0.125, 0.0);
			lambda.template operator()<float128_t, float128_t> (0.125L, 0.0L);
			lambda.template operator()<std::string, std::string> ("value-1", "value-2");
			lambda.template operator()<si::Length> (1.15_m, -2.5_m);
			lambda.template operator()<TestEnum> (TestEnum::Value1, TestEnum::Value2);
			lambda.template operator()<TestEnumWithNil> (TestEnumWithNil::Value1, TestEnumWithNil::Value2);
		};
	}


template<class T>
	std::string
	desc_type (std::string str)
	{
		return std::string (str) + " <" + xf::demangle (typeid (T).name()) + ">";
	}


template<class T, template<class> class AnySocket>
	void
	test_nil_values (AnySocket<T> const& socket, T test_value)
	{
		test_asserts::verify (desc_type<T> ("nil socket is converted to false"),
							  !socket);

		test_asserts::verify (desc_type<T> ("nil socket says it's nil"),
							  socket.is_nil());

		test_asserts::verify (desc_type<T> ("nil socket says it's nil"),
							  !socket.valid());

		test_asserts::verify (desc_type<T> ("reading nil socket with operator*() throws"),
							  Exception::catch_and_log (g_null_logger, [&]{ static_cast<void> (*socket); }));

		test_asserts::verify (desc_type<T> ("reading nil socket with get_optional() returns empty std::optional"),
							  !socket.get_optional().has_value());

		test_asserts::verify (desc_type<T> ("reading nil socket with value_or() gives the argument"),
							  socket.value_or (test_value) == test_value);
	}


template<class T, template<class> class AnySocket>
	void
	test_non_nil_values (AnySocket<T> const& socket, T test_value, std::string what)
	{
		test_asserts::verify (desc_type<T> (what + " converts to true"),
							  !!socket);

		test_asserts::verify (desc_type<T> ("reading " + what + " with operator*() does not throw"),
							  !Exception::catch_and_log (g_null_logger, [&]{ static_cast<void> (*socket); }));

		test_asserts::verify (desc_type<T> ("socket's value != test_value"),
							  *socket != test_value);

		test_asserts::verify (desc_type<T> ("reading non-nil socket with value_or() gives socket's value"),
							  socket.value_or (test_value) == *socket);

		test_asserts::verify (desc_type<T> ("reading non-nil socket with get_optional() returns non-empty std::optional"),
							  socket.get_optional().has_value());

		test_asserts::verify (desc_type<T> ("reading socket with get_optional() returns correct value"),
							  *socket.get_optional() == *socket);
	}


template<class T>
	constexpr bool
	should_test_string_serialization()
	{
		// String-serialization is not symmetrical for all types, it doesn't have to, it's for user-interface anyway.
		// So test this only if it makes sense for T to be symmetrically string-serialized.
		return std::is_arithmetic<T>() || std::is_same<T, float16_t>() || std::is_same<T, bool>() || si::is_quantity<T>();
	}


AutoTest t1 ("xf::Socket nil and non-nil values", for_all_types ([](auto value1, auto value2) {
	TestEnvironment<decltype (value1)> env;

	// The socket should initially be nil:
	test_nil_values (env.in, value2);
	test_nil_values (env.out, value2);

	// Non-nil values:
	env.in << value1;
	env.in.fetch (env.cycle += 1_s);
	test_non_nil_values (env.in, value2, "non-nil socket");

	env.out = value1;
	// No fetching needed when directly assigning.
	test_non_nil_values (env.out, value2, "non-nil socket");

	// Nil again:
	env.in << xf::no_data_source;
	env.in.fetch (env.cycle += 1_s);
	test_nil_values (env.in, value2);

	env.out = xf::nil;
	// No fetching needed when directly assigning.
	test_nil_values (env.out, value2);
}));


AutoTest t2 ("xf::Socket fallback values", for_all_types ([](auto value1, auto value2) {
	using T = decltype (value1);

	{
		TestEnvironment<T> env;

		// Fallback values:
		env.in.set_fallback (value1);
		test_non_nil_values (env.in, value2, "socket with fallback value");

		env.out.set_fallback (value1);
		test_non_nil_values (env.out, value2, "socket with fallback value");

		// No fallback again:
		env.in.set_fallback (std::nullopt);
		test_nil_values (env.in, value2);

		env.out.set_fallback (std::nullopt);
		test_nil_values (env.out, value2);
	}

	// Test fallback_value provided in ctor:
	{
		auto const fallback_value = value1;
		Module module;
		ModuleIn<T> tmp_prop (&module, "fallback-test", fallback_value);
		test_asserts::verify (desc_type<T> ("fallback-value set in ctor works"), *tmp_prop == fallback_value);
	}

	// Test fallback on connected sockets:
	{
		TestEnvironment<T> env;

		env.in << env.mid;
		env.mid << env.out;

		// Fallback on output socket:
		env.out.set_fallback (value2);

		env.out = value1;
		env.in.fetch (env.cycle += 1_s);
		test_asserts::verify ("non-fallback value on ModuleOut works", *env.in == value1);

		env.out = xf::nil;
		env.in.fetch (env.cycle += 1_s);
		test_asserts::verify ("fallback value on ModuleOut works", *env.in == value2);

		// Fallback on middle socket:
		env.out.set_fallback (std::nullopt);
		env.mid.set_fallback (value2);

		env.out = value1;
		env.in.fetch (env.cycle += 1_s);
		test_asserts::verify ("non-fallback value on middle ModuleOut works", *env.in == value1);

		env.out = xf::nil;
		env.in.fetch (env.cycle += 1_s);
		test_asserts::verify ("fallback value on ModuleIn works", *env.in == value2);

		// Fallback on input socket:
		env.out.set_fallback (std::nullopt);
		env.mid.set_fallback (std::nullopt);
		env.in.set_fallback (value2);

		env.out = value1;
		env.in.fetch (env.cycle += 1_s);
		test_asserts::verify ("non-fallback value on ModuleIn works", *env.in == value1);

		env.out = xf::nil;
		env.in.fetch (env.cycle += 1_s);
		test_asserts::verify ("fallback value on ModuleIn works", *env.in == value2);
	}
}));


AutoTest t3 ("xf::Socket serial numbers", for_all_types ([](auto value1, auto value2) {
	using T = decltype (value1);

	// Serial number:
	{
		TestEnvironment<T> env;

		env.out = value1;
		auto serial_0 = env.out.serial();

		env.out = value2;
		auto serial_1 = env.out.serial();
		test_asserts::verify (desc_type<T> ("serial increments when value changes"), serial_1 == serial_0 + 1);

		env.out = value2;
		auto serial_2 = env.out.serial();
		test_asserts::verify (desc_type<T> ("serial does not increment when value doesn't change"), serial_2 == serial_1);
	}

	// Test serial on connected sockets:
	{
		TestEnvironment<T> env;

		env.in << env.mid;
		env.mid << env.out;

		env.out = value1;
		env.in.fetch (env.cycle += 1_s);
		auto serial_0 = env.in.serial();

		env.out = value2;
		env.in.fetch (env.cycle += 1_s);
		auto serial_1 = env.in.serial();
		test_asserts::verify (desc_type<T> ("serial increments when value changes over connected sockets"), serial_1 == serial_0 + 1);

		env.out = value2;
		env.in.fetch (env.cycle += 1_s);
		auto serial_2 = env.in.serial();
		test_asserts::verify (desc_type<T> ("serial does not increment when value doesn't change over connected sockets"), serial_2 == serial_1);
	}
}));


AutoTest t4 ("xf::Socket transferring data", for_all_types ([](auto value1, auto value2) {
	using T = decltype (value1);

	TestEnvironment<T> env;

	// Test if data is transferred from output to input sockets properly:
	env.in << env.mid;
	env.mid << env.out;

	env.out = value1;
	env.in.fetch (env.cycle += 1_s);
	test_asserts::verify ("transferring data from output to input sockets works (1)", *env.in == value1);

	env.out = value2;
	env.in.fetch (env.cycle += 1_s);
	test_asserts::verify ("transferring data from output to input sockets works (2)", *env.in == value2);

	env.out = value1;
	env.in.fetch (env.cycle); // Note the same cycle as before; value should not change.
	test_asserts::verify ("caching values if cycle-number doesn't change works", *env.in == value2);

	env.out = xf::nil;
	env.in.fetch (env.cycle += 1_s);
	test_asserts::verify ("transferring nil-values from output to input sockets works", env.in.is_nil());
}));


AutoTest t5 ("xf::Socket serialization", for_all_types ([](auto value1, auto value2) {
	using T = decltype (value1);

	// Serialization:
	{
		TestEnvironment<T> env;

		// String serialization:
		if constexpr (should_test_string_serialization<T>())
		{
			env.in << value1;
			env.in.fetch (env.cycle += 1_s);
			auto serialized = env.in.to_string();
			env.out = value2;
			test_asserts::verify (desc_type<T> ("to_string(): socket == value2"), *env.out == value2);
			env.out.from_string (serialized);
			test_asserts::verify (desc_type<T> ("to_string() serialization works correctly for"), *env.out == value1);

			// If this is si::Quantity, make sure that from_string() throws on incompatible types:
			if constexpr (si::is_quantity<T>())
			{
				test_asserts::verify ("from_string() throws when units are incompatible",
									  Exception::catch_and_log (g_null_logger, [&]{ env.out.from_string ("1.15 m^2"); }));
			}
		}

		// Blob serialization:
		{
			env.in << value1;
			env.in.fetch (env.cycle += 1_s);
			auto serialized = env.in.to_blob();
			env.out = value2;
			test_asserts::verify (desc_type<T> ("to_blob(): socket == value2"), *env.out == value2);
			env.out.from_blob (serialized);
			test_asserts::verify (desc_type<T> ("to_blob() serialization works correctly for"), *env.out == value1);
		}
	}

	// Serialization of nil-values:
	{
		TestEnvironment<T> env;

		// String serialization:
		if constexpr (should_test_string_serialization<T>())
		{
			env.in << xf::no_data_source;
			env.in.fetch (env.cycle += 1_s);
			auto serialized = env.in.to_string();
			env.out = value1;
			test_asserts::verify (desc_type<T> ("to_string() on nil: socket == value1"), *env.out == value1);
			env.out.from_string (serialized);
			test_asserts::verify (desc_type<T> ("to_string() serialization on nil value works correctly for"), !env.out);
		}

		// Blob serialization:
		{
			env.in << xf::no_data_source;
			env.in.fetch (env.cycle += 1_s);
			auto serialized = env.in.to_blob();
			env.out = value1;
			test_asserts::verify (desc_type<T> ("to_blob() on nil: socket == value1"), *env.out == value1);
			env.out.from_blob (serialized);
			test_asserts::verify (desc_type<T> ("to_blob() serialization on nil value works correctly for"), !env.out);
		}
	}
}));


AutoTest t6 ("xf::Socket various behavior", for_all_types ([](auto value1, auto) {
	using T = decltype (value1);

	Module module;
	ModuleOut<T> out { &module, "out" };
	ModuleIn<T> in { &module, "in" };

	in << std::function ([&](std::optional<T>) -> std::optional<T> { throw std::runtime_error ("test"); }) << out;

	test_asserts::verify ("nil_by_fetch_exception flag is false before first fetching",
						  !in.nil_by_fetch_exception());
	test_asserts::verify ("fetch() doesn't throw when a transform function throws",
						  !Exception::catch_and_log (g_null_logger, [&]{ in.fetch (TestCycle()); }));
	test_asserts::verify ("fetch() signals nil_by_fetch_exception flag when a transform function throws",
						  in.nil_by_fetch_exception());
}));


AutoTest t7 ("xf::Socket operator=", for_all_types ([](auto value1, auto value2) {
	using T = decltype (value1);

	Module module;

	// Make sure ModuleIn<T>::operator= (ModuleIn<T>) is forbidden as it may be misleading:
	test_asserts::verify ("ModuleIn<T>::operator= (ModuleIn<T>) is forbidden", !std::copyable<ModuleIn<T>>);

	// Make sure operator= copies value, not the identity of ModuleOut:
	ModuleOut<T> out1 { &module, "out1" };
	ModuleOut<T> out2 { &module, "out2" };
	out1 = value1;
	out2 = value2;
	test_asserts::verify ("out1 has test value1", *out1 == value1);
	test_asserts::verify ("out2 has test value2", *out2 == value2);
	out1 = *out2;
	test_asserts::verify ("out1's identity haven't changed", out1.path() == ModuleSocketPath ("out1"));
	test_asserts::verify ("out2 has test value2", *out1 == value2);
}));


AutoTest t8 ("xf::Socket << 5; xf::Socket << \"abc\"", []{
	TestEnvironment<int> env1;
	TestEnvironment<std::string> env2;

	env1.in << 5;
	env1.in.fetch (env1.cycle += 1_s);
	test_asserts::verify ("can use literal constant as data source", env1.in.value_or (0) == 5);

	env2.in << "abc";
	env2.in.fetch (env2.cycle += 1_s);
	test_asserts::verify ("can use literal constant as data source", env2.in.value_or ("") == "abc");
});


AutoTest t9 ("xf::Socket << std::unique_ptr<xf::Socket>", []{
	TestEnvironment<int> env1;

	// TODO Zabroń konstrukcji ConnectableSocket (5), bo przy następnym fetchu i tak się zniluje,
	// albo zrób tak, że jest to ekwiwalent ConnectableSocket() << ConstantSource (5);
	auto mid = std::make_unique<xf::ConnectableSocket<int>>();
	*mid << 5;
	auto& mid_ref = env1.in << std::move (mid);
	env1.in.fetch (env1.cycle += 1_s);
	test_asserts::verify ("can use unique_ptr<Socket> as data source", env1.in.value_or (0) == 5);

	mid_ref << env1.out;
	env1.out = 10;
	env1.in.fetch (env1.cycle += 1_s);
	test_asserts::verify ("can use unique_ptr<Socket> as data source", env1.in.value_or (0) == 10);
});

AutoTest t10 ("xf::ConnectableSocket expression", []{
	TestEnvironment<int32_t> env;

	env.in << std::function<int (int)> ([](auto const value) { return value * 2; }) << env.out;

	env.out = 11;
	env.in.fetch (env.cycle += 1_s);
	test_asserts::verify ("expression transforms data properly", *env.in == 22);
});


AutoTest t11 ("xf::ConnectableSocket expression (different input/output types)", []{
	Module						module;
	ModuleOut<int>				out		{ &module, "out" };
	ModuleIn<std::string>		in		{ &module, "in" };
	TestCycle					cycle;

	in
		<< std::function<std::string (int)> ([](auto const value) { return std::to_string (value) + "abc"; })
		<< std::function<int (std::string)> ([](auto const value) { return std::stoi (value) + 11; })
		<< std::function<std::string (int)> ([](auto const value) { return std::to_string (value) + "000"; })
		<< out;

	out = 33;
	in.fetch (cycle += 1_s);
	test_asserts::verify ("expression transforms data properly", *in == "33011abc");
});


AutoTest t12 ("xf::ConnectableSocket expression (reactions to nil)", []{
	Module						module;
	ModuleOut<int>				out		{ &module, "out" };
	ModuleIn<std::string>		in0		{ &module, "in0" };
	ModuleIn<std::string>		in1		{ &module, "in1" };
	ModuleIn<std::string>		in2		{ &module, "in2" };
	ModuleIn<std::string>		in3		{ &module, "in3" };
	ModuleIn<std::string>		in4nil	{ &module, "in4nil" };
	ModuleIn<std::string>		in4str	{ &module, "in4str" };
	ModuleIn<std::string>		in5		{ &module, "in5" };
	TestCycle					cycle;

	in0 << std::function<std::string (int)>() << out;
	in1 << std::function<std::string (int)> ([](auto const) { return "never"; }) << out;
	in2 << std::function<std::string (std::optional<int>)> ([](auto const) { return "always str"; }) << out;
	in3 << std::function<std::optional<std::string> (int)> ([](auto const) { return "never str"; }) << out;
	in4nil << std::function<std::optional<std::string> (std::optional<int>)> ([](auto const) { return std::nullopt; }) << out;
	in4str << std::function<std::optional<std::string> (std::optional<int>)> ([](auto const) { return "always str"; }) << out;
	in5 << std::function<std::optional<std::string> (std::optional<int>)> ([](std::optional<int> const) -> std::optional<std::string> { throw "exception"; }) << out;

	out = xf::nil;
	cycle += 1_s;
	in0.fetch (cycle);
	in1.fetch (cycle);
	in2.fetch (cycle);
	in3.fetch (cycle);
	in4nil.fetch (cycle);
	in4str.fetch (cycle);
	in5.fetch (cycle);
	test_asserts::verify ("expression transforms data properly (in0)", in0.is_nil());
	test_asserts::verify ("expression transforms data properly (in1)", in1.is_nil());
	test_asserts::verify ("expression transforms data properly (in2)", *in2 == "always str");
	test_asserts::verify ("expression transforms data properly (in3)", in3.is_nil());
	test_asserts::verify ("expression transforms data properly (in4nil)", in4nil.is_nil());
	test_asserts::verify ("expression transforms data properly (in4str)", *in4str == "always str");
	test_asserts::verify ("expression transforms data properly (in5)", in5.is_nil());
});

} // namespace
} // namespace xf::test


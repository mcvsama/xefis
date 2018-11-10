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
#include <memory>
#include <sstream>
#include <string>
#include <type_traits>

// Xefis:
#include <xefis/core/cycle.h>
#include <xefis/core/property.h>
#include <xefis/test/test.h>
#include <xefis/utility/demangle.h>


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
		PropertyOut<T>				out		{ io.get(), "out" };
		PropertyOut<T>				mid		{ io.get(), "mid" };
		PropertyIn<T>				in		{ io.get(), "in" };
		Module<ModuleIO>			module	{ std::move (io) };
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


template<class T, template<class> class AnyProperty>
	void
	test_nil_values (AnyProperty<T> const& property, T test_value)
	{
		test_asserts::verify (desc_type<T> ("nil property is converted to false"),
							  !property);

		test_asserts::verify (desc_type<T> ("nil property says it's nil"),
							  property.is_nil());

		test_asserts::verify (desc_type<T> ("nil property says it's nil"),
							  !property.valid());

		test_asserts::verify (desc_type<T> ("reading nil property with operator*() throws"),
							  Exception::catch_and_log (g_null_logger, [&]{ static_cast<void> (*property); }));

		test_asserts::verify (desc_type<T> ("reading nil property with get_optional() returns empty std::optional"),
							  !property.get_optional().has_value());

		test_asserts::verify (desc_type<T> ("reading nil property with value_or() gives the argument"),
							  property.value_or (test_value) == test_value);
	}


template<class T, template<class> class AnyProperty>
	void
	test_non_nil_values (AnyProperty<T> const& property, T test_value, std::string what)
	{
		test_asserts::verify (desc_type<T> (what + " converts to true"),
							  !!property);

		test_asserts::verify (desc_type<T> ("reading " + what + " with operator*() does not throw"),
							  !Exception::catch_and_log (g_null_logger, [&]{ static_cast<void> (*property); }));

		test_asserts::verify (desc_type<T> ("property's value != test_value"),
							  *property != test_value);

		test_asserts::verify (desc_type<T> ("reading non-nil property with value_or() gives property's value"),
							  property.value_or (test_value) == *property);

		test_asserts::verify (desc_type<T> ("reading non-nil property with get_optional() returns non-empty std::optional"),
							  property.get_optional().has_value());

		test_asserts::verify (desc_type<T> ("reading property with get_optional() returns correct value"),
							  *property.get_optional() == *property);
	}


template<class T>
	constexpr bool
	should_test_string_serialization()
	{
		// String-serialization is not symmetrical for all types, it doesn't have to, it's for user-interface anyway.
		// So test this only if it makes sense for T to be symmetrically string-serialized.
		return std::is_arithmetic<T>() || std::is_same<T, float16_t>() || std::is_same<T, bool>() || si::is_quantity<T>();
	}


RuntimeTest t1 ("xf::Property nil and non-nil values", for_all_types ([](auto value1, auto value2) {
	TestEnvironment<decltype (value1)> env;

	// The property should initially be nil:
	test_nil_values (env.in, value2);
	test_nil_values (env.out, value2);

	// Non-nil values:
	env.in << ConstantSource (value1);
	test_non_nil_values (env.in, value2, "non-nil property");

	env.out = value1;
	test_non_nil_values (env.out, value2, "non-nil property");

	// Nil again:
	env.in << xf::no_data_source;
	test_nil_values (env.in, value2);

	env.out = xf::nil;
	test_nil_values (env.out, value2);
}));


RuntimeTest t2 ("xf::Property fallback values", for_all_types ([](auto value1, auto value2) {
	using T = decltype (value1);

	{
		TestEnvironment<T> env;

		// Fallback values:
		env.in.set_fallback (value1);
		test_non_nil_values (env.in, value2, "property with fallback value");

		env.out.set_fallback (value1);
		test_non_nil_values (env.out, value2, "property with fallback value");

		// No fallback again:
		env.in.set_fallback (std::nullopt);
		test_nil_values (env.in, value2);

		env.out.set_fallback (std::nullopt);
		test_nil_values (env.out, value2);
	}

	// Test fallback_value provided in ctor:
	{
		auto const fallback_value = value1;
		ModuleIO io;
		PropertyIn<T> tmp_prop (&io, "fallback-test", fallback_value);
		test_asserts::verify (desc_type<T> ("fallback-value set in ctor works"), *tmp_prop == fallback_value);
	}

	// Test fallback on connected properties:
	{
		TestEnvironment<T> env;

		env.in << env.mid;
		env.mid << env.out;

		// Fallback on output property:
		env.out.set_fallback (value2);

		env.out = value1;
		env.in.fetch (env.cycle += 1_s);
		test_asserts::verify ("non-fallback value on PropertyOut works", *env.in == value1);

		env.out = xf::nil;
		env.in.fetch (env.cycle += 1_s);
		test_asserts::verify ("fallback value on PropertyOut works", *env.in == value2);

		// Fallback on middle property:
		env.out.set_fallback (std::nullopt);
		env.mid.set_fallback (value2);

		env.out = value1;
		env.in.fetch (env.cycle += 1_s);
		test_asserts::verify ("non-fallback value on middle PropertyOut works", *env.in == value1);

		env.out = xf::nil;
		env.in.fetch (env.cycle += 1_s);
		test_asserts::verify ("fallback value on PropertyIn works", *env.in == value2);

		// Fallback on input property:
		env.out.set_fallback (std::nullopt);
		env.mid.set_fallback (std::nullopt);
		env.in.set_fallback (value2);

		env.out = value1;
		env.in.fetch (env.cycle += 1_s);
		test_asserts::verify ("non-fallback value on PropertyIn works", *env.in == value1);

		env.out = xf::nil;
		env.in.fetch (env.cycle += 1_s);
		test_asserts::verify ("fallback value on PropertyIn works", *env.in == value2);
	}
}));


RuntimeTest t3 ("xf::Property serial numbers", for_all_types ([](auto value1, auto value2) {
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

	// Test serial on connected properties:
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
		test_asserts::verify (desc_type<T> ("serial increments when value changes over connected properties"), serial_1 == serial_0 + 1);

		env.out = value2;
		env.in.fetch (env.cycle += 1_s);
		auto serial_2 = env.in.serial();
		test_asserts::verify (desc_type<T> ("serial does not increment when value doesn't change over connected properties"), serial_2 == serial_1);
	}
}));


RuntimeTest t4 ("xf::Property transferring data", for_all_types ([](auto value1, auto value2) {
	using T = decltype (value1);

	TestEnvironment<T> env;

	// Test if data is transferred from output to input properties properly:
	env.in << env.mid;
	env.mid << env.out;

	env.out = value1;
	env.in.fetch (env.cycle += 1_s);
	test_asserts::verify ("transferring data from output to input properties works (1)", *env.in == value1);

	env.out = value2;
	env.in.fetch (env.cycle += 1_s);
	test_asserts::verify ("transferring data from output to input properties works (2)", *env.in == value2);

	env.out = value1;
	env.in.fetch (env.cycle); // Note the same cycle as before; value should not change.
	test_asserts::verify ("caching values if cycle-number doesn't change works", *env.in == value2);

	env.out = xf::nil;
	env.in.fetch (env.cycle += 1_s);
	test_asserts::verify ("transferring nil-values from output to input properties works", env.in.is_nil());
}));


RuntimeTest t5 ("xf::Property serialization", for_all_types ([](auto value1, auto value2) {
	using T = decltype (value1);

	// Serialization:
	{
		TestEnvironment<T> env;

		// String serialization:
		if constexpr (should_test_string_serialization<T>())
		{
			env.in << ConstantSource (value1);
			auto serialized = env.in.to_string();
			env.out = value2;
			test_asserts::verify (desc_type<T> ("to_string(): property == value2"), *env.out == value2);
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
			env.in << ConstantSource (value1);
			auto serialized = env.in.to_blob();
			env.out = value2;
			test_asserts::verify (desc_type<T> ("to_blob(): property == value2"), *env.out == value2);
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
			auto serialized = env.in.to_string();
			env.out = value1;
			test_asserts::verify (desc_type<T> ("to_string() on nil: property == value1"), *env.out == value1);
			env.out.from_string (serialized);
			test_asserts::verify (desc_type<T> ("to_string() serialization on nil value works correctly for"), !env.out);
		}

		// Blob serialization:
		{
			env.in << xf::no_data_source;
			auto serialized = env.in.to_blob();
			env.out = value1;
			test_asserts::verify (desc_type<T> ("to_blob() on nil: property == value1"), *env.out == value1);
			env.out.from_blob (serialized);
			test_asserts::verify (desc_type<T> ("to_blob() serialization on nil value works correctly for"), !env.out);
		}
	}
}));


RuntimeTest t6 ("xf::Property various behavior", for_all_types ([](auto value1, auto) {
	using T = decltype (value1);

	ModuleIO io;
	PropertyOut<T> out { &io, "out" };
	PropertyIn<T> in { &io, "in" };

	in << out;
	test_asserts::verify ("fetch() throws when no Module is assigned",
						  Exception::catch_and_log (g_null_logger, [&]{ in.fetch (TestCycle()); }));
}));


// Primary template handles types that do not support T::operator= (T const&):
template<class, class = std::void_t<>>
	struct HasAssignmentOperator: public std::false_type
	{ };


// Specialization recognizes types that do support T::operator= (T const&):
template<class T>
	struct HasAssignmentOperator<T, std::void_t<decltype (std::declval<T&> = std::declval<T&>())>>: public std::true_type
	{ };


RuntimeTest t7 ("xf::Property operator=", for_all_types ([](auto value1, auto value2) {
	using T = decltype (value1);

	ModuleIO io;

	// Make sure PropertyIn<T>::operator= is forbidden as it may be misleading:
	test_asserts::verify ("PropertyIn<T>::operator= (PropertyIn<T>) is forbidden", !HasAssignmentOperator<PropertyIn<T>>::value);

	// Make sure operator= copies value, not the identity of PropertyOut:
	PropertyOut<T> out1 { &io, "out1" };
	PropertyOut<T> out2 { &io, "out2" };
	out1 = value1;
	out2 = value2;
	test_asserts::verify ("out1 has test value1", *out1 == value1);
	test_asserts::verify ("out2 has test value2", *out2 == value2);
	out1 = out2;
	test_asserts::verify ("out1's identity haven't change", out1.path() == PropertyPath ("out1"));
	test_asserts::verify ("out2 has test value2", *out1 == value2);
}));

} // namespace
} // namespace xf::test


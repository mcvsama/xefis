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

// Xefis:
#include <xefis/core/processing_loop.h>
#include <xefis/core/property.h>
#include <xefis/test/test.h>
#include <xefis/utility/demangle.h>


namespace xf::test {

using namespace test_asserts;
using namespace si::units;
using namespace std::literals;


static std::ostringstream g_logger_buffer;
static xf::Logger g_null_logger (g_logger_buffer);


template<class T>
	static std::string
	desc_type (std::string_view str)
	{
		return str + " <"s + xf::demangle (typeid (T).name()) + ">";
	}


Cycle
get_cycle (size_t cycle_number)
{
	return Cycle (cycle_number, 1_s, 1_s, g_null_logger);
}


template<class T, template<class> class AnyProperty>
	static void
	test_nil_values (AnyProperty<T> const& property, T test_value)
	{
		test_asserts::verify (desc_type<T> ("nil property is converted to false"),
							  !property);

		test_asserts::verify (desc_type<T> ("nil property says it's nil"),
							  property.is_nil());

		test_asserts::verify (desc_type<T> ("nil property says it's nil"),
							  !property.valid());

		test_asserts::verify (desc_type<T> ("reading nil property with operator*() throws"),
							  Exception::catch_and_log (g_null_logger, [&]{ *property; }));

		test_asserts::verify (desc_type<T> ("reading nil property with get_optional() returns empty std::optional"),
							  !property.get_optional().has_value());

		test_asserts::verify (desc_type<T> ("reading nil property with value_or() gives the argument"),
							  property.value_or (test_value) == test_value);
	}


template<class T, template<class> class AnyProperty>
	static void
	test_non_nil_values (AnyProperty<T> const& property, T test_value, std::string_view what)
	{
		test_asserts::verify (desc_type<T> (what + " converts to true"s),
							  !!property);

		test_asserts::verify (desc_type<T> ("reading "s + what + " with operator*() does not throw"),
							  !Exception::catch_and_log (g_null_logger, [&]{ *property; }));

		test_asserts::verify (desc_type<T> ("property's value != test_value"),
							  *property != test_value);

		test_asserts::verify (desc_type<T> ("reading non-nil property with value_or() gives property's value"),
							  property.value_or (test_value) == *property);

		test_asserts::verify (desc_type<T> ("reading non-nil property with get_optional() returns non-empty std::optional"),
							  property.get_optional().has_value());

		test_asserts::verify (desc_type<T> ("reading property with get_optional() returns correct value"),
							  *property.get_optional() == *property);
	}


template<class T, template<class> class AnyProperty>
	static void
	test_serialization (AnyProperty<T>& property, T value)
	{
		// String serialization:
		{
			property.from_string (property.to_string());
			test_asserts::verify (desc_type<T> ("to_string() serialization works correctly for "), *property == value);
		}

		// Blob serialization:
		{
			property.from_blob (property.to_blob());
			test_asserts::verify (desc_type<T> ("to_blob() serialization works correctly for "), *property == value);
		}
	}


template<class T, template<class> class AnyProperty>
	static void
	test_nil_serialization (AnyProperty<T>& property)
	{
		// String serialization:
		{
			property.from_string (property.to_string());
			test_asserts::verify (desc_type<T> ("to_string() on nil-property deserializes correctly to nil-property"), !property);
		}

		// Blob serialization:
		{
			property.from_blob (property.to_blob());
			test_asserts::verify (desc_type<T> ("to_blob() on nil-property deserializes correctly to nil-property"), !property);
		}
	}


template<class T>
	inline void
	do_t1 (T value1, T value2)
	{
		ModuleIO io;
		PropertyOut<T> output (&io, "output");
		PropertyIn<T> input (&io, "input");

		// The property should initially be nil:
		test_nil_values (input, value2);
		test_nil_values (output, value2);

		// Non-nil values:
		input << ConstantSource (value1);
		test_non_nil_values (input, value2, "non-nil property");

		output = value1;
		test_non_nil_values (output, value2, "non-nil property");

		// Nil again:
		input << xf::no_data_source;
		test_nil_values (input, value2);

		output = xf::nil;
		test_nil_values (output, value2);

		// Fallback values:
		input.set_fallback (value1);
		test_non_nil_values (input, value2, "property with fallback value");

		output.set_fallback (value1);
		test_non_nil_values (output, value2, "property with fallback value");

		// No fallback again:
		input.set_fallback (std::nullopt);
		test_nil_values (input, value2);

		output.set_fallback (std::nullopt);
		test_nil_values (output, value2);

		// Test fallback_value provided in ctor:
		{
			auto const fallback_value = value1;
			PropertyIn<T> f (&io, "fallback-test", fallback_value);
			test_asserts::verify (desc_type<T> ("fallback-value set in ctor works"), *f == fallback_value);
		}

		// Serial number:
		{
			output = value1;
			auto serial_0 = output.serial();

			output = value2;
			auto serial_1 = output.serial();
			test_asserts::verify (desc_type<T> ("serial increments when value changes"), serial_1 == serial_0 + 1);

			output = value2;
			auto serial_2 = output.serial();
			test_asserts::verify (desc_type<T> ("serial does not increment when value doesn't change"), serial_2 == serial_1);
		}

		// Test serial on connected properties:
		{
			auto io = std::make_unique<ModuleIO>();
			PropertyIn<T> i (io.get(), "input");
			PropertyOut<T> o (io.get(), "output");
			PropertyOut<T> m (io.get(), "middle");
			Module<ModuleIO> mod (std::move (io));

			i << m;
			m << o;

			o = value1;
			i.fetch (get_cycle (1));
			auto serial_0 = i.serial();

			o = value2;
			i.fetch (get_cycle (2));
			auto serial_1 = i.serial();
			test_asserts::verify (desc_type<T> ("serial increments when value changes over connected properties"), serial_1 == serial_0 + 1);

			o = value2;
			i.fetch (get_cycle (3));
			auto serial_2 = i.serial();
			test_asserts::verify (desc_type<T> ("serial does not increment when value doesn't change over connected properties"), serial_2 == serial_1);
		}

		{
			PropertyIn<T> i (&io, "input");
			PropertyOut<T> o (&io, "output");

			i << o;
			test_asserts::verify ("fetch() throws when no Module is assigned",
								  Exception::catch_and_log (g_null_logger, [&]{ i.fetch (get_cycle (1)); }));
		}

		// Test if data is transferred from output to input properties properly:
		{
			auto io = std::make_unique<ModuleIO>();
			PropertyIn<T> i (io.get(), "input");
			PropertyOut<T> o (io.get(), "output");
			PropertyOut<T> m (io.get(), "middle");
			Module<ModuleIO> mod (std::move (io));

			i << m;
			m << o;

			o = value1;
			i.fetch (get_cycle (1));
			test_asserts::verify ("transferring data from output to input properties works (1)", *i == value1);

			o = value2;
			i.fetch (get_cycle (2));
			test_asserts::verify ("transferring data from output to input properties works (2)", *i == value2);

			o = value1;
			i.fetch (get_cycle (2)); // Note the same cycle number as before; value should not change.
			test_asserts::verify ("caching values if cycle-number doesn't change works", *i == value2);

			o = xf::nil;
			i.fetch (get_cycle (3));
			test_asserts::verify ("transferring nil-values from output to input properties works", i.is_nil());
		}

		// Serialization:
		for (auto const& value: { value1, value2 })
		{
			input << ConstantSource (value);
			test_serialization (input, value);

			output = value;
			test_serialization (output, value);
		}

		// Serialization of nil-values:
		{
			input << xf::no_data_source;
			test_nil_serialization (input);

			output = xf::nil;
			test_nil_serialization (output);
		}
	}


static xf::RuntimeTest t1 ("xf::Property behavior", []{
	do_t1<bool> (true, false);
	do_t1<int8_t> (120, -5);
	do_t1<int16_t> (1337, -5);
	do_t1<int32_t> (1337, -5);
	do_t1<int64_t> (1337, -5);
	do_t1<uint8_t> (133, 5);
	do_t1<uint16_t> (1337, 5);
	do_t1<uint32_t> (1337, 5);
	do_t1<uint64_t> (1337, 5);
	do_t1<float16_t> (0.125_half, 0.0_half);
	do_t1<float32_t> (0.125f, 0.0f);
	do_t1<float64_t> (0.125, 0.0);
	do_t1<std::string> ("value-1", "value-2");
});

} // namespace xf::test


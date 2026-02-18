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

#ifndef XEFIS__CORE__TESTS__TEST_ENUM_H__INCLUDED
#define XEFIS__CORE__TESTS__TEST_ENUM_H__INCLUDED

// Standard:
#include <cstddef>


namespace xf::test {
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


template<std::same_as<TestEnum> Enum>
	Enum
	parse (std::string_view const str)
	{
		if (str == "Value1")
			return TestEnum::Value1;
		else if (str == "Value2")
			return TestEnum::Value2;
		else
			throw nu::Exception ("invalid enum string \"" + std::string (str) + "\"");
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


template<std::same_as<TestEnumWithNil> Enum>
	Enum
	parse (std::string_view const str)
	{
		if (str == "Value1")
			return TestEnumWithNil::Value1;
		else if (str == "Value2")
			return TestEnumWithNil::Value2;
		else if (str == "")
			return TestEnumWithNil::xf_nil_value;
		else
			throw nu::Exception ("invalid enum string \"" + std::string (str) + "\"");
	}

} // namespace test_enum
} // namespace xf::test

#endif

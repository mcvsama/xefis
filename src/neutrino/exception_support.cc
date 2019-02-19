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
#include <any>
#include <cstddef>
#include <functional>
#include <future>
#include <optional>
#include <regex>
#include <stdexcept>
#include <string>
#include <variant>

// Boost:
#include <boost/format.hpp>

// Local:
#include "exception_support.h"


namespace neutrino {

std::string
describe_exception (std::exception_ptr eptr)
{
	using namespace std::literals::string_literals;

	if (!eptr)
		return "<no exception>";
	else
	{
		try {
			std::rethrow_exception (eptr);
		}
		// boost::format exceptions:
		catch (boost::io::too_few_args& e)
		{
			return "format: too few args: "s + e.what();
		}
		catch (boost::io::too_many_args& e)
		{
			return "format: too many args: "s + e.what();
		}
		catch (boost::io::bad_format_string& e)
		{
			return "format: ill formed: "s + e.what();
		}
		// std::logic_error derivatives:
		catch (std::invalid_argument& e)
		{
			return "std: invalid argument: "s + e.what();
		}
		catch (std::domain_error& e)
		{
			return "std: domain error: "s + e.what();
		}
		catch (std::length_error& e)
		{
			return "std: length error: "s + e.what();
		}
		catch (std::out_of_range& e)
		{
			return "std: out of range: "s + e.what();
		}
		catch (std::future_error& e)
		{
			return "std: future error: "s + e.what();
		}
		catch (std::logic_error& e)
		{
			return "std: logic error: "s + e.what();
		}
		// std::runtime_error derivatives:
		catch (std::range_error& e)
		{
			return "std: range error: "s + e.what();
		}
		catch (std::overflow_error& e)
		{
			return "std: overflow error: "s + e.what();
		}
		catch (std::underflow_error& e)
		{
			return "std: underflow error: "s + e.what();
		}
		catch (std::regex_error& e)
		{
			return "std: regex error: "s + e.what();
		}
		catch (std::ios_base::failure& e)
		{
			return "std: ios_base failure: "s + e.what();
		}
		catch (std::system_error& e)
		{
			return "std: system error: "s + e.what();
		}
		catch (std::runtime_error& e)
		{
			return "std: runtime error: "s + e.what();
		}
		// Generic:
		catch (std::bad_optional_access& e)
		{
			return "std: bad optional access: "s + e.what();
		}
		catch (std::bad_typeid& e)
		{
			return "std: bad typeid(): "s + e.what();
		}
		catch (std::bad_any_cast& e)
		{
			return "std: bad any_cast(): "s + e.what();
		}
		catch (std::bad_cast& e)
		{
			return "std: bad cast: "s + e.what();
		}
		catch (std::bad_weak_ptr& e)
		{
			return "std: bad weak_ptr<>: "s + e.what();
		}
		catch (std::bad_function_call& e)
		{
			return "std: bad function call: "s + e.what();
		}
		catch (std::bad_array_new_length& e)
		{
			return "std: bad array new length: "s + e.what();
		}
		catch (std::bad_alloc& e)
		{
			return "std: bad alloc: "s + e.what();
		}
		catch (std::bad_exception& e)
		{
			return "std: bad exception: "s + e.what();
		}
		catch (std::bad_variant_access& e)
		{
			return "std: bad variant access: "s + e.what();
		}
		// Most generic:
		catch (std::exception& e)
		{
			return "generic exception: "s + e.what();
		}
		catch (...)
		{
			return "<unknown exception>";
		}
	}
}


std::optional<std::string>
handle_format_exception (std::function<void()> try_block)
{
	try {
		try_block();
	}
	catch (...)
	{
		return describe_exception (std::current_exception());
	}

	return std::nullopt;
}

} // namespace neutrino


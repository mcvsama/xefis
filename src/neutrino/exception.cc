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

// Boost:
#include <boost/lexical_cast.hpp>

// Neutrino:
#include <neutrino/logger.h>
#include <neutrino/backtrace.h>

// Local:
#include "exception.h"


namespace neutrino {

template<class E>
	static inline void
	print_exception (std::ostream& os, E const& e)
	{
		if constexpr (std::is_base_of_v<Exception, E>)
			os << e.message();
		else if constexpr (std::is_base_of_v<boost::exception, E>)
			os << "boost::exception " << demangle (typeid (e));
		else if constexpr (std::is_base_of_v<std::exception, E>)
			os << e.what();
		else
			os << "unknown exception";

		try {
			std::rethrow_if_nested (e);
		}
		catch (Exception const& nested)
		{
			os << "; cause: ";
			print_exception (os, nested);
		}
		catch (boost::exception const& nested)
		{
			os << "; cause: ";
			print_exception (os, nested);
		}
		catch (std::exception const& nested)
		{
			os << "; cause: ";
			print_exception (os, nested);
		}
		catch (...)
		{
			os << "; caused by unknown nested exception";
		}
	}


void
Exception::log (Logger const& logger, std::function<void()> guarded_code)
{
	try {
		guarded_code();
	}
	catch (...)
	{
		using namespace exception_ops;

		logger << "Exception: " << std::current_exception() << std::endl;
		throw;
	}
}


bool
Exception::catch_and_log (Logger const& logger, std::function<void()> guarded_code)
{
	bool has_thrown = false;

	try {
		guarded_code();
	}
	catch (...)
	{
		using namespace exception_ops;

		has_thrown = true;
		logger << "Exception: " << std::current_exception() << std::endl;
	}

	return has_thrown;
}


[[noreturn]]
void
Exception::terminate (std::string_view message)
{
	std::cerr << "\n----- TERMINATE -----\n" << message << std::endl;
	std::terminate();
}


namespace exception_ops {

std::ostream&
operator<< (std::ostream& os, Exception const& e)
{
	print_exception (os, e);

	if (!e.backtrace_hidden())
		os << std::endl << e.backtrace();

	return os;
}


std::ostream&
operator<< (std::ostream& os, boost::exception const& e)
{
	print_exception (os, e);
	return os;
}


std::ostream&
operator<< (std::ostream& os, std::exception const& e)
{
	print_exception (os, e);
	return os;
}


std::ostream&
operator<< (std::ostream& os, std::exception_ptr const& eptr)
{
	try {
		if (eptr)
			std::rethrow_exception (eptr);
	}
	catch (Exception const& e)
	{
		os << e;
	}
	catch (boost::exception const& e)
	{
		os << e;
	}
	catch (std::exception const& e)
	{
		os << e;
	}
	catch (...)
	{
		os << "unknown exception";
	}

	return os;
}

} // namespace exception_ops
} // namespace neutrino


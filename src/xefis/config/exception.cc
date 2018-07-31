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

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "exception.h"


namespace xf {

void
Exception::guard_and_rethrow (std::function<void()> guarded_code)
{
	try {
		guarded_code();
	}
	catch (Exception const& e)
	{
		// TODO use print_exception instead
		std::cerr << e << std::endl;
		throw;
	}
	catch (boost::exception const& e)
	{
		std::cerr << "boost::exception " << demangle (typeid (e)) << std::endl;
		throw;
	}
	catch (std::exception const& e)
	{
		std::cerr << "std::exception " << demangle (typeid (e)) << std::endl;
		throw;
	}
	catch (...)
	{
		std::cerr << "unknown exception" << std::endl;
		throw;
	}
}


template<class E>
	static void
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


std::ostream&
operator<< (std::ostream& os, Exception const& e)
{
	print_exception (os, e);

	if (!e.backtrace_hidden())
		os << std::endl << e.backtrace();

	return os;
}


namespace exception_ops {

std::ostream&
operator<< (std::ostream& out, std::exception_ptr const& eptr)
{
	try {
		if (eptr)
			std::rethrow_exception (eptr);
	}
	catch (Exception const& e)
	{
		out << e << std::endl;
	}
	catch (boost::exception const& e)
	{
		out << "boost::exception " << demangle (typeid (e)) << std::endl;
	}
	catch (std::exception const& e)
	{
		out << "std::exception " << demangle (typeid (e)) << std::endl;
	}
	catch (...)
	{
		out << "unknown exception" << std::endl;
	}

	return out;
}

} // namespace exception_ops
} // namespace xf


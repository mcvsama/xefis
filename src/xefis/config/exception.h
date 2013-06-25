/* vim:ts=4
 *
 * Copyleft 2012…2013  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 * --
 * Here be basic, global functions and macros like asserts, debugging helpers, etc.
 */

#ifndef XEFIS__CONFIG__EXCEPTION_H__INCLUDED
#define XEFIS__CONFIG__EXCEPTION_H__INCLUDED

// Standard:
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <functional>

// Xefis:
#include <xefis/utility/backtrace.h>


namespace Xefis {

class Exception: public std::runtime_error
{
  public:
	Exception (std::string const& message, Exception const* inner = nullptr);

	virtual ~Exception() noexcept;

	bool
	has_inner() const noexcept;

	std::string const&
	message() const;

	std::string const&
	inner_message() const;

	Backtrace const&
	backtrace() const;

	static void
	guard (std::function<void()> guarded_code);

	static void
	guard_and_rethrow (std::function<void()> guarded_code);

  private:
	bool		_has_inner = false;
	std::string	_message;
	std::string	_inner_message;
	Backtrace	_backtrace;
};


inline std::ostream&
operator<< (std::ostream& os, Exception const& e)
{
	os << e.message() << std::endl;
	os << e.backtrace() << std::endl;
	return os;
}


inline
Exception::Exception (std::string const& message, Exception const* inner):
	std::runtime_error (message),
	_message ("Error: " + message)
{
	if (inner)
	{
		_message += "\n" + inner->message();
		_inner_message = inner->message();
		_backtrace = inner->backtrace();
	}
}


inline
Exception::~Exception() noexcept
{ }


inline bool
Exception::has_inner() const noexcept
{
	return _has_inner;
}


inline std::string const&
Exception::message() const
{
	return _message;
}


inline std::string const&
Exception::inner_message() const
{
	return _inner_message;
}


inline Backtrace const&
Exception::backtrace() const
{
	return _backtrace;
}


inline void
Exception::guard (std::function<void()> guarded_code)
{
	try {
		guard_and_rethrow (guarded_code);
	}
	catch (...)
	{ }
}


inline void
Exception::guard_and_rethrow (std::function<void()> guarded_code)
{
	try {
		guarded_code();
	}
	catch (Exception const& e)
	{
		std::clog << e << std::endl;
		throw;
	}
	catch (boost::exception const& e)
	{
		std::clog << "boost::exception " << typeid (e).name() << std::endl;
		throw;
	}
	catch (std::exception const& e)
	{
		std::clog << "std::exception " << typeid (e).name() << std::endl;
		throw;
	}
	catch (...)
	{
		std::clog << "unknown exception" << std::endl;
		throw;
	}
}

} // namespace Xefis

#endif

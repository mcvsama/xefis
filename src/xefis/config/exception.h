/* vim:ts=4
 *
 * Copyleft 2012…2014  Michał Gawron
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

// Qt:
#include <QtCore/QString>

// Xefis:
#include <xefis/utility/backtrace.h>


namespace Xefis {

class Exception: public std::exception
{
  public:
	/**
	 * Create exception object.
	 * \param	message
	 * 			Message for user. Don't capitalize first letter, and don't add dot at the end.
	 * 			It should be a simple phrase, that can be embedded into a bigger sentence.
	 */
	Exception (const char* message, Exception const* inner = nullptr);

	/**
	 * Convenience function.
	 */
	Exception (std::string const& message, Exception const* inner = nullptr);

	/**
	 * Convenience function.
	 */
	Exception (QString const& message, Exception const* inner = nullptr);

	// Dtor
	virtual ~Exception() noexcept;

	// Ctor
	Exception (Exception const&) = default;

	/**
	 * Return true if this exception wraps another exception.
	 */
	bool
	has_inner() const noexcept;

	/**
	 * Return exception message.
	 */
	std::string const&
	message() const;

	/**
	 * Return message of the wrapped exception, if there's any.
	 * If there is none, return empty string.
	 */
	std::string const&
	inner_message() const;

	/**
	 * Return backtrace created when the exception was constructed.
	 */
	Backtrace const&
	backtrace() const;

	/**
	 * Same as guard_and_rethrow, but doesn't rethrow. Instead it returns
	 * true if exception occured, and false otherwise.
	 */
	static bool
	guard (std::function<void()> guarded_code);

	/**
	 * Execute guarded_code and catch exceptions. If exception is catched,
	 * it's logged and rethrown. If exception is of type Xefis::Exceptions,
	 * it's full message is logged (backtrace, etc). Boost and standard
	 * exceptions are logged just by their typeid().name. Other types
	 * just cause mentioning an exception.
	 */
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
Exception::Exception (const char* message, Exception const* inner):
	Exception (std::string (message), inner)
{ }


inline
Exception::Exception (std::string const& message, Exception const* inner):
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
Exception::Exception (QString const& message, Exception const* inner):
	Exception (message.toStdString(), inner)
{ }


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


inline bool
Exception::guard (std::function<void()> guarded_code)
{
	try {
		guard_and_rethrow (guarded_code);
	}
	catch (...)
	{
		return true;
	}
	return false;
}


inline void
Exception::guard_and_rethrow (std::function<void()> guarded_code)
{
	try {
		guarded_code();
	}
	catch (Exception const& e)
	{
		std::cerr << e << std::endl;
		throw;
	}
	catch (boost::exception const& e)
	{
		std::cerr << "boost::exception " << typeid (e).name() << std::endl;
		throw;
	}
	catch (std::exception const& e)
	{
		std::cerr << "std::exception " << typeid (e).name() << std::endl;
		throw;
	}
	catch (...)
	{
		std::cerr << "unknown exception" << std::endl;
		throw;
	}
}

} // namespace Xefis

#endif

/* vim:ts=4
 *
 * Copyleft 2008…2012  Michał Gawron
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
#include <stdexcept>


namespace Xefis {

class Exception: public std::runtime_error
{
  public:
	Exception (std::string const& message, Exception* inner = nullptr);

	virtual ~Exception() noexcept;

	Exception*
	inner() const;

  private:
	Exception* _inner = nullptr;
};


inline
Exception::Exception (std::string const& message, Exception* inner):
	std::runtime_error (message),
	_inner (inner)
{ }


inline
Exception::~Exception() noexcept
{
	delete _inner;
}


inline Exception*
Exception::inner() const
{
	return _inner;
}

} // namespace Xefis

#endif

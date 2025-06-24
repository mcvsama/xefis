/* vim:ts=4
 *
 * Copyleft 2021  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__CORE__SOCKETS__EXCEPTION_H__INCLUDED
#define XEFIS__CORE__SOCKETS__EXCEPTION_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/exception.h>

// Standard:
#include <cstddef>


namespace xf {

class BasicSocket;


/**
 * Exception object thrown when trying to read a nil socket.
 */
class NilValueException: public nu::Exception
{
  public:
	// Ctor
	explicit
	NilValueException (BasicSocket const& socket);
};

} // namespace xf

#endif


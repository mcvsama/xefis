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

#ifndef XEFIS__SUPPORT__SOCKETS__SOCKET_ACTION_H__INCLUDED
#define XEFIS__SUPPORT__SOCKETS__SOCKET_ACTION_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/cycle.h>

// Standard:
#include <cstddef>


namespace xf {

/**
 * Common interface for Socket*Action classes.
 */
class SocketAction
{
  public:
	// Dtor
	virtual
	~SocketAction() = default;

	virtual void
	process() = 0;
};

} // namespace xf

#endif


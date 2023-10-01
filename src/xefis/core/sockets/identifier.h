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

#ifndef XEFIS__CORE__SOCKETS__IDENTIFIER_H__INCLUDED
#define XEFIS__CORE__SOCKETS__IDENTIFIER_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/sockets/module_socket.h>

// Standard:
#include <cstddef>


namespace xf {

inline std::string
identifier (BasicSocket const& socket)
{
	if (auto const* module_socket = dynamic_cast<BasicModuleSocket const*> (&socket))
		return module_socket->path().string();
	else
		return "(unnamed socket)";
}


inline std::string
identifier (BasicSocket const* socket)
{
	return socket ? identifier (*socket) : "(nullptr)";
}

} // namespace xf

#endif


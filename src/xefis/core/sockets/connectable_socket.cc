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

// Local:
#include "connectable_socket.h"

// Xefis:
#include <xefis/core/xefis.h>

// Standard:
#include <cstddef>


namespace xf {

std::optional<nu::Logger> g_connectable_socket_exception_logger;


nu::Logger const*
connectable_socket_fetch_exception_logger()
{
	if (g_connectable_socket_exception_logger)
		return &*g_connectable_socket_exception_logger;
	else
		return nullptr;
}


void
set_connectable_socket_fetch_exception_logger (nu::Logger const* logger)
{
	if (logger)
		g_connectable_socket_exception_logger = *logger;
	else
		g_connectable_socket_exception_logger.reset();
}

} // namespace xf


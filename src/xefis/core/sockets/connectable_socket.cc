/* vim:ts=4
 *
 * Copyleft 2012  Michał Gawron
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

namespace global {

std::optional<nu::Logger> connectable_socket_exception_logger;

} // namespace global


nu::Logger const*
connectable_socket_fetch_exception_logger()
{
	if (global::connectable_socket_exception_logger)
		return &*global::connectable_socket_exception_logger;
	else
		return nullptr;
}


void
set_connectable_socket_fetch_exception_logger (nu::Logger const* logger)
{
	if (logger)
		global::connectable_socket_exception_logger = *logger;
	else
		global::connectable_socket_exception_logger.reset();
}

} // namespace xf


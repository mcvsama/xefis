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
#include "exception.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/sockets/basic_module_socket.h>

// Standard:
#include <cstddef>


namespace xf {

std::string
make_nil_value_exception_message (BasicSocket const& socket)
{
	if (auto const* basic_module_socket = dynamic_cast<BasicModuleSocket const*> (&socket))
		return std::format ("tried to read a nil socket '{}'", basic_module_socket->path().string());
	else
		return "tried to read a nil socket";
}


NilValueException::NilValueException (BasicSocket const& socket):
	Exception (make_nil_value_exception_message (socket))
{ }

} // namespace xf


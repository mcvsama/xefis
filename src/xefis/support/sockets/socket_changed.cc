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
#include "socket_changed.h"

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>


namespace xf {

bool
SocketChanged::serial_changed()
{
	return perhaps_shift_cycles() && _prev_serial != _curr_serial;
}


bool
SocketChanged::perhaps_shift_cycles()
{
	auto const next_serial = _socket.serial();

	if (next_serial > _curr_serial)
	{
		_prev_serial = _curr_serial;
		_curr_serial = next_serial;
		return true;
	}
	else
		return false;
}

} // namespace xf


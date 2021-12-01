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

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "socket_changed.h"


namespace xf {

bool
SocketChanged::serial_changed (Cycle const& cycle)
{
	perhaps_shift_cycles (cycle);
	return _prev_serial != _curr_serial;
}


bool
SocketChanged::perhaps_shift_cycles (Cycle const& cycle)
{
	if (cycle.number() > _curr_cycle_number)
	{
		_prev_cycle_number = _curr_cycle_number;
		_curr_cycle_number = cycle.number();
		return true;
	}
	else
		return false;
}

} // namespace xf


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
#include "socket_button.h"

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>


namespace xf {

SocketButton::SocketButton (Socket<bool>& socket, CallbackVariant const& callback):
	_socket (socket),
	_callback (callback)
{ }


void
SocketButton::process()
{
	auto const current_state = _socket.value_or (false);

	std::visit (overload {
		[&] (PressCallback const& press_callback) {
			if (current_state && !_last_state)
				press_callback();
		},
		[&] (ChangeCallback const& change_callback) {
			if (current_state != _last_state)
				change_callback (current_state);
		},
	}, _callback);

	_last_state = current_state;
}

} // namespace xf


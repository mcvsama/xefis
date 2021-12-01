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

#ifndef XEFIS__SUPPORT__SOCKETS__SOCKET_BUTTON_H__INCLUDED
#define XEFIS__SUPPORT__SOCKETS__SOCKET_BUTTON_H__INCLUDED

// Standard:
#include <cstddef>
#include <functional>
#include <variant>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/sockets/socket.h>


namespace xf {

/**
 * Observes a Socket<bool> and executes a function when the value becomes true (button press)
 * or just changes (press or release).
 */
class SocketButton
{
  public:
	using PressCallback = std::function<void()>;
	using ChangeCallback = std::function<void (bool)>;
	using CallbackVariant = std::variant<PressCallback, ChangeCallback>;

  public:
	// Ctor
	explicit
	SocketButton (Socket<bool>&, CallbackVariant const&);

	void
	process();

  private:
	Socket<bool> const&	_socket;
	CallbackVariant		_callback;
	bool				_last_state { false };
};

} // namespace xf

#endif


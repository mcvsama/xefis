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

#ifndef XEFIS__SUPPORT__SOCKETS__SOCKET_CHANGED_H__INCLUDED
#define XEFIS__SUPPORT__SOCKETS__SOCKET_CHANGED_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/cycle.h>
#include <xefis/core/sockets/basic_socket.h>


namespace xf {

/**
 * Base for change-observing objects.
 */
class SocketChanged
{
  public:
	// Ctor
	explicit
	SocketChanged (BasicSocket& socket):
		_socket (socket)
	{ }

	// Dtor
	virtual
	~SocketChanged() = default;

	/**
	 * Return true if socket's serial number changed since last cycle.
	 */
	[[nodiscard]]
	bool
	serial_changed();

	[[nodiscard]]
	BasicSocket&
	socket() noexcept
		{ return _socket; }

	[[nodiscard]]
	BasicSocket const&
	socket() const noexcept
		{ return _socket; }

  protected:
	virtual bool
	perhaps_shift_cycles();

  private:
	BasicSocket&		_socket;
	BasicSocket::Serial	_prev_serial	{ 0 };
	BasicSocket::Serial	_curr_serial	{ 0 };
};

} // namespace xf

#endif


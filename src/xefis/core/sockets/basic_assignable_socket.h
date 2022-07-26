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

#ifndef XEFIS__CORE__SOCKETS__BASIC_ASSIGNABLE_SOCKET_H__INCLUDED
#define XEFIS__CORE__SOCKETS__BASIC_ASSIGNABLE_SOCKET_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/sockets/basic_socket.h>

// Standard:
#include <cstddef>
#include <cstdint>


namespace xf {

class BasicAssignableSocket: virtual public BasicSocket
{
  public:
	/**
	 * Set socket to nil-value.
	 */
	virtual void
	operator= (Nil) = 0;

	/**
	 * Unserialize socket's value from string.
	 */
	virtual void
	from_string (std::string_view const&, SocketConversionSettings const& = {}) = 0;

	/**
	 * Unserialize socket's value from Blob.
	 *
	 * \throw	InvalidBlobSize
	 *			If blob has size not corresponding to this socket type.
	 */
	virtual void
	from_blob (BlobView const&) = 0;
};

} // namespace xf

#endif


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

#ifndef XEFIS__CORE__SOCKETS__BASIC_MODULE_SOCKET_H__INCLUDED
#define XEFIS__CORE__SOCKETS__BASIC_MODULE_SOCKET_H__INCLUDED

// Standard:
#include <cstddef>
#include <string_view>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/module_io.h>
#include <xefis/core/sockets/basic_socket.h>
#include <xefis/core/sockets/module_socket_path.h>


namespace xf {

/**
 * Base class for all ModuleSocket* types.
 * ModuleIn and ModuleOut belong to ModuleIO, allow ModuleIO to be aware of those sockets.
 */
class BasicModuleSocket: virtual public BasicSocket
{
  protected:
	/**
	 * Create ModuleSocket that doesn't have any data-source yet and is not coupled to any module.
	 */
	explicit
	BasicModuleSocket (std::string_view const& path);

	/**
	 * Create ModuleSocket that's coupled by a ModuleIO.
	 *
	 * \param	owner
	 *			Owner object for this socket. May be nullptr.
	 */
	explicit
	BasicModuleSocket (ModuleIO* owner, std::string_view const& path);

  public:
	// Dtor
	virtual
	~BasicModuleSocket() = default;

	/**
	 * Return socket owner (an ModuleIO object). May be nullptr.
	 */
	[[nodiscard]]
	ModuleIO*
	io() const noexcept
		{ return _owner; }

	/**
	 * Return socket path.
	 */
	[[nodiscard]]
	ModuleSocketPath const&
	path() const noexcept
		{ return _path; }

	/**
	 * Deregisters socket from ModuleIO: resets pointer to IO owner and makes it impossible
	 * to use this socket again. Use in preparation for destroy in non-standard order
	 * (eg. when ModuleIO has to be destroyed first).
	 */
	virtual void
	deregister() = 0;

  protected:
	ModuleIO*			_owner { nullptr };
	ModuleSocketPath	_path;
};


/*
 * BasicModuleSocket
 */


inline
BasicModuleSocket::BasicModuleSocket (std::string_view const& path):
	_path (path)
{ }


inline
BasicModuleSocket::BasicModuleSocket (ModuleIO* owner, std::string_view const& path):
	_owner (owner),
	_path (path)
{ }

} // namespace xf


#include <xefis/core/sockets/module_in.h>
#include <xefis/core/sockets/module_out.h>

#endif


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

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/sockets/basic_socket.h>
#include <xefis/core/sockets/module_socket_path.h>

// Standard:
#include <cstddef>
#include <string_view>


namespace xf {

/**
 * Base class for all ModuleSocket* types.
 * ModuleIn and ModuleOut belong to Module and allow Module to be aware of those sockets.
 */
class BasicModuleSocket: virtual public BasicSocket
{
  protected:
	/**
	 * Create ModuleSocket that's coupled by a Module.
	 *
	 * \param	owner
	 *			Owner object for this socket. May be nullptr.
	 */
	explicit
	BasicModuleSocket (Module* owner, std::string_view const& path);

  public:
	// Dtor
	virtual
	~BasicModuleSocket() = default;

	/**
	 * Return socket owner (a Module object). May be nullptr.
	 */
	[[nodiscard]]
	Module&
	module() const noexcept
		{ return *_module; }

	/**
	 * Return socket path.
	 */
	[[nodiscard]]
	ModuleSocketPath const&
	path() const noexcept
		{ return _path; }

	/**
	 * Deregisters socket from Module: resets pointer to owner module and makes it impossible
	 * to use this socket again. Use in preparation for destroy in non-standard order
	 * (eg. when Module has to be destroyed first).
	 */
	virtual void
	deregister() = 0;

  protected:
	Module*				_module;
	ModuleSocketPath	_path;
};


/*
 * BasicModuleSocket
 */


inline
BasicModuleSocket::BasicModuleSocket (Module* owner, std::string_view const& path):
	_module (owner),
	_path (path)
{ }

} // namespace xf


#include <xefis/core/sockets/module_in.h>
#include <xefis/core/sockets/module_out.h>

#endif


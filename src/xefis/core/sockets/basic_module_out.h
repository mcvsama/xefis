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

#ifndef XEFIS__CORE__SOCKETS__BASIC_MODULE_OUT_H__INCLUDED
#define XEFIS__CORE__SOCKETS__BASIC_MODULE_OUT_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/sockets/basic_assignable_socket.h>
#include <xefis/core/sockets/basic_module_socket.h>

// Standard:
#include <cstddef>
#include <cstdint>


namespace xf {

/**
 * Base class for all ModuleOut<T>
 */
class BasicModuleOut:
	public BasicModuleSocket,
	virtual public BasicAssignableSocket
{
  public:
	using BasicModuleSocket::BasicModuleSocket;
	using BasicAssignableSocket::operator=;
};

} // namespace xf

#endif


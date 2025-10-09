/* vim:ts=4
 *
 * Copyleft 2020  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__CORE__SOCKETS__MODULE_SOCKET_H__INCLUDED
#define XEFIS__CORE__SOCKETS__MODULE_SOCKET_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/sockets/basic_module_socket.h>
#include <xefis/core/sockets/module_in.h>
#include <xefis/core/sockets/module_out.h>

// Neutrino:
#include <neutrino/types.h>


namespace xf {

template<class Socket>
	concept ModuleInOutConcept = neutrino::is_specialization_v<Socket, ModuleIn> || neutrino::is_specialization_v<Socket, ModuleOut>;

template<template<class> class SocketTemplate>
	concept ModuleInTemplateConcept = std::same_as<SocketTemplate<int>, ModuleIn<int>>;

template<template<class> class SocketTemplate>
	concept ModuleOutTemplateConcept = std::same_as<SocketTemplate<int>, ModuleOut<int>>;

template<template<class> class SocketTemplate>
	concept ModuleInOutTemplateConcept = ModuleInTemplateConcept<SocketTemplate> || ModuleOutTemplateConcept<SocketTemplate>;

} // namespace xf

#endif


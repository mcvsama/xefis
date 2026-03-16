/* vim:ts=4
 *
 * Copyleft 2026  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__MACHINES__SIM_1__AIRCRAFT__COMMON__LINK__FLIGHT_COMPUTER_TO__RADIO_DATA_H__INCLUDED
#define XEFIS__MACHINES__SIM_1__AIRCRAFT__COMMON__LINK__FLIGHT_COMPUTER_TO__RADIO_DATA_H__INCLUDED

// Xefis:
#include <xefis/core/module.h>
#include <xefis/core/processing_loop.h>
#include <xefis/core/sockets/module_socket.h>
#include <xefis/modules/comm/link/helpers.h>
#include <xefis/modules/comm/link/link_decoder.h>
#include <xefis/modules/comm/link/link_encoder.h>
#include <xefis/modules/comm/link/link_protocol.h>


namespace sim1 {

/**
 * It's an xf::Module so it can be tracked in ConfiguratorWidget.
 */
template<template<class> class SocketType>
	requires xf::ModuleInOutTemplateConcept<SocketType>
	class FlightComputerToRadioData: public xf::Module
	{
	  public:
		SocketType<si::Pressure>	static_pressure	{ this, "sensors/pressure/static" };
		SocketType<si::Pressure>	total_pressure	{ this, "sensors/pressure/total" };

	  public:
		// Ctor
		using Module::Module;
	};

} // namespace sim1

#endif

/* vim:ts=4
 *
 * Copyleft 2025  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__MACHINES__SIM_1__AIRCRAFT__COMMON__LINK__FLIGHT_COMPUTER_TO__HARDWARE_DATA_H__INCLUDED
#define XEFIS__MACHINES__SIM_1__AIRCRAFT__COMMON__LINK__FLIGHT_COMPUTER_TO__HARDWARE_DATA_H__INCLUDED

// Xefis:
#include <xefis/core/module.h>
#include <xefis/core/sockets/module_socket.h>
#include <xefis/core/processing_loop.h>
#include <xefis/modules/comm/link/helpers.h>
#include <xefis/modules/comm/link/link_decoder.h>
#include <xefis/modules/comm/link/link_encoder.h>
#include <xefis/modules/comm/link/link_protocol.h>


namespace sim1 {

using namespace std::string_literals;


/**
 * It's an xf::Module so it can be tracked in ConfiguratorWidget.
 */
template<template<class> class SocketType>
	requires xf::ModuleInOutTemplateConcept<SocketType>
	class FlightComputerToHardwareData: public xf::Module
	{
	  public:
		SocketType<si::Angle>	left_aileron	{ this, "flight-controls/left-aileron" };
		SocketType<si::Angle>	right_aileron	{ this, "flight-controls/right-aileron" };
		SocketType<si::Angle>	elevator		{ this, "flight-controls/elevator" };
		SocketType<si::Angle>	rudder			{ this, "flight-controls/rudder" };

	  public:
		// Ctor
		using Module::Module;
	};

} // namespace sim1

#endif

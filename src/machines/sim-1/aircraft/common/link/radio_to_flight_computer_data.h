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

#ifndef XEFIS__MACHINES__SIM_1__AIRCRAFT__COMMON__LINK__RADIO_TO__FLIGHT_COMPUTER_DATA_H__INCLUDED
#define XEFIS__MACHINES__SIM_1__AIRCRAFT__COMMON__LINK__RADIO_TO__FLIGHT_COMPUTER_DATA_H__INCLUDED

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
	class RadioToFlightComputerData: public xf::Module
	{
	  public:
		SocketType<double>	joystick_pitch	{ this, "joystick/pitch" };
		SocketType<double>	joystick_roll	{ this, "joystick/roll" };
		SocketType<double>	joystick_yaw	{ this, "joystick/yaw" };
		SocketType<double>	trim_pitch		{ this, "trim/pitch" };
		SocketType<double>	trim_roll		{ this, "trim/roll" };
		SocketType<double>	trim_yaw		{ this, "trim/yaw" };
		SocketType<double>	throttle_left	{ this, "throttle/left" };
		SocketType<double>	throttle_right	{ this, "throttle/right" };

	  public:
		// Ctor
		using Module::Module;
	};

} // namespace sim1

#endif

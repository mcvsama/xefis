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
#include <xefis/modules/comm/link/input_link.h>
#include <xefis/modules/comm/link/link_protocol.h>
#include <xefis/modules/comm/link/output_link.h>


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

	  private:
		static inline auto const kEnvelopePrefix = Blob { 0x34, 0x45 };
		static inline auto const kDir = "flight computer → hardware"s;
		static inline auto const kContext = kDir + " (link)";

	  public:
		// Ctor
		using Module::Module;

		InputLink
		make_link_decoder (xf::ProcessingLoop& loop, nu::Logger const& logger)
		{
			return InputLink(
				loop,
				make_link_protocol_from_outputs (*this, kDir, kEnvelopePrefix),
				{},
				logger.with_context (kContext),
				kContext
			);
		}

		OutputLink
		make_link_encoder (xf::ProcessingLoop& loop, nu::Logger const& logger)
		{
			return OutputLink(
				loop,
				make_link_protocol_from_inputs (*this, kDir, kEnvelopePrefix),
				120_Hz,
				logger.with_context (kContext),
				kContext
			);
		}
	};

} // namespace sim1

#endif

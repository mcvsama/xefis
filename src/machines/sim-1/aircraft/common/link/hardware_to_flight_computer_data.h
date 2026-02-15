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

#ifndef XEFIS__MACHINES__SIM_1__AIRCRAFT__COMMON__LINK__HARDWARE_TO_FLIGHT_COMPUTER_DATA_H__INCLUDED
#define XEFIS__MACHINES__SIM_1__AIRCRAFT__COMMON__LINK__HARDWARE_TO_FLIGHT_COMPUTER_DATA_H__INCLUDED

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
	class HardwareToFlightComputerData: public xf::Module
	{
	  public:
		SocketType<si::Temperature>	air_total_temperature	{ this, "sensors/temperature/total-temperature" };
		SocketType<si::Pressure>	air_total_pressure		{ this, "sensors/pressure/total-pressure" };
		SocketType<si::Pressure>	air_static_pressure		{ this, "sensors/pressure/static-pressure" };

	  private:
		static inline auto const kEnvelopePrefix = Blob { 0x12, 0x23 };
		static inline auto const kDir = "hardware → flight computer"s;
		static inline auto const kContext = kDir + " (link)";

	  public:
		// Ctor
		using Module::Module;

		LinkDecoder
		make_link_decoder (xf::ProcessingLoop& loop, nu::Logger const& logger)
		{
			return LinkDecoder(
				loop,
				make_link_protocol_from_outputs (*this, kDir, kEnvelopePrefix),
				{},
				logger.with_context (kContext),
				kContext
			);
		}

		LinkEncoder
		make_link_encoder (xf::ProcessingLoop& loop, nu::Logger const& logger)
		{
			return LinkEncoder(
				loop,
				make_link_protocol_from_inputs (*this, kDir, kEnvelopePrefix),
				{ .send_frequency = 120_Hz },
				logger.with_context (kContext),
				kContext
			);
		}
	};

} // namespace sim1

#endif

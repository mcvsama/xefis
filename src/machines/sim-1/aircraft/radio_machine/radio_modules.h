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

#ifndef XEFIS__MACHINES__SIM_1__AIRCRAFT__RADIO_MACHINE__RADIO_MODULES_H__INCLUDED
#define XEFIS__MACHINES__SIM_1__AIRCRAFT__RADIO_MACHINE__RADIO_MODULES_H__INCLUDED

// Sim-1:
#include <machines/sim-1/aircraft/common/link/addresses.h>
#include <machines/sim-1/aircraft/common/link/flight_computer_to_radio_data.h>
#include <machines/sim-1/aircraft/common/link/radio_to_flight_computer_data.h>
#include <machines/sim-1/common/common.h>
#include <machines/sim-1/common/link/addresses.h>
#include <machines/sim-1/common/link/air_to_ground.h>
#include <machines/sim-1/common/link/crypto.h>
#include <machines/sim-1/common/link/ground_to_air.h>

// Xefis:
#include <xefis/core/sockets/module_in.h>
#include <xefis/core/sockets/module_out.h>
#include <xefis/modules/comm/udp_transceiver.h>
#include <xefis/modules/comm/udp_transceiver_with_codec.h>

// Standard:
#include <cstddef>


namespace sim1::aircraft {

class RadioModules
{
  public:
	// Ctor
	explicit
	RadioModules (xf::ProcessingLoop&, nu::Logger const&);

  private:
	nu::Logger			_logger;
	xf::ProcessingLoop&	_loop;

  public:
	sim1::RadioToFlightComputerData<xf::ModuleIn>		radio_to_flight_computer_data	{ _loop };
	sim1::FlightComputerToRadioData<xf::ModuleOut>		flight_computer_to_radio_data	{ _loop };

  private:
	// TODO move this to VirtualRadioModules:
	UDPTransceiver _udp_link_to_ground_station {
		_loop,
		UDPTransceiver::Parameters {
			.rx_udp_address		= global::ground_station_to_aircraft_address,
			.tx_udp_address		= global::aircraft_to_ground_station_address,
			.rx_interference	= false,
			.tx_interference	= false,
		},
		_logger.with_context ("udp"),
		"udp",
	};

	UDPTransceiverWithCodec _udp_link_to_flight_computer_machine {
		_loop,
		UDPTransceiver::Parameters {
			.rx_udp_address = sim1::global::flight_computer_to_radio_address,
			.tx_udp_address = sim1::global::radio_to_flight_computer_address,
		},
		make_link_protocol_from_inputs (radio_to_flight_computer_data, "radio → flight computer", { 0x67, 0x78 }),
		LinkEncoder::Parameters {
			.send_frequency = 120_Hz,
		},
		make_link_protocol_from_outputs (flight_computer_to_radio_data, "flight computer → radio", { 0x45, 0x56 }),
		LinkDecoder::Parameters {
		},
		_logger,
		"flight computer machine",
	};

	// TODO pasted here, restructure it:

	xf::crypto::xle::SlaveSecureChannel _slave_secure_channel {
		_loop,
		sim1::kCryptoParams,
		{}, // TODO Should store used IDs somewhere
		_logger.with_context ("slave secure channel"),
		"slave secure channel",
	};

	sim1::GroundToAirData<xf::ModuleOut> _ground_to_air_data { _loop };

	LinkDecoder _ground_to_air_link {
		_loop,
		std::make_unique<sim1::GroundToAirProtocol> (_ground_to_air_data, _slave_secure_channel),
		{},
		_logger.with_context ("link decoder"),
		"link decoder",
	};

	sim1::AirToGroundData<xf::ModuleIn> _air_to_ground_data { _loop };

	LinkEncoder _air_to_ground_link {
		_loop,
		std::make_unique<sim1::AirToGroundProtocol> (_air_to_ground_data, _slave_secure_channel),
		{ .send_frequency = 30_Hz },
		_logger.with_context ("link encoder"),
		"link encoder",
	};
};

} // namespace sim1::aircraft

#endif

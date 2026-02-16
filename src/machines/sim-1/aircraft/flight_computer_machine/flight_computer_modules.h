/* vim:ts=4
 *
 * Copyleft 2012  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__MACHINES__SIM_1__AIRCRAFT__FLIGHT_COMPUTER_MACHINE__FLIGHT_COMPUTER_MODULES_H__INCLUDED
#define XEFIS__MACHINES__SIM_1__AIRCRAFT__FLIGHT_COMPUTER_MACHINE__FLIGHT_COMPUTER_MODULES_H__INCLUDED

// Sim-1:
#include <machines/sim-1/aircraft/common/link/addresses.h>
#include <machines/sim-1/aircraft/common/link/flight_computer_to_hardware_data.h>
#include <machines/sim-1/aircraft/common/link/hardware_to_flight_computer_data.h>

// Xefis:
#include <xefis/modules/comm/udp_transceiver_with_codec.h>
#include <xefis/modules/systems/air_data_computer.h>

// Standard:
#include <cstddef>


namespace sim1::aircraft {

class FlightComputerModules
{
  public:
	// Ctor
	explicit
	FlightComputerModules (xf::ProcessingLoop&, nu::Logger const&);

  private:
	nu::Logger			_logger;
	xf::ProcessingLoop&	_loop;

	sim1::FlightComputerToHardwareData<xf::ModuleIn>	_flight_computer_to_hardware_data	{ _loop };
	sim1::HardwareToFlightComputerData<xf::ModuleOut>	_hardware_to_flight_computer_data	{ _loop };

  public:
	AirDataComputer air_data_computer { _loop, nullptr, _logger };

	UDPTransceiverWithCodec hardware_machine_transceiver {
		_loop,
		UDPTransceiver::Parameters {
			.rx_udp_address = sim1::global::hardware_to_flight_computer_address,
			.tx_udp_address = sim1::global::flight_computer_to_hardware_address,
		},
		make_link_protocol_from_inputs (_flight_computer_to_hardware_data, "flight computer → hardware", { 0x01, 0x12 }),
		LinkEncoder::Parameters {
			.send_frequency = 120_Hz,
		},
		make_link_protocol_from_outputs (_hardware_to_flight_computer_data, "hardware → flight computer", { 0x23, 0x34 }),
		LinkDecoder::Parameters {
		},
		_logger,
		"hardware machine",
	};
};

} // namespace sim1::aircraft

#endif

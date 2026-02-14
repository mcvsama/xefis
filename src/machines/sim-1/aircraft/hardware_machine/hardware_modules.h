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

#ifndef XEFIS__MACHINES__SIM_1__AIRCRAFT__HARDWARE_MACHINE__HARDWARE_MODULES_H__INCLUDED
#define XEFIS__MACHINES__SIM_1__AIRCRAFT__HARDWARE_MACHINE__HARDWARE_MODULES_H__INCLUDED

// Sim-1:
#include <machines/sim-1/aircraft/common/link/addresses.h>
#include <machines/sim-1/aircraft/common/link/flight_computer_to_hardware_data.h>
#include <machines/sim-1/aircraft/common/link/hardware_to_flight_computer_data.h>

// Xefis:
#include <xefis/modules/comm/udp_transceiver.h>

// Standard:
#include <cstddef>


namespace sim1::aircraft {

class HardwareModules
{
  public:
	// Ctor
	explicit
	HardwareModules (xf::ProcessingLoop&, nu::Logger const&);

	// Dtor
	virtual
	~HardwareModules() = default;

	xf::ProcessingLoop&
	loop() noexcept
		{ return _loop; }

	nu::Logger&
	logger() noexcept
		{ return _logger; }

  private:
	nu::Logger			_logger;
	xf::ProcessingLoop&	_loop;

  protected:
	sim1::HardwareToFlightComputerData<xf::ModuleIn>	hardware_to_flight_computer_data	{ _loop };
	sim1::FlightComputerToHardwareData<xf::ModuleOut>	flight_computer_to_hardware_data	{ _loop };

  public:
	UDPTransceiver flight_computer_machine_transceiver {
		_loop,
		UDPTransceiver::Parameters {
			.rx_udp_address = sim1::global::flight_computer_to_hardware_address,
			.tx_udp_address = sim1::global::hardware_to_flight_computer_address,
		},
		_logger,
		"connection to Flight Computer machine",
	};

	LinkEncoder	hardware_to_flight_computer_data_encoder	{ hardware_to_flight_computer_data.make_link_encoder (_loop, _logger) };
	LinkDecoder	flight_computer_to_hardware_data_decoder	{ flight_computer_to_hardware_data.make_link_decoder (_loop, _logger) };
};

} // namespace sim1::aircraft

#endif

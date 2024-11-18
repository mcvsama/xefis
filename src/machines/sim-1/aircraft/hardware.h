/* vim:ts=4
 *
 * Copyleft 2008…2023  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__MACHINES__SIM_1__AIRCRAFT__HARDWARE_H__INCLUDED
#define XEFIS__MACHINES__SIM_1__AIRCRAFT__HARDWARE_H__INCLUDED

// Machine:
#include <machines/sim-1/common/link/air_to_ground.h>
#include <machines/sim-1/common/link/crypto.h>
#include <machines/sim-1/common/link/ground_to_air.h>

// Xefis:
#include <xefis/core/sockets/module_in.h>
#include <xefis/core/sockets/module_out.h>
#include <xefis/modules/comm/link/input_link.h>
#include <xefis/modules/comm/link/output_link.h>
#include <xefis/modules/comm/udp.h>
#include <xefis/modules/comm/xle_transceiver.h>
#include <xefis/modules/simulation/virtual_servo_controller.h>

// Standard:
#include <cstddef>


namespace sim1::aircraft {

namespace xle = xf::crypto::xle;


class Hardware
{
  public:
	// Ctor
	explicit
	Hardware (xf::ProcessingLoop&, xf::Logger const&);

  private:
	xf::Logger			_logger;
	xf::ProcessingLoop&	_loop;

  public:
	VirtualServoController					servo_controller			{ _loop };
	sim1::GroundToAirData<xf::ModuleOut>	ground_to_air_data			{ _loop };
	sim1::AirToGroundData<xf::ModuleIn>		air_to_ground_data			{ _loop };

	xle::SlaveTransceiver slave_transceiver {
		_loop,
		sim1::kCryptoParams,
		[] (xle::HandshakeID) { return false; }, // TODO Should store used IDs somewhere
		_logger.with_context ("slave transceiver"),
		"slave transceiver",
	};

	InputLink ground_to_air_link {
		_loop,
		std::make_unique<sim1::GroundToAirProtocol> (ground_to_air_data, slave_transceiver),
		{},
		_logger.with_context ("input link"),
		"input link",
	};

	OutputLink air_to_ground_link {
		_loop,
		std::make_unique<sim1::AirToGroundProtocol> (air_to_ground_data, slave_transceiver),
		30_Hz,
		_logger.with_context ("output link"),
		"output link",
	};

	UDP udp_link {
		_loop,
		UDP::Parameters {
			.rx_udp_address		= UDP::Address { "127.0.0.1", 9991 },
			.tx_udp_address		= UDP::Address { "127.0.0.1", 9990 },
			.rx_interference	= false,
			.tx_interference	= false,
		},
		_logger.with_context ("udp"),
		"udp",
	};
};

} // namespace sim1::aircraft

#endif


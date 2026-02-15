/* vim:ts=4
 *
 * Copyleft 2024  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__MACHINES__SIM_1__GROUND_STATION__CONTROL_MACHINE__HARDWARE_H__INCLUDED
#define XEFIS__MACHINES__SIM_1__GROUND_STATION__CONTROL_MACHINE__HARDWARE_H__INCLUDED

// Sim-1:
#include <machines/sim-1/common/link/air_to_ground.h>
#include <machines/sim-1/common/link/crypto.h>
#include <machines/sim-1/common/link/ground_to_air.h>

// Xefis:
#include <xefis/core/machine.h>
#include <xefis/core/sockets/module_in.h>
#include <xefis/core/sockets/module_out.h>
#include <xefis/modules/comm/link/link_decoder.h>
#include <xefis/modules/comm/link/link_encoder.h>
#include <xefis/modules/comm/udp_transceiver.h>
#include <xefis/modules/comm/xle_secure_channel.h>
#include <xefis/modules/simulation/virtual_joystick.h>
#include <xefis/modules/simulation/virtual_servo_controller.h>

// Standard:
#include <cstddef>


namespace sim1::ground_station::control_machine {

namespace xle = xf::crypto::xle;


class Hardware
{
  public:
	// Ctor
	explicit
	Hardware (xf::ProcessingLoop&, xf::Machine*, nu::Logger const&);

  private:
	nu::Logger			_logger;
	xf::ProcessingLoop&	_loop;
	xf::Machine*		_machine;

  public:
	VirtualJoystick							joystick			{ _loop, _machine, "virtual joystick" };
	sim1::GroundToAirData<xf::ModuleIn>		ground_to_air_data	{ _loop };
	sim1::AirToGroundData<xf::ModuleOut>	air_to_ground_data	{ _loop };

	xle::MasterSecureChannel master_secure_channel {
		_loop,
		sim1::kCryptoParams,
		_logger.with_context ("master secure channel"),
		"master secure channel",
	};

	LinkDecoder air_to_ground_link {
		_loop,
		std::make_unique<sim1::AirToGroundProtocol> (air_to_ground_data, master_secure_channel),
		{},
		_logger.with_context ("link decoder"),
		"link decoder",
	};

	LinkEncoder ground_to_air_link {
		_loop,
		std::make_unique<sim1::GroundToAirProtocol> (ground_to_air_data, master_secure_channel),
		{ .send_frequency = 30_Hz },
		_logger.with_context ("link encoder"),
		"link encoder",
	};

	UDPTransceiver udp_link {
		_loop,
		UDPTransceiver::Parameters {
			.rx_udp_address		= UDPTransceiver::Address { "127.0.0.1", 9990 },
			.tx_udp_address		= UDPTransceiver::Address { "127.0.0.1", 9991 },
			.rx_interference	= false,
			.tx_interference	= false,
		},
		_logger.with_context ("udp"),
		"udp",
	};
};

} // namespace sim1::ground_station::control_machine

#endif


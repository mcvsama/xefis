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

#ifndef XEFIS__MACHINES__SIM_1__GROUND_STATION__CONTROL_MACHINE__HARDWARE_H__INCLUDED
#define XEFIS__MACHINES__SIM_1__GROUND_STATION__CONTROL_MACHINE__HARDWARE_H__INCLUDED

// Machine:
#include <machines/sim-1/common/link/air_to_ground.h>
#include <machines/sim-1/common/link/crypto.h>
#include <machines/sim-1/common/link/ground_to_air.h>

// Xefis:
#include <xefis/core/machine.h>
#include <xefis/core/sockets/module_in.h>
#include <xefis/core/sockets/module_out.h>
#include <xefis/modules/comm/link/input_link.h>
#include <xefis/modules/comm/link/output_link.h>
#include <xefis/modules/comm/udp.h>
#include <xefis/modules/comm/xle_transceiver.h>
#include <xefis/modules/simulation/virtual_joystick.h>
// TODO #include <xefis/modules/simulation/virtual_pressure_sensor.h>
#include <xefis/modules/simulation/virtual_servo_controller.h>
// TODO #include <xefis/modules/simulation/virtual_temperature_sensor.h>

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

	xle::MasterTransceiver master_transceiver {
		_loop,
		sim1::kCryptoParams,
		_logger.with_context ("master transceiver"),
		"master transceiver",
	};

	InputLink air_to_ground_link {
		_loop,
		std::make_unique<sim1::AirToGroundProtocol> (air_to_ground_data, master_transceiver),
		{},
		_logger.with_context ("input link"),
		"input link",
	};

	OutputLink ground_to_air_link {
		_loop,
		std::make_unique<sim1::GroundToAirProtocol> (ground_to_air_data, master_transceiver),
		30_Hz,
		_logger.with_context ("output link"),
		"output link",
	};

	UDP udp_link {
		_loop,
		UDP::Parameters {
			.rx_udp_address		= UDP::Address { "127.0.0.1", 9990 },
			.tx_udp_address		= UDP::Address { "127.0.0.1", 9991 },
			.rx_interference	= false,
			.tx_interference	= false,
		},
		_logger.with_context ("udp"),
		"udp",
	};
};

} // namespace sim1::ground_station::control_machine

#endif


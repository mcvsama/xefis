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

#ifndef XEFIS__MACHINES__SIM_1__AIRCRAFT__RADIO_MACHINE__RADIO_MACHINE_H__INCLUDED
#define XEFIS__MACHINES__SIM_1__AIRCRAFT__RADIO_MACHINE__RADIO_MACHINE_H__INCLUDED

// Local:
#include "radio_modules.h"

// Sim-1:
#include <machines/sim-1/aircraft/common/link/flight_computer_to_hardware_data.h>
#include <machines/sim-1/aircraft/common/link/hardware_to_flight_computer_data.h>
#include <machines/sim-1/common/common.h>
#include <machines/sim-1/common/link/air_to_ground.h>
#include <machines/sim-1/common/link/crypto.h>
#include <machines/sim-1/common/link/ground_to_air.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/sockets/module_in.h>
#include <xefis/core/sockets/module_out.h>
#include <xefis/core/clock.h>
#include <xefis/core/xefis.h>
#include <xefis/modules/comm/xle_secure_channel.h>
#include <xefis/support/core/single_loop_machine.h>

// Standard:
#include <cstddef>


namespace sim1::aircraft {

class RadioMachine: public xf::SingleLoopMachine
{
  public:
	// Ctor
	explicit
	RadioMachine (xf::Xefis&, xf::Clock& xefis_clock, xf::Clock& steady_clock, bool inside_simulation);

  private:
	void
	connect_modules() override;

  private:
	xf::Clock&		_xefis_clock;
	xf::Clock&		_steady_clock;
	RadioModules	_modules;

	xf::crypto::xle::SlaveSecureChannel slave_secure_channel {
		loop(),
		sim1::kCryptoParams,
		{}, // TODO Should store used IDs somewhere
		logger().with_context ("slave secure channel"),
		"slave secure channel",
	};

	sim1::GroundToAirData<xf::ModuleOut> ground_to_air_data { loop() };

	LinkDecoder ground_to_air_link {
		loop(),
		std::make_unique<sim1::GroundToAirProtocol> (ground_to_air_data, slave_secure_channel),
		{},
		logger().with_context ("link decoder"),
		"link decoder",
	};

	sim1::AirToGroundData<xf::ModuleIn> air_to_ground_data { loop() };

	LinkEncoder air_to_ground_link {
		loop(),
		std::make_unique<sim1::AirToGroundProtocol> (air_to_ground_data, slave_secure_channel),
		30_Hz,
		logger().with_context ("link encoder"),
		"link encoder",
	};
};

} // namespace sim1::aircraft

#endif

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

#ifndef XEFIS__MACHINES__SIM_1__GROUND_STATION__HARDWARE_MACHINE__MACHINE_H__INCLUDED
#define XEFIS__MACHINES__SIM_1__GROUND_STATION__HARDWARE_MACHINE__MACHINE_H__INCLUDED

// Sim-1:
#include <machines/sim-1/common/common.h>

// Xefis:
#include <xefis/support/core/single_loop_machine.h>

// Neutrino:
#include <neutrino/logger.h>

// Standard:
#include <cstddef>


namespace sim1::ground_station {

class HardwareMachine: public xf::SingleLoopMachine
{
  public:
	// Ctor
	HardwareMachine (xf::Xefis&);

	// SingleLoopMachine API
	void
	connect_modules() override
	{ }
};

} // namespace sim1::ground_station

#endif

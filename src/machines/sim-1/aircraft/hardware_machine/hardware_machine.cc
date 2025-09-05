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

// Local:
#include "hardware_machine.h"

// Sim-1:
#include <machines/sim-1/aircraft/hardware_machine/real_hardware_modules.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/xefis.h>

// Standard:
#include <cstddef>


namespace sim1::aircraft {

using namespace neutrino::si::literals;


HardwareMachine::HardwareMachine (xf::Xefis& xefis,
								  xf::Clock& xefis_clock,
								  xf::Clock& steady_clock):
	HardwareMachine (xefis, xefis_clock, steady_clock, [this](xf::ProcessingLoop&) { return std::make_unique<RealHardwareModules> (loop(), logger()); })
{
	start();
}


HardwareMachine::HardwareMachine (xf::Xefis& xefis,
								  xf::Clock& xefis_clock,
								  xf::Clock& steady_clock,
								  MakeModulesFunction const make_modules):
	xf::SingleLoopMachine (xefis, xefis.logger().with_context ("hardware").with_context ("machine"), 120_Hz, u8"Aircraft/Hardware machine"),
	_xefis_clock (xefis_clock),
	_steady_clock (steady_clock),
	_modules (make_modules (loop()))
{
	initialize();
	// Not starting the machine, since we might be in a simulation. The simulator will
	// manage the loop's time using `ProcessingLoop::advance (si::Time)`.
}


void
HardwareMachine::connect_modules()
{
	// TODO empty?
}

} // namespace sim1::aircraft

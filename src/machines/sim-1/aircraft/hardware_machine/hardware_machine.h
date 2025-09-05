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

#ifndef XEFIS__MACHINES__SIM_1__AIRCRAFT__HARDWARE_MACHINE__HARDWARE_MACHINE_H__INCLUDED
#define XEFIS__MACHINES__SIM_1__AIRCRAFT__HARDWARE_MACHINE__HARDWARE_MACHINE_H__INCLUDED

// Sim-1:
#include <machines/sim-1/aircraft/hardware_machine/hardware_modules.h>
#include <machines/sim-1/common/common.h>
#include <machines/sim-1/simulation/simulated_aircraft.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/sockets/module_in.h>
#include <xefis/core/sockets/module_out.h>
#include <xefis/core/clock.h>
#include <xefis/core/xefis.h>
#include <xefis/modules/comm/link/input_link.h>
#include <xefis/modules/comm/link/output_link.h>
#include <xefis/support/core/single_loop_machine.h>

// Standard:
#include <cstddef>
#include <functional>
#include <memory>


namespace sim1::aircraft {

class HardwareMachine: public xf::SingleLoopMachine
{
  public:
	using MakeModulesFunction = std::function<std::unique_ptr<HardwareModules> (xf::ProcessingLoop&)>;

  public:
	/**
	 * Create machine with RealHardwareModules.
	 */
	explicit
	HardwareMachine (xf::Xefis&, xf::Clock& xefis_clock, xf::Clock& steady_clock);

	/**
	 * Create machine with provided HardwareModules implementation.
	 */
	explicit
	HardwareMachine (xf::Xefis&, xf::Clock& xefis_clock, xf::Clock& steady_clock, MakeModulesFunction make_modules);

  private:
	void
	connect_modules() override;

  private:
	xf::Clock&	_xefis_clock;
	xf::Clock&	_steady_clock;

	// Has to be last since it requires constructed HardwareMachine:
	std::unique_ptr<HardwareModules> _modules;
};

} // namespace sim1::aircraft

#endif

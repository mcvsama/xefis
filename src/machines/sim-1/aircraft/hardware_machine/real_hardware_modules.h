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

#ifndef XEFIS__MACHINES__SIM_1__AIRCRAFT__HARDWARE_MACHINE__REAL_HARDWARE_MODULES_H__INCLUDED
#define XEFIS__MACHINES__SIM_1__AIRCRAFT__HARDWARE_MACHINE__REAL_HARDWARE_MODULES_H__INCLUDED

// Sim-1:
#include <machines/sim-1/aircraft/common/link/flight_computer_to_hardware_data.h>
#include <machines/sim-1/aircraft/common/link/hardware_to_flight_computer_data.h>

// Standard:
#include <cstddef>


namespace sim1::aircraft {

class RealHardwareModules: public sim1::aircraft::HardwareModules
{
  public:
	// Ctor
	using HardwareModules::HardwareModules;
};

} // namespace sim1::aircraft

#endif

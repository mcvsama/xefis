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

#ifndef XEFIS__MACHINES__SIM_1__AIRCRAFT__MACHINE_H__INCLUDED
#define XEFIS__MACHINES__SIM_1__AIRCRAFT__MACHINE_H__INCLUDED

// Local:
#include "computers.h"
#include "data_center.h"
#include "hardware.h"
#include "models.h"
#include "simulation.h"

// Machine:
#include <machines/sim-1/common/common.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/core/single_loop_machine.h>

// Neutrino:
#include <neutrino/logger.h>

// Standard:
#include <cstddef>
#include <optional>


namespace sim1::aircraft {

class Machine: public xf::SingleLoopMachine
{
  public:
	// Ctor
	Machine (xf::Xefis&);

	DataCenter&
	data_center() noexcept
		{ return _data_center; }

	DataCenter const&
	data_center() const noexcept
		{ return _data_center; }

  private:
	void
	connect_modules() override;

  private:
	DataCenter			_data_center	{ loop() };
	Models				_models;
	Hardware			_hardware		{ loop(), logger() };
	Computers			_computers;
	Simulation			_simulation		{ *this, _models, logger().with_context ("simulation") };
};

} // namespace sim1::aircraft

#endif


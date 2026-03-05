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

#ifndef XEFIS__MACHINES__SIM_1__GROUND_STATION__CONTROL_MACHINE__CONTROL_MACHINE_H__INCLUDED
#define XEFIS__MACHINES__SIM_1__GROUND_STATION__CONTROL_MACHINE__CONTROL_MACHINE_H__INCLUDED

// Local:
#include "control_data_center.h"
#include "control_models.h"
#include "control_modules.h"

// Sim-1:
#include <machines/sim-1/common/common.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/core/single_loop_machine.h>

// Neutrino:
#include <neutrino/logger.h>

// Standard:
#include <cstddef>


namespace sim1::ground_station {

class ControlMachine: public xf::SingleLoopMachine
{
  public:
	// Ctor
	explicit
	ControlMachine (xf::Xefis&);

	ControlDataCenter&
	data_center() noexcept
		{ return _data_center; }

	ControlDataCenter const&
	data_center() const noexcept
		{ return _data_center; }

  private:
	void
	connect_modules() override;

  private:
	ControlDataCenter	_data_center	{ loop() };
	ControlModels		_models;
	ControlModules		_modules		{ loop(), this, logger() };
};

} // namespace sim1::ground_station

#endif

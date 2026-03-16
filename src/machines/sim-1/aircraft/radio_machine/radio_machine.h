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
};

} // namespace sim1::aircraft

#endif

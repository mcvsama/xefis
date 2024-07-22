/* vim:ts=4
 *
 * Copyleft 2012…2016  Michał Gawron
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
#include "single_loop_machine.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/components/configurator/configurator_widget.h>

// Standard:
#include <cstddef>


namespace xf {

SingleLoopMachine::SingleLoopMachine (Xefis& xefis, xf::Logger const& logger, si::Frequency const loop_frequency):
	xf::Machine (xefis),
	_logger (logger),
	_loop ("Main loop", loop_frequency, _logger.with_scope ("main loop"))
{ }


void
SingleLoopMachine::start()
{
	connect_modules();
	register_processing_loop (_loop);
	_loop.start();
}

} // namespace xf


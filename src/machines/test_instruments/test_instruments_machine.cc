/* vim:ts=4
 *
 * Copyleft 2008…2017  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

// Standard:
#include <cstddef>
#include <thread>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/xefis_machine.h>

// Local:
#include "test_instruments_machine.h"


TestInstrumentsMachine::TestInstrumentsMachine (xf::Xefis* xefis):
	Machine (xefis),
	_logger (xefis->logger())
{
	_navaid_storage = std::make_unique<xf::NavaidStorage> (_logger);
	_work_performer = std::make_unique<xf::WorkPerformer> (std::thread::hardware_concurrency(), _logger);

	_test_loop.emplace (this, xefis, _logger);
	register_processing_loop (*_test_loop);
	(*_test_loop)->start();
}


std::unique_ptr<xf::Machine>
xefis_machine (xf::Xefis* xefis)
{
	return std::make_unique<TestInstrumentsMachine> (xefis);
}


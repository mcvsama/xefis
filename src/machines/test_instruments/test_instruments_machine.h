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

#ifndef XEFIS__MACHINES__TEST_INSTRUMENTS__TEST_INSTRUMENTS_MACHINE_H__INCLUDED
#define XEFIS__MACHINES__TEST_INSTRUMENTS__TEST_INSTRUMENTS_MACHINE_H__INCLUDED

// Local:
#include "test_screen_1.h"
#include "test_screen_2.h"

// Xefis:
#include <xefis/app/xefis.h>
#include <xefis/config/all.h>
#include <xefis/core/machine.h>
#include <xefis/core/processing_loop.h>
#include <xefis/modules/test/test_generator.h>
#include <xefis/support/earth/navigation/navaid_storage.h>

// Neutrino:
#include <neutrino/logger.h>
#include <neutrino/tracker.h>
#include <neutrino/work_performer.h>

// Standard:
#include <cstddef>
#include <optional>
#include <memory>


class TestInstrumentsMachine: public xf::Machine
{
  public:
	// Ctor
	TestInstrumentsMachine (xf::Xefis&);

	// Dtor
	~TestInstrumentsMachine();

  private:
	xf::Logger											_logger;
	std::unique_ptr<xf::NavaidStorage>					_navaid_storage;
	std::unique_ptr<xf::WorkPerformer>					_work_performer;
	std::optional<xf::Registrant<xf::ProcessingLoop>>	_test_loop;
	std::optional<xf::Registrant<TestScreen1>>			_test_screen_1;
	std::optional<xf::Registrant<TestScreen2>>			_test_screen_2;
	std::optional<xf::Registrant<TestGenerator>>		_test_generator;
};

#endif


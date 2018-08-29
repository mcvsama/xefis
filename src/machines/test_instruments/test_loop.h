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

#ifndef XEFIS__MACHINES__TEST_INSTRUMENTS__TEST_LOOP_H__INCLUDED
#define XEFIS__MACHINES__TEST_INSTRUMENTS__TEST_LOOP_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/logger.h>
#include <xefis/core/machine.h>
#include <xefis/core/processing_loop.h>
#include <xefis/core/screen.h>
#include <xefis/core/xefis.h>
#include <xefis/modules/test/test_generator.h>

// Local:
#include "test_screen.h"


class TestLoop: public xf::ProcessingLoop
{
  public:
	// Ctor
	explicit
	TestLoop (xf::Machine*, xf::Xefis*, xf::Logger const&);

  private:
	xf::Logger										_logger;
	std::optional<TestScreen>						_test_screen;
	std::optional<xf::Registrant<TestGenerator>>	_test_generator;
};

#endif


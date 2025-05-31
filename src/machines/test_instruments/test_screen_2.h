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

#ifndef XEFIS__MACHINES__TEST_INSTRUMENTS__TEST_SCREEN_2_H__INCLUDED
#define XEFIS__MACHINES__TEST_INSTRUMENTS__TEST_SCREEN_2_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/graphics.h>
#include <xefis/core/machine.h>
#include <xefis/core/screen.h>
#include <xefis/modules/instruments/adi.h>
#include <xefis/modules/instruments/flaps.h>
#include <xefis/modules/instruments/gear.h>
#include <xefis/modules/instruments/hsi.h>
#include <xefis/modules/instruments/horizontal_trim.h>
#include <xefis/modules/instruments/label.h>
#include <xefis/modules/instruments/linear_gauge.h>
#include <xefis/modules/instruments/vertical_trim.h>
#include <xefis/modules/instruments/radial_gauge.h>
#include <xefis/support/universe/earth/navaid_storage.h>

// Neutrino:
#include <neutrino/work_performer.h>

// Standard:
#include <cstddef>
#include <memory>


class TestScreen2: public xf::Screen
{
  public:
	// Ctor
	explicit
	TestScreen2 (xf::ProcessingLoop&, xf::ScreenSpec const&, xf::Graphics const&, xf::NavaidStorage const&, xf::Machine&, xf::Logger const& logger);

  private:
	xf::Logger					_logger;
	xf::Graphics const&			_graphics;
	xf::NavaidStorage const&	_navaid_storage;

  public:
	HSI							hsi_1;
	HSI							hsi_2;

  private:
	// Since work performers perform tasks provided by modules (painting stuff, etc),
	// they should be deleted first, to make sure no task is being executed when the module
	// itself is deleted.
	xf::WorkPerformer			_hsi_work_performer;
};

#endif


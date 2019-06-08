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

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "test_screen_2.h"


TestScreen2::TestScreen2 (xf::ScreenSpec const& spec, xf::Graphics const& graphics, xf::NavaidStorage const& navaid_storage, xf::Machine& machine, xf::Logger const& logger):
	Screen (spec, graphics, machine, "Test Screen 2", logger.with_scope ("TestScreen2")),
	_graphics (graphics),
	_navaid_storage (navaid_storage),
	_hsi_work_performer (2, logger.with_scope ("big-HSI"))
{
	hsi_1_io->arpt_runways_range_threshold	= 2_nmi;
	hsi_1_io->arpt_map_range_threshold		= 1_nmi;
	hsi_1_io->arpt_runway_extension_length	= 10_nmi;

	hsi_2_io->arpt_runways_range_threshold	= 20_nmi;
	hsi_2_io->arpt_map_range_threshold		= 1_nmi;
	hsi_2_io->arpt_runway_extension_length	= 10_nmi;
}


void
TestScreen2::create_instruments()
{
	_hsi_1.emplace (std::move (hsi_1_io), _graphics, _navaid_storage, "big-hsi-1");
	register_instrument (*_hsi_1, _hsi_work_performer);
	set (**_hsi_1, { 0.0f, 0.0f, 0.5f, 1.0f });

	_hsi_2.emplace (std::move (hsi_2_io), _graphics, _navaid_storage, "big-hsi-2");
	register_instrument (*_hsi_2, _hsi_work_performer);
	set (**_hsi_2, { 0.5f, 0.0f, 0.5f, 1.0f });

	set_paint_bounding_boxes (false);
}



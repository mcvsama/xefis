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

// Local:
#include "test_screen_2.h"

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>


TestScreen2::TestScreen2 (xf::ScreenSpec const& spec, xf::Graphics const& graphics, xf::NavaidStorage const& navaid_storage, xf::Machine& machine, xf::Logger const& logger):
	Screen (spec, graphics, machine, "Test Screen 2", logger.with_scope ("TestScreen2")),
	_logger (logger),
	_graphics (graphics),
	_navaid_storage (navaid_storage),
	_hsi_work_performer (2, _logger.with_scope ("big-HSI")),
	// Instruments:
	hsi_1 (_graphics, _navaid_storage, _logger, "big-hsi-1"),
	hsi_2 (_graphics, _navaid_storage, _logger, "big-hsi-2")
{
	register_instrument (this->hsi_1, _hsi_work_performer);
	register_instrument (this->hsi_2, _hsi_work_performer);

	this->hsi_1->arpt_runways_range_threshold	= 2_nmi;
	this->hsi_1->arpt_map_range_threshold		= 1_nmi;
	this->hsi_1->arpt_runway_extension_length	= 10_nmi;

	this->hsi_2->arpt_runways_range_threshold	= 20_nmi;
	this->hsi_2->arpt_map_range_threshold		= 1_nmi;
	this->hsi_2->arpt_runway_extension_length	= 10_nmi;

	set (*this->hsi_1, { 0.0f, 0.0f, 0.5f, 1.0f });
	set (*this->hsi_2, { 0.5f, 0.0f, 0.5f, 1.0f });

	set_paint_bounding_boxes (false);
}


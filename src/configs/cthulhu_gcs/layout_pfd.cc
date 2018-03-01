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
#include "layout_pfd.h"


LayoutPFD::LayoutPFD (Loop* loop)
{
	_window = std::make_unique<v2::Window>();

	auto* layout = new QVBoxLayout();
	layout->addWidget (loop->adi, 8);
	// layout->setContentsMargins (left, top, right, bottom);
	// TODO layout->addWidget (loop->hsi_aux, 5);

	// TODO class InstrumentStack with InputProperties that control visible instrument
}


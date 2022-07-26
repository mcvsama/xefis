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
#include "instrument_painter.h"

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>


namespace xf {

InstrumentPainter::InstrumentPainter (QPaintDevice& device, TextPainter::Cache& cache):
	QPainter (&device),
	TextPainter (device, cache)
{
	setRenderHint (QPainter::Antialiasing, true);
	setRenderHint (QPainter::TextAntialiasing, true);
	setRenderHint (QPainter::SmoothPixmapTransform, true);
	set_font_position_correction ({ 0.0, 0.04 });
}


void
InstrumentPainter::save_context (std::function<void()> paint_callback)
{
	save();

	try {
		paint_callback();
	}
	catch (...)
	{
		restore();
		throw;
	}

	restore();
}

} // namespace xf


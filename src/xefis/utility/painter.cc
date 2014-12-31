/* vim:ts=4
 *
 * Copyleft 2012…2015  Michał Gawron
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
#include <cmath>

// Xefis:
#include <xefis/utility/numeric.h>

// Local:
#include "painter.h"


namespace Xefis {

Painter::Painter (TextPainter::Cache* cache):
	TextPainter (cache)
{ }


Painter::Painter (QPaintDevice* device, TextPainter::Cache* cache):
	TextPainter (device, cache)
{ }


void
Painter::draw_outlined_line (QPointF const& from, QPointF const& to)
{
	configure_for_shadow();
	drawLine (from, to);
	configure_normal();
	drawLine (from, to);
}


void
Painter::draw_outlined_polygon (QPolygonF const& polygon)
{
	configure_for_shadow();
	drawPolygon (polygon);
	configure_normal();
	drawPolygon (polygon);
}

} // namespace Xefis


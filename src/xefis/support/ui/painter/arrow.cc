/* vim:ts=4
 *
 * Copyleft 2024  Michał Gawron
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
#include "arrow.h"

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/numeric.h>

// Standard:
#include <cstddef>


namespace xf {

void
draw_arrow (QPainter& painter, QPointF const& from, QPointF const& to, double arrowhead_size)
{
	auto const abs = [](QPointF const& point) {
		return sqrt (square (point.x()) + square (point.y()));
	};

	auto const w = painter.pen().widthF();
	auto const abs_from = QPointF (abs (from), 0.0f);
	auto const abs_to = QPointF (abs (to), 0.0f);

	painter.save();
	auto const angle = 1_rad * std::atan2 (to.y() - from.y(), to.x() - from.x());
	painter.rotate (angle / 1_deg);
	painter.drawLine (abs_from, abs_to);

	auto const b = abs_to + QPointF (-2.5 * arrowhead_size * w, -arrowhead_size * w);
	auto const c = b + QPointF (0.0f, +2 * arrowhead_size * w);
	QPointF const points[] = { abs_to, b, c, abs_to };
	painter.drawPolygon (points, std::size (points));

	painter.restore();
}

} // namespace xf


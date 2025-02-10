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
	QLineF const line (from, to);
	auto const line_length = line.length();
	auto const pen_width = painter.pen().widthF();
	auto const tip = QPointF (line_length, 0);

	painter.save();

	// Translate so that 'from' is at the origin:
	painter.translate (from);
	// Compute the angle (in radians) and rotate the painter.
	auto const angle = 1_rad * std::atan2 (to.y() - from.y(), to.x() - from.x());
	painter.rotate (angle.in<si::Degree>());
	// Draw the main line of the arrow:
	painter.drawLine (QPointF (0.0f, 0.0f), tip);

	// Compute the arrowhead points:
	auto const top = QPointF (line_length - 2.5 * arrowhead_size * pen_width, -arrowhead_size * pen_width);
	auto const bottom = QPointF (line_length - 2.5 * arrowhead_size * pen_width, +arrowhead_size * pen_width);

	// Draw arrowhead as a filled polygon:
	QPointF const points[] = { tip, top, bottom, tip };
	painter.drawPolygon (points, std::size (points));

	painter.restore();
}

} // namespace xf


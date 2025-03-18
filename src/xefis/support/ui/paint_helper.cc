/* vim:ts=4
 *
 * Copyleft 2022  Michał Gawron
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
#include "paint_helper.h"

// Xefis:
#include <xefis/config/all.h>

// Qt:
#include <QPainter>

// Standard:
#include <cstddef>


namespace xf {

PaintHelper::PaintHelper (QPaintDevice const& canvas, QPalette const palette, QFont const font):
	_canvas (canvas),
	_palette (palette),
	_font (font)
{ }


PaintHelper::PaintHelper (QWidget const& widget):
	_canvas (widget),
	_palette (widget.palette()),
	_font (widget.font())
{ }


void
PaintHelper::setup_painter (QPainter& painter)
{
	painter.setRenderHint (QPainter::Antialiasing, true);
	painter.setRenderHint (QPainter::TextAntialiasing, true);
	painter.setRenderHint (QPainter::SmoothPixmapTransform, true);
}


QMargins
PaintHelper::group_box_margins() const
{
	auto const s = em_pixels_int (0.5f);
	auto const x = em_pixels_int (1.0f);
	return QMargins (s, s, x, s);
}

} // namespace xf


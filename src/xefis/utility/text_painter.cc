/* vim:ts=4
 *
 * Copyleft 2008…2012  Michał Gawron
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

// Local:
#include "text_painter.h"


TextPainter::TextPainter (QPainter& painter, float oversampling_factor):
	_painter (painter),
	_buffer (1, 1, QImage::Format_ARGB32),
	_oversampling_factor (oversampling_factor)
{
}


void
TextPainter::drawText (QRectF const& target, int flags, QString const& text)
{
	if (_buffer.rect() != target)
		_buffer = QImage (_oversampling_factor * target.width(), _oversampling_factor * target.height(), QImage::Format_ARGB32);

	QFont font = _painter.font();
	font.setPixelSize (_oversampling_factor * font.pixelSize());

	QPainter ov_painter (&_buffer);
	ov_painter.setRenderHint (QPainter::SmoothPixmapTransform, true);
	ov_painter.setRenderHint (QPainter::TextAntialiasing, true);
	ov_painter.setPen (_painter.pen());
	ov_painter.setFont (font);

	// We don't know target background color, so fill the buffer
	// with the desired text color (pen color) with 0 alpha.
	QColor fill_color = _painter.pen().color();
	fill_color.setAlpha (0);
	_buffer.fill (fill_color.rgba());
	ov_painter.drawText (_buffer.rect(), flags, text);
	_painter.drawImage (target, _buffer, _buffer.rect());
}


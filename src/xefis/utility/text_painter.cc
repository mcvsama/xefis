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


QImage*
TextPainter::Cache::load_image (QRect const& size, QColor const& color, QString const& text, int flags)
{
	CacheMap::iterator img = _cache.find (Key { size, color, text, flags });
	if (img == _cache.end())
		return nullptr;
	return &img->second;
}


void
TextPainter::Cache::store_image (QRect const& size, QColor const& color, QString const& text, int flags, QImage& image)
{
	_cache[Key { size, color, text, flags }] = image;
}


TextPainter::TextPainter (QPainter& painter, Cache* cache, float oversampling_factor):
	_cache (cache),
	_painter (painter),
	_buffer (1, 1, QImage::Format_ARGB32),
	_oversampling_factor (oversampling_factor)
{
}


void
TextPainter::drawText (QRectF const& target, int flags, QString const& text)
{
	QRect target_int_rect (0, 0, std::ceil (_oversampling_factor * target.width()), std::ceil (_oversampling_factor * target.height()));

	if (_cache)
	{
		QImage* cached = _cache->load_image (target_int_rect, _painter.pen().color(), text, flags);
		if (cached)
		{
			_painter.drawImage (target, *cached, cached->rect());
			return;
		}
	}

	if (_buffer.rect() != target_int_rect)
		_buffer = QImage (target_int_rect.width(), target_int_rect.height(), QImage::Format_ARGB32);

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

	if (_cache)
		_cache->store_image (target_int_rect, _painter.pen().color(), text, flags, _buffer);

	_painter.drawImage (target, _buffer, _buffer.rect());
}


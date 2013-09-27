/* vim:ts=4
 *
 * Copyleft 2012…2013  Michał Gawron
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
#include "text_painter.h"


namespace Xefis {

TextPainter::Cache::Glyph::Glyph (QFont const& font, QColor const& color, QChar const& character):
	data (new Data())
{
	QFontMetricsF metrics (font);
	QSize size (std::ceil (metrics.width (character)) + 1, std::ceil (metrics.height()) + 1);
	QImage image (size, QImage::Format_ARGB32_Premultiplied);
	QColor alpha = color;
	alpha.setAlpha (0);
	QPainter painter (&image);
	painter.setRenderHint (QPainter::Antialiasing, true);
	painter.setRenderHint (QPainter::TextAntialiasing, true);
	painter.setRenderHint (QPainter::SmoothPixmapTransform, true);
	QColor shadow_color = color.darker (800);
	shadow_color.setAlpha (100);
	QPen shadow_pen (shadow_color, 1.5);// TODO make width relative

	for (int x = 0; x < Rank; ++x)
	{
		float const fx = 1.f * x / Rank;

		for (int y = 0; y < Rank; ++y)
		{
			float const fy = 1.f * y / Rank;

			QPointF position (fx, fy + metrics.ascent());
			QPainterPath glyph_path;
			glyph_path.addText (position, font, character);

			QPainterPath clip_path;
			clip_path.addRect (image.rect());
			clip_path -= glyph_path;

			image.fill (alpha);
			painter.setClipPath (clip_path);
			painter.setPen (shadow_pen);
			painter.setBrush (Qt::NoBrush);
			painter.drawPath (glyph_path);
			painter.setClipping (false);
			painter.setPen (Qt::NoPen);
			painter.setBrush (color);
			painter.drawPath (glyph_path);

			data->positions[x][y] = image;
		}
	}
}


TextPainter::TextPainter (QPaintDevice* device, Cache* cache):
	QPainter (device),
	_cache (cache)
{
	if (!_cache)
		throw std::runtime_error ("attempted to create TextPainter with nullptr cache");
}


QRectF
TextPainter::get_text_box (QPointF const& position, Qt::Alignment flags, QString const& text) const
{
	QFontMetricsF metrics (font());
	QRectF target (position.x(), position.y(), metrics.width (text), metrics.height());

	if (flags & Qt::AlignHCenter)
		target.translate (-0.5f * target.width(), 0.f);
	else if (flags & Qt::AlignRight)
		target.translate (-target.width(), 0.f);

	if (flags & Qt::AlignVCenter)
		target.translate (0.f, -0.5f * target.height());
	else if (flags & Qt::AlignBottom)
		target.translate (0.f, -target.height());

	return target;
}


void
TextPainter::fast_draw_text (QPointF const& position, QString const& text)
{
	QFontMetricsF metrics (font());
	QRectF target (position - QPointF (0.f, metrics.ascent()), QSizeF (metrics.width (text), metrics.height()));
	fast_draw_text (target, 0, text);
}


void
TextPainter::fast_draw_text (QPointF const& position, Qt::Alignment flags, QString const& text)
{
	fast_draw_text (get_text_box (position, flags, text), 0, text);
}


void
TextPainter::fast_draw_text (QRectF const& target, Qt::Alignment flags, QString const& text)
{
	QFontMetricsF metrics (font());
	QPointF target_center = target.center();
	QPointF offset;

	if (flags & Qt::AlignHCenter)
		offset.setX (target_center.x() - 0.5f * metrics.width (text));
	else if (flags & Qt::AlignRight)
		offset.setX (target.right() - metrics.width (text));
	else // default: AlignLeft
		offset.setX (target.left());

	if (flags & Qt::AlignVCenter)
		offset.setY (target_center.y() - 0.5f * metrics.height());
	else if (flags & Qt::AlignBottom)
		offset.setY (target.bottom() - metrics.height());
	else // default: AlignTop
		offset.setY (target.top());

	// Check if we need to take into account painter transform:
	QTransform painter_transform = transform();
	bool saved_transform = false;
	if (painter_transform.isAffine() && !painter_transform.isRotating() && !painter_transform.isScaling() && painter_transform.isTranslating())
	{
		qreal x, y;
		painter_transform.map (0.f, 0.f, &x, &y);
		resetTransform();
		offset.rx() += x;
		offset.ry() += y;
		saved_transform = true;
	}

	QColor color = pen().color();

	// TODO cache previous painting info { font, color }, and if the same, use cached glyphs_cache iterator.
	// Find font cache:
	Cache::Fonts::iterator glyphs_cache_it = _cache->fonts.find (Cache::Font { font(), color });
	if (glyphs_cache_it == _cache->fonts.end())
		glyphs_cache_it = _cache->fonts.insert ({ Cache::Font { font(), color }, Cache::Glyphs() }).first;
	Cache::Glyphs* glyphs_cache = &glyphs_cache_it->second;

	for (QString::ConstIterator c = text.begin(); c != text.end(); ++c)
	{
		auto glyph = glyphs_cache->find (*c);
		if (glyph == glyphs_cache->end())
			glyph = glyphs_cache->insert ({ *c, Cache::Glyph (font(), color, *c) }).first;
		float fx = floored_mod<float> (offset.x(), 1.f);
		float fy = floored_mod<float> (offset.y(), 1.f);
		int dx = limit<int> (fx * Cache::Glyph::Rank, 0, Cache::Glyph::Rank - 1);
		int dy = limit<int> (fy * Cache::Glyph::Rank, 0, Cache::Glyph::Rank - 1);
		drawImage (QPoint (offset.x(), offset.y()), glyph->second.data->positions[dx][dy]);
		offset.rx() += metrics.width (*c);
	}

	if (saved_transform)
		setTransform (painter_transform);
}

} // namespace Xefis


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
#include "text_painter.h"

// Xefis:
#include <xefis/config/all.h>

// Qt:
#include <QPainterPath>

// Neutrino:
#include <neutrino/numeric.h>

// Standard:
#include <cstddef>
#include <cmath>


namespace xf {

TextPainter::Cache::Glyph::Glyph (QFont const& font, QColor color, QChar character, QPointF position_correction, std::optional<Shadow> shadow):
	data (std::make_shared<Data>())
{
	QFontMetricsF metrics (font);
	position_correction.setX (position_correction.x() * metrics.width("0"));
	position_correction.setY (position_correction.y() * metrics.height());
	QSize size (std::ceil (metrics.width (character)) + 1, std::ceil (metrics.height()) + 1);
	QImage image (size, QImage::Format_ARGB32_Premultiplied);
	QColor alpha = color;
	alpha.setAlpha (0);
	QPainter painter (&image);
	painter.setRenderHint (QPainter::Antialiasing, true);
	painter.setRenderHint (QPainter::TextAntialiasing, true);
	painter.setRenderHint (QPainter::SmoothPixmapTransform, true);

	QPen shadow_pen = painter.pen();

	if (shadow)
	{
		QColor shadow_color = color.darker (800);
		shadow_color.setAlpha (100);
		shadow_pen.setColor (shadow_color);
		shadow_pen.setWidthF (shadow->width_for_pen (shadow_pen));
	}

	for (int x = 0; x < Rank; ++x)
	{
		float const fx = 1.f * x / Rank;

		for (int y = 0; y < Rank; ++y)
		{
			float const fy = 1.f * y / Rank;

			QPointF position (fx, fy + metrics.ascent());
			position += position_correction;
			QPainterPath glyph_path;
			glyph_path.addText (position, font, character);

			QPainterPath clip_path;
			clip_path.addRect (image.rect());
			clip_path -= glyph_path;

			image.fill (alpha);
			painter.setClipPath (clip_path);

			if (shadow)
			{
				painter.setPen (shadow_pen);
				painter.setBrush (Qt::NoBrush);
				painter.drawPath (glyph_path);
			}

			painter.setClipping (false);
			painter.setPen (Qt::NoPen);
			painter.setBrush (color);
			painter.drawPath (glyph_path);

			data->positions[x][y] = image;
		}
	}
}


TextPainter::TextPainter (Cache& cache):
	_cache (cache)
{ }


TextPainter::TextPainter (QPaintDevice& device, Cache& cache):
	QPainter (&device),
	_cache (cache)
{ }


void
TextPainter::set_font_position_correction (QPointF correction)
{
	_position_correction = correction;
}


QRectF
TextPainter::get_text_box (QPointF const& position, Qt::Alignment flags, QString const& text) const
{
	QFontMetricsF metrics (font());
	QRectF target (position.x(), position.y(), metrics.width (text), metrics.height());
	apply_alignment (target, flags);

	return target;
}


QRectF
TextPainter::get_vertical_text_box (QPointF const& position, Qt::Alignment flags, QString const& text) const
{
	QFontMetricsF metrics (font());

	float widest_char = 0.f;
	for (auto const& c: text)
		widest_char = std::max<float> (widest_char, metrics.width (c));

	QRectF target (position.x(), position.y(), widest_char, metrics.height() * text.size());
	apply_alignment (target, flags);

	return target;
}


void
TextPainter::fast_draw_text (QPointF const& position, QString const& text, std::optional<Shadow> shadow)
{
	QFontMetricsF metrics (font());
	QRectF target (position - QPointF (0.f, metrics.ascent()), QSizeF (metrics.width (text), metrics.height()));
	fast_draw_text (target, {}, text, shadow);
}


void
TextPainter::fast_draw_text (QPointF const& position, Qt::Alignment flags, QString const& text, std::optional<Shadow> shadow)
{
	fast_draw_text (get_text_box (position, flags, text), {}, text, shadow);
}


void
TextPainter::fast_draw_text (QRectF const& target, Qt::Alignment flags, QString const& text, std::optional<Shadow> shadow)
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

	float const shadow_width = shadow ? shadow->width_for_pen (pen()) : 0.0f;

	// Find/insert to font cache:
	Cache::Font const required_font { font(), color, shadow_width };
	Cache::Glyphs* glyphs_cache { nullptr };

	if (!_cache.last || _cache.last->font != required_font)
	{
		if (auto it = _cache.fonts.find (required_font); it != _cache.fonts.end())
			_cache.last = Cache::Last { required_font, &it->second };
		else
		{
			it = _cache.fonts.emplace (required_font, Cache::Glyphs()).first;
			_cache.last = Cache::Last { required_font, &it->second };
		}
	}

	glyphs_cache = _cache.last->glyphs;

	for (QString::ConstIterator c = text.begin(); c != text.end(); ++c)
	{
		auto glyph = glyphs_cache->find (*c);

		if (glyph == glyphs_cache->end())
			glyph = glyphs_cache->insert ({ *c, Cache::Glyph (font(), color, *c, _position_correction, shadow) }).first;

		float fx = floored_mod<float> (offset.x(), 1.f);
		float fy = floored_mod<float> (offset.y(), 1.f);
		int dx = clamped<int> (fx * Cache::Glyph::Rank, 0, Cache::Glyph::Rank - 1);
		int dy = clamped<int> (fy * Cache::Glyph::Rank, 0, Cache::Glyph::Rank - 1);
		drawImage (QPoint (offset.x(), offset.y()), glyph->second.data->positions[dx][dy]);
		offset.rx() += metrics.width (*c);
	}

	if (saved_transform)
		setTransform (painter_transform);
}


void
TextPainter::fast_draw_vertical_text (QPointF const& position, Qt::Alignment flags, QString const& text, std::optional<Shadow> shadow)
{
	QFontMetricsF metrics (font());
	QRectF rect = get_vertical_text_box (position, flags, text);

	QPointF top_char (rect.center().x(), rect.top() + 0.5f * metrics.height());
	QPointF char_height (0.f, metrics.height());

	for (int i = 0; i < text.size(); ++i)
		fast_draw_text (top_char + i * char_height, Qt::AlignVCenter | Qt::AlignHCenter, text[i], shadow);
}


void
TextPainter::apply_alignment (QRectF& rect, Qt::Alignment flags)
{
	if (flags & Qt::AlignHCenter)
		rect.translate (-0.5f * rect.width(), 0.f);
	else if (flags & Qt::AlignRight)
		rect.translate (-rect.width(), 0.f);

	if (flags & Qt::AlignVCenter)
		rect.translate (0.f, -0.5f * rect.height());
	else if (flags & Qt::AlignBottom)
		rect.translate (0.f, -rect.height());
}

} // namespace xf


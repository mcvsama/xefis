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
#include "text_layout.h"

// Standard:
#include <cstddef>


namespace xf {

void
TextLayout::Fragment::paint (QPointF top_left, TextPainter& painter, std::optional<Shadow> shadow) const
{
	QPointF lh_corr (0.0, 0.5 * (_metrics.height() - _height));
	painter.setFont (_font);
	painter.setPen (QPen (_color, 1.0));
	painter.fast_draw_text (top_left - lh_corr, Qt::AlignTop | Qt::AlignLeft, _text, shadow);
	painter.setPen (_box_pen);
	painter.setBrush (Qt::NoBrush);
	float const m = 0.15 * _height;
	painter.drawRect (QRectF (top_left - QPointF (m, 0.0), QSizeF (_width + 2.0 * m, _height)));
}


TextLayout::Line::Line (double line_height_factor):
	_line_height_factor (line_height_factor)
{ }


double
TextLayout::Line::width() const
{
	double w = 0.0;
	for (auto const& fragment: _fragments)
		w += fragment.width();
	return w;
}


double
TextLayout::Line::height() const
{
	double h = 0.0;
	for (auto const& fragment: _fragments)
		h = std::max (h, fragment.height());
	return h;
}


void
TextLayout::Line::paint (QPointF top_left, TextPainter& painter, std::optional<Shadow> shadow) const
{
	// Make sure that baseline of all fonts used is at the same vertical location,
	// determined by the fragment with the biggest font's ascent:
	double biggest_ascent = 0.0;
	for (Fragment const& fragment: _fragments)
		biggest_ascent = std::max (biggest_ascent, fragment.metrics().ascent());
	QPointF offset;
	QPointF corr;

	for (Fragment const& fragment: _fragments)
	{
		corr.setY (biggest_ascent - fragment.metrics().ascent());
		fragment.paint (top_left + offset + corr, painter, shadow);
		offset.rx() += fragment.width();
	}
}


void
TextLayout::add_fragment (QString const& text, QFont const& font, QColor const& color, QPen const& box_pen)
{
	_lines.back().add_fragment (Fragment (text, font, color, box_pen, _line_height_factor));
}


void
TextLayout::add_skips (QFont const& font, unsigned int number)
{
	for (unsigned int i = 0; i < number; ++i)
	{
		add_fragment ("", font, Qt::white);
		add_new_line();
		add_fragment ("", font, Qt::white);
	}
}


double
TextLayout::width() const
{
	double w = 0.0;
	for (auto const& line: _lines)
		w = std::max (w, line.width());
	return w;
}


double
TextLayout::height() const
{
	double h = 0.0;
	for (auto const& line: _lines)
		h += line.height();
	return h;
}


void
TextLayout::paint (QPointF position, Qt::Alignment alignment, TextPainter& painter, std::optional<Shadow> shadow) const
{
	QSizeF size = this->size();

	if (alignment & Qt::AlignHCenter)
		position.rx() -= 0.5 * size.width();
	else if (alignment & Qt::AlignRight)
		position.rx() -= size.width();

	if (alignment & Qt::AlignVCenter)
		position.ry() -= 0.5 * size.height();
	else if (alignment & Qt::AlignBottom)
		position.ry() -= size.height();

	QPointF margin = { _background_margin.width(), _background_margin.height() };
	painter.save();
	painter.setPen (Qt::NoPen);
	painter.setBrush (_background);

	if (_background_mode == Whole)
		painter.drawRect (QRectF (position - margin, size + 2 * _background_margin));

	painter.translate (position);

	for (Line const& line: _lines)
	{
		QPointF pos (0.0, 0.0);

		// Line alignment:
		if (_default_line_alignment & Qt::AlignRight)
			pos.setX (size.width() - line.width());
		else if (_default_line_alignment & Qt::AlignHCenter)
			pos.setX (0.5 * (size.width() - line.width()));

		if (_background_mode == PerLine)
		{
			painter.setBrush (_background);
			painter.drawRect (QRectF (pos, QSizeF (line.width(), line.height())));
		}

		line.paint (pos, painter, shadow);
		painter.translate (0.0, line.height());
	}
	painter.restore();
}

} // namespace xf


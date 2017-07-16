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

// Standard:
#include <cstddef>
#include <vector>

// Qt:
#include <QtGui/QPainter>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v1/window.h>
#include <xefis/utility/numeric.h>

// Local:
#include "linear_indicator_widget.h"


LinearIndicatorWidget::LinearIndicatorWidget (QWidget* parent):
	InstrumentWidget (parent),
	InstrumentAids (0.8f)
{ }


void
LinearIndicatorWidget::resizeEvent (QResizeEvent* event)
{
	InstrumentWidget::resizeEvent (event);

	auto xw = dynamic_cast<v1::Window*> (window());
	if (xw)
		InstrumentAids::set_scaling (1.2f * xw->pen_scale(), 0.95f * xw->font_scale());

	InstrumentAids::update_sizes (size(), window()->size());
}


void
LinearIndicatorWidget::paintEvent (QPaintEvent*)
{
	auto painting_token = get_token (this);

	float const w = width();
	float const h = height();

	QPen pen_white = get_pen (Qt::white, 1.f);
	QPen pen_silver = get_pen (QColor (0xbb, 0xbd, 0xbf), 1.f);

	clear_background();

	if (_mirrored)
	{
		painter().translate (w, 0.f);
		painter().scale (-1.f, 1.f);
	}

	float q = 0.05f * w;
	float m = 0.7f * q;
	QRectF area (m, m, w - 2.f * m, h - 2.f * m);

	QPointF p0 (area.right() - 3.f * q, area.top());
	QPointF p1 (area.right() - 3.f * q, area.bottom());

	// Indicator

	painter().setPen (pen_silver);
	painter().drawLine (p0, p1);

	if (_value)
	{
		auto value = xf::clamped (*_value, _range.min(), _range.max());
		bool inbound = _range.includes (*_value);

		if (inbound)
			painter().setBrush (Qt::white);
		else
			painter().setBrush (Qt::NoBrush);

		painter().setPen (pen_white);
		QPolygonF polygon = QPolygonF()
			<< QPointF (0.f, 0.f)
			<< QPointF (1.9f * q, -0.5f * q)
			<< QPointF (1.9f * q, +0.5f * q);
		polygon.translate (p1.x(), xf::renormalize (value, _range.min(), _range.max(), p1.y(), p0.y()));
		painter().add_shadow ([&] {
			painter().drawPolygon (polygon);
		});
	}

	// Text

	QFont font (_font_20);
	QFontMetricsF metrics (font);
	float char_width = metrics.width ("0");
	float hcorr = 0.025f * metrics.height();

	QString text;
	if (_value)
		text = stringify_value (*_value);
	text = pad_string (text);

	painter().setFont (font);
	QRectF text_rect = painter().get_text_box (QPointF (p0.x() - q, h / 2.f), Qt::AlignRight | Qt::AlignVCenter, text);
	text_rect.adjust (-0.5f * char_width, 0, 0.f, -2.f * hcorr);
	painter().setPen (get_pen (Qt::white, 0.8f));
	painter().setBrush (Qt::NoBrush);
	painter().drawRect (text_rect);
	QPointF position;
	if (_mirrored)
	{
		position = QPointF (text_rect.left() + 0.25f * char_width, text_rect.center().y());
		position = painter().transform().map (position);
		painter().resetTransform();
	}
	else
		position = QPointF (text_rect.right() - 0.25f * char_width, text_rect.center().y());
	painter().fast_draw_text (position, Qt::AlignVCenter | Qt::AlignRight, text);
}


QString
LinearIndicatorWidget::stringify_value (double value) const
{
	double numeric_value = value;
	if (_precision < 0)
		numeric_value /= std::pow (10.0, -_precision);
	if (_modulo > 0)
		numeric_value = static_cast<int> (numeric_value) / _modulo * _modulo;
	return QString ("%1").arg (numeric_value, 0, 'f', std::max (0, _precision));
}


QString
LinearIndicatorWidget::pad_string (QString const& input) const
{
	return QString ("%1").arg (input, _digits);
}


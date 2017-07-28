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
#include <string>
#include <algorithm>
#include <cmath>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v1/window.h>
#include <xefis/utility/numeric.h>
#include <xefis/utility/painter.h>

// Local:
#include "linear_indicator.h"


LinearIndicator::LinearIndicator (std::unique_ptr<LinearIndicatorIO> module_io, v2::PropertyDigitizer value_digitizer, std::string const& instance):
	InstrumentAids (0.8f),
	BasicIndicator (std::move (module_io), instance),
	_value_digitizer (value_digitizer)
{
	_inputs_observer.set_callback ([&]{ update(); });
	_inputs_observer.observe (_value_digitizer.property());
}


void
LinearIndicator::process (v2::Cycle const& cycle)
{
	_inputs_observer.process (cycle.update_dt());
}


void
LinearIndicator::paintEvent (QPaintEvent*)
{
	std::optional<double> value = _value_digitizer.to_numeric();
	xf::Range<double> range { *io.value_minimum, *io.value_maximum };

	auto painting_token = get_token (this);

	float const w = width();
	float const h = height();

	QPen pen_white = get_pen (Qt::white, 1.f);
	QPen pen_silver = get_pen (QColor (0xbb, 0xbd, 0xbf), 1.f);

	clear_background();

	if (*io.mirrored_style)
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

	if (value)
	{
		auto v_value = xf::clamped (*value, range);
		bool inbound = range.includes (*value);

		if (inbound)
			painter().setBrush (Qt::white);
		else
			painter().setBrush (Qt::NoBrush);

		painter().setPen (pen_white);
		QPolygonF polygon = QPolygonF()
			<< QPointF (0.f, 0.f)
			<< QPointF (1.9f * q, -0.5f * q)
			<< QPointF (1.9f * q, +0.5f * q);
		polygon.translate (p1.x(), xf::renormalize (v_value, range.min(), range.max(), p1.y(), p0.y()));
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
	if (value)
		text = stringify_value (*value);
	text = pad_string (text);

	painter().setFont (font);
	QRectF text_rect = painter().get_text_box (QPointF (p0.x() - q, h / 2.f), Qt::AlignRight | Qt::AlignVCenter, text);
	text_rect.adjust (-0.5f * char_width, 0, 0.f, -2.f * hcorr);
	painter().setPen (get_pen (Qt::white, 0.8f));
	painter().setBrush (Qt::NoBrush);
	painter().drawRect (text_rect);
	QPointF position;

	if (*io.mirrored_style)
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
LinearIndicator::pad_string (QString const& input) const
{
	return QString ("%1").arg (input, *io.digits);
}


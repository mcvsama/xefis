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
#include <vector>

// Qt:
#include <QtGui/QPainter>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/window.h>
#include <xefis/utility/numeric.h>

// Local:
#include "radial_indicator_widget.h"


RadialIndicatorWidget::RadialIndicatorWidget (QWidget* parent):
	InstrumentWidget (parent),
	InstrumentAids (1.f)
{
	auto xw = dynamic_cast<Xefis::Window*> (window());
	if (xw)
		InstrumentAids::set_scaling (xw->pen_scale(), xw->font_scale());
}


void
RadialIndicatorWidget::resizeEvent (QResizeEvent*)
{
	InstrumentAids::update_sizes (size(), window()->size());
}


void
RadialIndicatorWidget::paintEvent (QPaintEvent*)
{
	float const w = width();
	float const h = height();

	Xefis::Painter painter (this, &_text_painter_cache);
	painter.setRenderHint (QPainter::Antialiasing, true);
	painter.setRenderHint (QPainter::TextAntialiasing, true);
	painter.setRenderHint (QPainter::SmoothPixmapTransform, true);
	painter.setRenderHint (QPainter::NonCosmeticDefaultPen, true);

	// Clear with black background:
	painter.setPen (Qt::NoPen);
	painter.setBrush (QBrush (QColor (0, 0, 0), Qt::SolidPattern));
	painter.drawRect (rect());

	painter.translate (w / 2.25f, h / 2.f);

	float q = 0.06f * wh();
	float r = 6.5f * q;

	paint_text (painter, q, r);
	paint_indicator (painter, q, r);
}


void
RadialIndicatorWidget::paint_text (Xefis::Painter& painter, float q, float)
{
	QString text = QString ("%1").arg (_value, 0, 'f', 1);

	QFont font (_font_20);
	QFontMetricsF metrics (font);
	QFont small_font (_font_16);
	QFontMetricsF small_metrics (small_font);

	QPen pen = get_pen (Qt::white, 1.f);
	pen.setCapStyle (Qt::RoundCap);

	float margin = 0.4f * q;
	float zero_width = metrics.width ('0');
	float small_zero_width = small_metrics.width ('0');

	QRectF text_rect (0.5f * pen.width(), -0.6f * q, metrics.width ("000.0"), 0.9f * metrics.height());
	text_rect.translate (margin, -text_rect.height());
	QRectF rect = text_rect.adjusted (-margin, 0, margin, 0);

	painter.save();
	painter.setFont (font);
	if (_value_visible)
	{
		painter.setPen (pen);
		painter.drawRect (rect);
		float const bit_lower = 0.13f * q;
		painter.fast_draw_text (text_rect.translated (0.f, bit_lower), Qt::AlignRight | Qt::AlignVCenter, text);
	}
	else
	{
		painter.setPen (pen);
		painter.drawRect (rect);
	}

	if (_normal_visible)
	{
		painter.setFont (small_font);
		painter.setPen (get_pen (Qt::green, 1.0f));
		painter.fast_draw_text (QPointF (text_rect.right() - zero_width + small_zero_width, text_rect.top()),
								Qt::AlignBottom | Qt::AlignRight,
								QString ("%1").arg (_normal_value, 0, 'f', 1));
	}

	painter.restore();
}


void
RadialIndicatorWidget::paint_indicator (Xefis::Painter& painter, float, float r)
{
	QColor silver (0xbb, 0xbd, 0xbf);
	QColor gray (0x7a, 0x7a, 0x7a);
	QColor yellow (255, 220, 0);
	QColor orange (255, 150, 0);
	QColor red (255, 0, 0);

	QPen silver_pen = get_pen (silver, 1.0f);
	silver_pen.setCapStyle (Qt::RoundCap);

	QPen pen = get_pen (Qt::white, 1.0f);
	pen.setCapStyle (Qt::RoundCap);

	QPen pointer_pen = get_pen (Qt::white, 1.1f);
	pointer_pen.setCapStyle (Qt::RoundCap);

	QPen warning_pen = get_pen (yellow, 1.f);
	warning_pen.setCapStyle (Qt::RoundCap);

	QPen critical_pen = get_pen (red, 1.f);
	critical_pen.setCapStyle (Qt::RoundCap);

	QPen green_pen = get_pen (QColor (0x00, 0xff, 0x00), 1.f);
	green_pen.setCapStyle (Qt::RoundCap);

	QPen gray_pen = get_pen (QColor (0xb0, 0xb0, 0xb0), 1.f);
	gray_pen.setCapStyle (Qt::RoundCap);

	QBrush brush (gray, Qt::SolidPattern);
	QRectF rect (-r, -r, 2.f * r, 2.f * r);

	float value_span_angle = 210.f;
	float value = limit (_value, _range);
	float warning = limit (_warning_value, _range);
	float critical = limit (_critical_value, _range);
	float normal = limit (_normal_value, _range);
	float target = limit (_target_value, _range);

	if (!_warning_visible)
		warning = _range.max();
	if (!_critical_visible)
		critical = _range.max();
	// Fill colors:
	if (_warning_visible && value >= warning)
		brush.setColor (orange.darker (100));
	if (_critical_visible && value >= critical)
		brush.setColor (red);

	float value_angle = value_span_angle * (value - _range.min()) / _range.extent();
	float warning_angle = value_span_angle * (warning - _range.min()) / _range.extent();
	float critical_angle = value_span_angle * (critical - _range.min()) / _range.extent();
	float normal_angle = value_span_angle * (normal - _range.min()) / _range.extent();
	float target_angle = value_span_angle * (target - _range.min()) / _range.extent();

	painter.save();

	if (_value_visible)
	{
		painter.save();
		painter.setPen (Qt::NoPen);
		painter.setBrush (brush);
		painter.drawPie (rect, -16.f * 0.f, -16.f * value_angle);
		painter.setPen (gray_pen);
		painter.drawLine (QPointF (0.f, 0.f), QPointF (r, 0.f));
		painter.restore();
	}

	// Warning/critical bugs:

	painter.save();

	struct PointInfo
	{
		PointInfo (float angle, QPen const& pen, float tick_len):
			angle (angle), pen (pen), tick_len (tick_len)
		{ }

		float angle; QPen pen; float tick_len;
	};

	std::vector<PointInfo> points;
	points.reserve (4);

	float gap_degs = 4;

	points.emplace_back (0.f, silver_pen, 0.f);
	if (_warning_visible)
		points.emplace_back (warning_angle, warning_pen, 0.1f * r);
	if (_critical_visible)
		points.emplace_back (critical_angle, critical_pen, 0.2f * r);
	points.emplace_back (value_span_angle, critical_pen, 0.f);

	for (auto i = 0u; i < points.size() - 1; ++i)
	{
		PointInfo curr = points[i];
		PointInfo next = points[i + 1];
		float is_last = i == points.size() - 2;

		painter.save();
		painter.setPen (curr.pen);
		painter.drawArc (rect,
						 -16.f * curr.angle,
						 -16.f * (next.angle - curr.angle - (is_last ? 0 : gap_degs)));
		painter.rotate (curr.angle);
		painter.drawLine (QPointF (r, 0.f), QPointF (r + curr.tick_len, 0.f));
		painter.restore();
	}

	// Normal value bug:
	if (_normal_visible)
	{
		painter.setPen (green_pen);
		painter.rotate (normal_angle);
		painter.drawLine (QPointF (r + pen_width (1.f), 0.f), QPointF (1.1f * r, 0.f));
		painter.drawLine (QPointF (1.1f * r + pen_width (1.f), 0.f), QPointF (1.3f * r, -0.14f * r));
		painter.drawLine (QPointF (1.1f * r + pen_width (1.f), 0.f), QPointF (1.3f * r, +0.14f * r));
	}

	painter.restore();

	// Needle:
	if (_value_visible)
	{
		painter.rotate (value_angle);
		painter.setPen (pointer_pen);
		painter.set_shadow_color (Qt::black);
		painter.set_shadow_width (1.9f);

		if (_target_visible)
		{
			float ext = 0.15f * r;
			float extr = 1.15f * r;
			painter.draw_outlined_line (QPointF (0.f, 0.f), QPointF (extr, 0.f));
			painter.rotate (target_angle - value_angle);
			painter.draw_outlined_line (QPointF (1.01f * r, 0.f), QPointF (extr, 0.f));
			painter.drawArc (rect.adjusted (-ext, -ext, +ext, +ext), arc_degs (90_deg), arc_span (1_deg * (value_angle - target_angle)));
		}
		else
			painter.draw_outlined_line (QPointF (0.f, 0.f), QPointF (0.99f * r, 0.f));
	}

	painter.restore();
}


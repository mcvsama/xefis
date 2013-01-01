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
#include <utility>
#include <cmath>

// Qt:
#include <QtCore/QTimer>
#include <QtGui/QPainter>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/numeric.h>
#include <xefis/utility/text_painter.h>

// Local:
#include "hsi_widget.h"


HSIWidget::HSIWidget (QWidget* parent):
	InstrumentWidget (parent, 0.5f, 1.f)
{ }


void
HSIWidget::paintEvent (QPaintEvent* paint_event)
{
	float const w = width();
	float const h = height();

	_aircraft_center_transform.reset();
	_aircraft_center_transform.translate (w / 2.f, 0.705f * h);
	_heading_transform.reset();
	_heading_transform.rotate (-_heading);

	// Draw on buffer:
	QPixmap buffer (w, h);
	QPainter painter (&buffer);
	TextPainter text_painter (painter, &_text_painter_cache);
	painter.setRenderHint (QPainter::Antialiasing, true);
	painter.setRenderHint (QPainter::TextAntialiasing, true);
	painter.setRenderHint (QPainter::SmoothPixmapTransform, true);
	painter.setRenderHint (QPainter::NonCosmeticDefaultPen, true);

	// Clear with black background:
	painter.setPen (Qt::NoPen);
	painter.setBrush (QBrush (QColor (0, 0, 0), Qt::SolidPattern));
	painter.drawRect (rect());

	painter.setTransform (_aircraft_center_transform);

	float q = 0.1f * wh();
	float r = 6.5f * q;
	_map_clip_rect = QRectF (-1.1f * r, -1.1f * r, 2.2f * r, 1.3f * r);
	_inside_map_clip_rect = QRectF (-0.9f * r, -0.9f * r, 1.8f * r, 0.9f * r);

	paint_track (painter, text_painter, q, r);
	paint_track_estimation (painter, text_painter, q, r);
	paint_ap_settings (painter, text_painter, q, r);
	paint_directions (painter, text_painter, q, r);
	paint_aircraft (painter, text_painter, q, r);
	paint_speeds (painter, text_painter, q, r);

	// Copy buffer to screen:
	QPainter (this).drawPixmap (paint_event->rect().topLeft(), buffer, paint_event->rect());
}


void
HSIWidget::paint_aircraft (QPainter& painter, TextPainter& text_painter, float q, float r)
{
	QPen pen1 = get_pen (QColor (255, 255, 255), 1.5f);
	QPen pen2 = get_pen (QColor (255, 255, 255), 2.8f);

	// Triangles
	{
		painter.save();

		// Big/small triangles:
		QPolygonF aircraft = QPolygonF()
			<< QPointF (0.f, 0.f)
			<< QPointF (+0.45f * q, q)
			<< QPointF (-0.45f * q, q)
			<< QPointF (0.f, 0.f);
		painter.setPen (pen1);
		painter.drawPolyline (aircraft);
		painter.translate (0.f, -r);
		painter.scale (0.5f, -0.5f);
		painter.setPen (pen2);
		painter.drawPolyline (aircraft);

		painter.restore();
	}

	// MAG heading
	if (_heading_visible)
	{
		painter.save();

		QString text_1 = "MAG";
		QString text_2 = QString ("%1").arg (static_cast<int> (_heading + 0.5f));

		QFont font_1 (_font_13_bold);
		QFont font_2 (_font_16_bold);
		QFontMetricsF metrics_1 (font_1);
		QFontMetricsF metrics_2 (font_2);
		QRectF rect_1 (0.f, 0.f, metrics_1.width (text_1), metrics_1.height());
		QRectF rect_2 (0.f, 0.f, metrics_2.width ("000"), metrics_2.height());
		rect_1.translate (0.f, translate_descent (metrics_1, metrics_2));
		rect_2.moveLeft (rect_1.right() + metrics_1.width ("  "));

		painter.setTransform (_aircraft_center_transform);
		painter.translate (q, 1.75f * q);
		painter.setPen (get_pen (_navigation_color, 1.f));
		painter.setFont (font_1);
		text_painter.drawText (rect_1, Qt::AlignLeft | Qt::AlignBottom, text_1);
		painter.setFont (font_2);
		text_painter.drawText (rect_2, Qt::AlignRight | Qt::AlignBottom, text_2);

		painter.restore();
	}
}


void
HSIWidget::paint_track (QPainter& painter, TextPainter&, float, float r)
{
	if (!_track_visible)
		return;

	QPen pen (QColor (255, 255, 0), pen_width (1.5f), Qt::DashLine, Qt::FlatCap);

	painter.save();
	painter.setClipRect (_map_clip_rect);

	painter.setPen (pen);
	painter.setTransform (_aircraft_center_transform);
	painter.rotate (_track_deg - _heading);
	painter.drawLine (QPointF (0.f, 0.f), QPointF (0.f, -r));

	painter.restore();
}


void
HSIWidget::paint_track_estimation (QPainter& painter, TextPainter&, float, float)
{
	QPen est_pen = QPen (Qt::white, pen_width (1.5f), Qt::SolidLine, Qt::SquareCap);

	painter.save();
	painter.setPen (est_pen);
	painter.setClipRect (_inside_map_clip_rect);

	if (_track_estimation_visible)
	{
		painter.setPen (est_pen);
		painter.setTransform (_aircraft_center_transform);

		Miles const step = _track_estimation_lookahead / 50.f;
		Degrees const degrees_per_step = step * _track_deviation;

		for (float pos = 0.f; pos < _track_estimation_lookahead; pos += step)
		{
			float px = nm_to_px (step);
			painter.rotate (degrees_per_step);
			painter.drawLine (QPointF (0.f, 0.f), QPointF (0.f, -px));
			painter.translate (0.f, -px);
		}
	}

	painter.restore();
}


void
HSIWidget::paint_ap_settings (QPainter& painter, TextPainter& text_painter, float q, float r)
{
	if (!_ap_heading_visible)
		return;

	// A/P bug
	{
		painter.save();
		painter.setClipRect (_map_clip_rect);

		QPolygonF bug = QPolygonF()
			<< QPointF (0.f, 0.f)
			<< QPointF (+0.45f * q, q)
			<< QPointF (+0.85f * q, q)
			<< QPointF (+0.85f * q, 0.f)
			<< QPointF (-0.85f * q, 0.f)
			<< QPointF (-0.85f * q, q)
			<< QPointF (-0.45f * q, q)
			<< QPointF (0.f, 0.f);

		for (auto& point: bug)
		{
			point.rx() *= +0.5f;
			point.ry() *= -0.5f;
		}

		QTransform transform = _aircraft_center_transform;
		transform.rotate (bound (floored_mod (_ap_heading - _heading + 180.0, 360.0) - 180.0, -102.0, +102.0));
		transform.translate (0.f, -r);

		QPen pen_1 = _autopilot_pen_1;
		pen_1.setMiterLimit (0.2f);
		QPen pen_2 = _autopilot_pen_2;
		pen_2.setMiterLimit (0.2f);

		painter.setTransform (transform);
		painter.setPen (pen_1);
		painter.drawPolyline (bug);
		painter.setPen (pen_2);
		painter.drawPolyline (bug);

		painter.restore();
	}

	// SEL HDG 000
	{
		painter.save();

		QString text_1 = "SEL  HDG";
		QString text_2 = QString ("%1").arg (static_cast<int> (_ap_heading + 0.5f));

		QFont font_1 (_font_13_bold);
		QFont font_2 (_font_16_bold);
		QFontMetricsF metrics_1 (font_1);
		QFontMetricsF metrics_2 (font_2);
		QRectF rect_1 (0.f, 0.f, metrics_1.width (text_1), metrics_1.height());
		QRectF rect_2 (0.f, 0.f, metrics_2.width ("000"), metrics_2.height());
		rect_1.translate (0.f, translate_descent (metrics_1, metrics_2));
		rect_1.moveLeft (-rect_1.right() - metrics_1.width ("  "));

		painter.setTransform (_aircraft_center_transform);
		painter.translate (-metrics_2.width ("000") - q, 1.75f * q);
		painter.setPen (_autopilot_pen_2);
		painter.setFont (font_1);
		text_painter.drawText (rect_1, Qt::AlignLeft | Qt::AlignBottom, text_1);
		painter.setFont (font_2);
		text_painter.drawText (rect_2, Qt::AlignRight | Qt::AlignBottom, text_2);

		painter.restore();
	}
}


void
HSIWidget::paint_directions (QPainter& painter, TextPainter& text_painter, float q, float r)
{
	if (!_heading_visible)
		return;

	QPen pen = get_pen (QColor (255, 255, 255), 1.5f);

	painter.save();
	painter.setClipRect (_map_clip_rect);

	painter.setPen (pen);
	painter.setFont (_font_13_bold);
	for (int deg = 0; deg < 360; deg += 5)
	{
		painter.setTransform (_heading_transform * _aircraft_center_transform);
		painter.rotate (deg);
		painter.drawLine (QPointF (0.f, -r),
						  deg % 10 == 0
							? QPointF (0.f, -0.945f * r)
							: QPointF (0.f, -0.970f * r));
		if (deg % 30 == 0)
		{
			text_painter.drawText (QRectF (-q, -0.93f * r, 2.f * q, 0.5f * q),
								   Qt::AlignVCenter | Qt::AlignHCenter, QString::number (deg / 10));
		}
	}

	painter.restore();
}


void
HSIWidget::paint_speeds (QPainter& painter, TextPainter& text_painter, float q, float)
{
	QPen pen = get_pen (QColor (255, 255, 255), 1.f);
	QFont font_a = _font_13_bold;
	QFont font_b = _font_16_bold;
	QFontMetricsF metr_a (font_a);
	QFontMetricsF metr_b (font_b);

	// Return width of painter strings:
	auto paint_speed = [&](QString str, QString val) -> float
	{
		QRectF str_rect (0.f, 0.f, metr_a.width (str) * 1.1f, metr_a.height());
		QRectF val_rect (0.f, 0.f, std::max (metr_b.width ("000"), metr_b.width (val)), metr_b.height());
		// Correct baseline position:
		str_rect.translate (0.f, translate_descent (metr_a, metr_b));
		val_rect.moveLeft (str_rect.right());

		painter.save();
		painter.setFont (font_a);
		text_painter.drawText (str_rect, Qt::AlignLeft | Qt::AlignBottom, str);
		painter.setFont (font_b);
		text_painter.drawText (val_rect, Qt::AlignRight | Qt::AlignBottom, val);
		painter.restore();

		return str_rect.width() + val_rect.width();
	};

	float offset = 0;

	painter.save();
	painter.resetTransform();
	painter.translate (0.2f * q, 0.f);
	painter.setPen (pen);

	if (_ground_speed_visible)
		offset = paint_speed ("GS", QString::number (static_cast<int> (_ground_speed)));

	if (_true_air_speed_visible)
	{
		painter.translate (offset * 1.2f, 0.f);
		paint_speed ("TAS", QString::number (static_cast<int> (_true_air_speed)));
	}

	painter.restore();
}


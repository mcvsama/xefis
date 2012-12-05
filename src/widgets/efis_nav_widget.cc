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
#include "efis_nav_widget.h"


EFISNavWidget::EFISNavWidget (QWidget* parent):
  InstrumentWidget (parent, 0.5f)
{ }


void
EFISNavWidget::paintEvent (QPaintEvent* paint_event)
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

	paint_track (painter, text_painter, q, r);
	paint_directions (painter, text_painter, q, r);
	paint_aircraft (painter, text_painter, q, r);
	paint_speeds (painter, text_painter, q, r);

	// Copy buffer to screen:
	QPainter (this).drawPixmap (paint_event->rect().topLeft(), buffer, paint_event->rect());
}


void
EFISNavWidget::paint_aircraft (QPainter& painter, TextPainter&, float q, float r)
{
	QPen pen1 = get_pen (QColor (255, 255, 255), 1.5f);
	QPen pen2 = get_pen (QColor (255, 255, 255), 2.8f);

	painter.save();

	// Heading line:
	painter.setPen (pen1);
	painter.drawLine (QPointF (0.f, 0.f), QPointF (0.f, -r));

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


void
EFISNavWidget::paint_track (QPainter& painter, TextPainter&, float, float r)
{
	if (!_track_visible)
		return;

	QPen pen (QColor (255, 255, 0), pen_width (1.5f), Qt::DashLine, Qt::FlatCap);

	painter.save();

	// Flight path line:
	QTransform track_transform = _heading_transform;
	track_transform.rotate (_track_deg);

	painter.setPen (pen);
	painter.setTransform (track_transform * _aircraft_center_transform);
	painter.drawLine (QPointF (0.f, 0.f), QPointF (0.f, -r));

	painter.restore();
}


void
EFISNavWidget::paint_directions (QPainter& painter, TextPainter& text_painter, float q, float r)
{
	if (!_heading_visible)
		return;

	QPen pen = get_pen (QColor (255, 255, 255), 1.5f);

	painter.save();

	painter.setClipRect (QRectF (-r, -r, 2.f * r, 1.1f * r));
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
EFISNavWidget::paint_speeds (QPainter& painter, TextPainter& text_painter, float q, float)
{
	QPen pen = get_pen (QColor (255, 255, 255), 1.f);
	QFont font_a = _font_13_bold;
	QFont font_b = _font_16_bold;
	QFontMetricsF metr_a (font_a);
	QFontMetricsF metr_b (font_b);

	// Return width of painter strings:
	auto paint_speed = [&](QString str, QString val) -> float
	{
		QRectF str_rect (0.f, 0.f, metr_a.width (str) * 1.1f, metr_b.height());
		QRectF val_rect (0.f, 0.f, std::max (metr_b.width ("000"), metr_b.width (val)), metr_b.height());
		// Correct baseline position:
		str_rect.translate (0.f, metr_a.descent() - metr_b.descent());
		val_rect.moveLeft (str_rect.right());

		painter.save();
		painter.setFont (font_a);
		text_painter.drawText (str_rect, Qt::AlignLeft | Qt::AlignBottom, str);
		painter.setFont (font_b);
		text_painter.drawText (val_rect, Qt::AlignLeft| Qt::AlignBottom, val);
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


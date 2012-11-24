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

	paint_flight_path (painter, text_painter);
	paint_directions (painter, text_painter);
	paint_aircraft (painter, text_painter);

	// Copy buffer to screen:
	QPainter (this).drawPixmap (paint_event->rect().topLeft(), buffer, paint_event->rect());
}


void
EFISNavWidget::paint_aircraft (QPainter& painter, TextPainter&)
{
	QPen pen1 = get_pen (QColor (255, 255, 255), 1.5f);
	QPen pen2 = get_pen (QColor (255, 255, 255), 2.8f);
	float q = 0.1f * wh();
	float r = 6.5f * q;

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
EFISNavWidget::paint_flight_path (QPainter& painter, TextPainter& text_painter)
{
	if (!_flight_path_visible)
		return;

	QPen pen (QColor (255, 255, 0), pen_width (1.5f), Qt::DashLine, Qt::FlatCap);
	float q = 0.1f * wh();
	float r = 6.5f * q;

	painter.save();

	// Flight path line:
	QTransform flight_path_transform;
	flight_path_transform.rotate (-_flight_path_beta);

	painter.setPen (pen);
	painter.setTransform (flight_path_transform * _aircraft_center_transform);
	painter.drawLine (QPointF (0.f, 0.f), QPointF (0.f, -r));

	painter.restore();
}


void
EFISNavWidget::paint_directions (QPainter& painter, TextPainter& text_painter)
{
	if (!_heading_visible)
		return;

	QPen pen = get_pen (QColor (255, 255, 255), 1.5f);
	float q = 0.1f * wh();
	float r = 6.5f * q;

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


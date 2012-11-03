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
#include <QtGui/QApplication>
#include <QtGui/QPainter>

// Xefis:
#include <xefis/utility/numeric.h>
#include <xefis/utility/text_painter.h>

// Local:
#include "efis.h"


EFIS::EFIS (QWidget* parent):
	QWidget (parent, nullptr)
{
	setAttribute (Qt::WA_NoBackground);
	_sky_color.setHsv (210, 255, 204);
	_ground_color.setHsv (30, 255, 101);
	_font = QApplication::font();

	// XXX for testing purposes only:
	QTimer* timer = new QTimer (this);
	QObject::connect (timer, SIGNAL (timeout()), this, SLOT (test()));
	timer->start (40);
}


// XXX
void
EFIS::test()
{
	set_roll (roll() + 0.1f);
	set_pitch (pitch() - 0.1f);
	set_heading (heading() + 0.25f);
}


void
EFIS::paintEvent (QPaintEvent* paint_event)
{
	const float w = width();
	const float h = height();

	float p = floored_mod (pitch() + 180.f, 360.f) - 180.f;
	float r = floored_mod (roll() + 180.f, 360.f) - 180.f;
	float hdg = floored_mod (heading(), 360.f);

	// Mirroring, eg. -180° pitch is the same
	// as 0° pitch with roll inverted:
	if (p < -90.f)
	{
		p = -180.f - p;
		r = +180.f - r;
	}
	else if (p > 90.f)
	{
		p = +180.f - p;
		r = +180.f - r;
	}

	// Prepare useful transforms:
	_center_transform.reset();
	_center_transform.translate (w / 2.f, h / 2.f);

	_pitch_transform.reset();
	_pitch_transform.translate (0.f, -pitch_to_px (p));

	_roll_transform.reset();
	_roll_transform.rotate (r);

	_heading_transform.reset();
	_heading_transform.translate (heading_to_px (hdg), 0.f);

	// Total transform of horizon (heading is not really necessary here):
	_horizon_transform = _pitch_transform * _roll_transform;

	// Draw on buffer:
	QPixmap buffer (w, h);
	QPainter painter (&buffer);
	painter.setRenderHint (QPainter::Antialiasing, true);
	painter.setRenderHint (QPainter::TextAntialiasing, true);
	painter.setRenderHint (QPainter::SmoothPixmapTransform, true);
	painter.setRenderHint (QPainter::NonCosmeticDefaultPen, true);

	paint_horizon (painter);
	paint_pitch_scale (painter);
	paint_heading (painter);
	paint_roll (painter);

	// Copy buffer to screen:
	QPainter (this).drawPixmap (paint_event->rect().topLeft(), buffer, paint_event->rect());
}


void
EFIS::paint_horizon (QPainter& painter)
{
	painter.save();
	painter.setTransform (_horizon_transform * _center_transform);

	const float max = std::max (width(), height());
	const float w_max = 2.f * max;
	const float h_max = 10.f * max;
	// Sky and ground:
	painter.fillRect (-w_max, -h_max, 2.f * w_max, h_max + 1.f, QBrush (_sky_color, Qt::SolidPattern));
	painter.fillRect (-w_max, 0.f, 2.f * w_max, h_max, QBrush (_ground_color, Qt::SolidPattern));

	painter.restore();
}


void
EFIS::paint_pitch_scale (QPainter& painter)
{
	QFont font = _font;
	font.setPixelSize (pen_width (10.f));

	const float w = std::min (width(), height()) * 2.f / 9.f;
	const float z = 0.5f * w;
	const float fpxs = font.pixelSize();

	painter.save();
	// Clip rectangle before and after rotation:
	painter.setTransform (_center_transform);
	painter.setClipRect (QRectF (-w, -w, 2.f * w, 2.f * w));
	painter.setTransform (_roll_transform * _center_transform);
	painter.setClipRect (QRectF (-w, -0.9f * w, 2.f * w, 2.f * w), Qt::IntersectClip);
	painter.setTransform (_horizon_transform * _center_transform);
	painter.setFont (font);

	TextPainter text_painter (painter);

	painter.setPen (QPen (QColor (255, 255, 255), pen_width(), Qt::SolidLine));
	// 10° lines, exclude +/-90°:
	for (int deg = -180; deg < 180; deg += 10)
	{
		if (deg == -90 || deg == 0 || deg == 90)
			continue;
		float d = pitch_to_px (deg);
		painter.drawLine (QPointF (-z, d), QPointF (z, d));
		// Degs number:
		int abs_deg = std::abs (deg);
		QString deg_t = QString::number (abs_deg > 90 ? 180 - abs_deg : abs_deg);
		// Text:
		QRectF lbox = QRectF (-z - 4.25f * fpxs, d - 0.5f * fpxs, 4.f * fpxs, fpxs);
		QRectF rbox = QRectF (+z + 0.25f * fpxs, d - 0.5f * fpxs, 4.f * fpxs, fpxs);
		text_painter.drawText (lbox, Qt::AlignVCenter | Qt::AlignRight, deg_t);
		text_painter.drawText (rbox, Qt::AlignVCenter | Qt::AlignLeft, deg_t);
	}
	// 5° lines:
	for (int deg = -180; deg < 180; deg += 5)
	{
		if (deg % 10 == 0)
			continue;
		float d = pitch_to_px (deg);
		painter.drawLine (QPointF (-z / 2.f, d), QPointF (z / 2.f, d));
	}
	// 2.5° lines:
	for (int deg = -1800; deg < 1800; deg += 25)
	{
		if (deg % 50 == 0)
			continue;
		float d = pitch_to_px (deg / 10.f);
		painter.drawLine (QPointF (-z / 4.f, d), QPointF (z / 4.f, d));
	}

	painter.setPen (QPen (QColor (255, 255, 255), pen_width (1.75f), Qt::SolidLine));
	// -90°, 90° lines:
	for (float deg: { -90.f, 90.f })
	{
		float d = pitch_to_px (deg);
		painter.drawLine (QPointF (-z * 1.5f, d), QPointF (z * 1.5f, d));
		QRectF lbox = QRectF (-1.5f * z - 4.25f * fpxs, d - 0.5f * fpxs, 4.f * fpxs, fpxs);
		QRectF rbox = QRectF (+1.5f * z + 0.25f * fpxs, d - 0.5f * fpxs, 4.f * fpxs, fpxs);
		text_painter.drawText (lbox, Qt::AlignVCenter | Qt::AlignRight, "90");
		text_painter.drawText (rbox, Qt::AlignVCenter | Qt::AlignLeft, "90");
	}

	painter.restore();
}


void
EFIS::paint_heading (QPainter& painter)
{
	QFont font = _font;
	font.setPixelSize (pen_width (10.f));

	const float w = std::min (width(), height()) * 2.f / 9.f;
	const float fpxs = font.pixelSize();

	painter.save();
	// Clip rectangle before and after rotation:
	painter.setTransform (_center_transform);
	painter.setClipRect (QRectF (-w, -w, 2.f * w, 2.f * w));
	painter.setTransform (_roll_transform * _center_transform);
	painter.setClipRect (QRectF (-w, -0.9f * w, 2.f * w, 2.f * w), Qt::IntersectClip);
	painter.setTransform (_horizon_transform * _center_transform);
	painter.setFont (font);

	TextPainter text_painter (painter);

	painter.setTransform (_horizon_transform * _center_transform);
	painter.setPen (QPen (QColor (255, 255, 255), pen_width (1.5f), Qt::SolidLine));
	painter.drawLine (QPointF (-1.25 * w, 0.f), QPointF (1.25f * w, 0.f));
	painter.setPen (QPen (QColor (255, 255, 255), pen_width(), Qt::SolidLine));

	painter.setTransform (_heading_transform * _horizon_transform * _center_transform);
	for (int deg = -360; deg < 360; deg += 10)
	{
		float d10 = heading_to_px (deg);
		float d05 = heading_to_px (deg + 5);
		// 10° lines:
		painter.drawLine (QPointF (d10, -w / 18.f), QPointF (d10, 0.f));
		text_painter.drawText (QRectF (d10 - 2.f * fpxs, 0.05f * fpxs, 4.f * fpxs, fpxs),
							   Qt::AlignVCenter | Qt::AlignHCenter, QString::number (floored_mod (1.f * deg, 360.f) / 10));
		// 5° lines:
		painter.drawLine (QPointF (d05, -w / 36.f), QPointF (d05, 0.f));
	}

	painter.restore();
}


void
EFIS::paint_roll (QPainter& painter)
{
	const float w = std::min (width(), height()) * 3.f / 9.f;
	const float h = std::min (width(), height()) * 3.f / 9.f;

	painter.save();

	QPen pen (QColor (255, 255, 255), pen_width(), Qt::SolidLine);
	pen.setJoinStyle (Qt::MiterJoin);
	painter.setPen (pen);
	painter.setBrush (QBrush (QColor (255, 255, 255)));

	painter.setTransform (_center_transform);
	painter.setClipRect (QRectF (-w, -h, 2.f * w, 2.25f * h));
	for (float deg: { -60.f, -45.f, -30.f, -20.f, -10.f, 0.f, +10.f, +20.f, +30.f, +45.f, +60.f })
	{
		painter.setTransform (_center_transform);
		painter.rotate (1.f * deg);
		painter.translate (0.f, -0.79f * h);

		if (deg == 0.f)
		{
			// Triangle:
			QPointF p0 (0.f, 0.f);
			QPointF px (0.025f * h, 0.f);
			QPointF py (0.f, 0.05f * h);
			painter.drawPolygon (QPolygonF() << p0 << p0 - px - py << p0 + px - py);
		}
		else
		{
			float length = -0.05f * h;
			if (std::abs (std::fmod (deg, 30.f)) < 1.f)
				length *= 2.f;
			painter.drawLine (QPointF (0.f, 0.f), QPointF (0.f, length));
		}
	}

	float const bold_width = pen_width (3.f);
	QPointF a (0, 0.01f * h); // Miter
	QPointF b (-0.052f * h, 0.1f * h);
	QPointF c (+0.052f * h, 0.1f * h);
	QPointF x0 (0.001f * h, 0.f);
	QPointF y0 (0.f, 0.005f * h);
	QPointF x1 (0.001f * h, 0.f);
	QPointF y1 (0.f, 1.f * bold_width);

	painter.setTransform (_roll_transform * _center_transform);
	painter.translate (0.f, -0.79f * h);
	painter.setBrush (QBrush (QColor (255, 255, 255)));
	painter.drawPolyline (QPolygonF() << b << a << c);
	painter.drawPolygon (QPolygonF() << b - x0 + y0 << b + x1 + y1 << c - x1 + y1 << c + x0 + y0);

	painter.restore();
}


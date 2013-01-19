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
HSIWidget::paintEvent (QPaintEvent*)
{
	float const w = width();
	float const h = height();

	_aircraft_center_transform.reset();
	_aircraft_center_transform.translate (w / 2.f, 0.705f * h);
	_mag_heading_transform.reset();
	_mag_heading_transform.rotate (-_mag_heading);
	_true_heading_transform.reset();
	_true_heading_transform.rotate (-_true_heading);

	QPainter painter (this);
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

	paint_dotted_earth (painter, q, r);
	paint_navaids (painter, text_painter, q, r);
	paint_track (painter, text_painter, q, r);
	paint_trend_vector (painter, text_painter, q, r);
	paint_ap_settings (painter, text_painter, q, r);
	paint_directions (painter, text_painter, q, r);
	paint_aircraft (painter, text_painter, q, r);
	paint_speeds (painter, text_painter, q, r);
}


void
HSIWidget::resizeEvent (QResizeEvent* event)
{
	InstrumentWidget::resizeEvent (event);

	float q = 0.1f * wh();
	float r = 6.5f * q;

	_map_clip_rect = QRectF (-1.1f * r, -1.1f * r, 2.2f * r, 1.3f * r);
	_inside_map_clip_rect = QRectF (-0.9f * r, -0.9f * r, 1.8f * r, 0.9f * r);

	QPainterPath clip1;
	clip1.addEllipse (QRectF (-0.85f * r, -0.85f * r, 1.7f * r, 1.7f * r));
	QPainterPath clip2;
	clip2.addEllipse (QRectF (-r, -r, 2.f * r, 2.f * r));
	QPainterPath clip3;
	clip3.addRect (QRectF (-r, -r, 2.f * r, 1.23f * r));

	_inner_map_clip = clip1 & clip3;
	_outer_map_clip = clip2 & clip3;
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
		QString text_2 = QString ("%1").arg (static_cast<int> (_mag_heading + 0.5f));

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

	QPen pen (QColor (255, 255, 0), pen_width (1.f), Qt::DashLine, Qt::FlatCap);

	painter.save();
	painter.setClipPath (_outer_map_clip);

	painter.setPen (pen);
	painter.setTransform (_aircraft_center_transform);
	painter.rotate (_track_deg - _mag_heading);
	painter.drawLine (QPointF (0.f, 0.f), QPointF (0.f, -r));

	painter.restore();
}


void
HSIWidget::paint_trend_vector (QPainter& painter, TextPainter&, float, float)
{
	QPen est_pen = QPen (Qt::white, pen_width (1.5f), Qt::SolidLine, Qt::SquareCap);

	painter.save();
	painter.setPen (est_pen);
	painter.setClipRect (_inside_map_clip_rect);

	if (_trend_vector_visible)
	{
		painter.setPen (est_pen);
		painter.setTransform (_aircraft_center_transform);

		Miles const step = _trend_vector_lookahead / 50.f;
		Degrees const degrees_per_step = step * _track_deviation;

		for (float pos = 0.f; pos < _trend_vector_lookahead; pos += step)
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

	float limited_rotation = bound (floored_mod (_ap_mag_heading - _mag_heading + 180.0, 360.0) - 180.0, -102.0, +102.0);

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
		transform.rotate (limited_rotation);
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
		QString text_2 = QString ("%1").arg (static_cast<int> (_ap_mag_heading + 0.5f));

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

	if (_ap_track_visible)
	{
		QPen pen (_autopilot_pen_2.color(), pen_width (1.5f), Qt::DashLine, Qt::FlatCap);

		painter.save();
		painter.setClipPath (_outer_map_clip);
		painter.setPen (pen);
		painter.setTransform (_aircraft_center_transform);
		painter.rotate (_ap_mag_heading - _mag_heading);
		painter.drawLine (QPointF (0.f, 0.f), QPointF (0.f, -r));
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
		painter.setTransform (_mag_heading_transform * _aircraft_center_transform);
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


void
HSIWidget::paint_dotted_earth (QPainter& painter, float q, float r)
{
	if (!_dotted_earth_visible)
		return;

	float const scale = 0.8f;
	QRectF dot (0.f, 0.f, 0.05f * q, 0.05f * q);
	dot.translate (-0.5f * dot.width(), -0.5f * dot.height());

	painter.setTransform (_mag_heading_transform * _aircraft_center_transform);
	painter.setBrush (Qt::white);
	painter.setPen (Qt::NoPen);

	for (float lat = -180; lat < 180; lat += 10)
	{
		for (float lng = -180; lng < 180; lng += 10)
		{
			LatLng point_on_earth (lat, lng);

			if (haversine (point_on_earth, _position) >= 1.7)
				continue;

			QPointF p = point_on_earth.rotate (_position).project_flat();
			painter.drawEllipse (dot.translated (p.x() * scale * r, p.y() * scale * r));
		}
	}
}


void
HSIWidget::paint_navaids (QPainter& painter, TextPainter& text_painter, float q, float)
{
	if (!_navaids_visible)
		return;

	painter.save();

	painter.setClipPath (_inner_map_clip);
	painter.setFont (_font_10_bold);
	QFontMetrics font_metrics (painter.font());

	// Unscaled pens - painter will be scaled:
	QPen ndb_pen (Qt::darkCyan, 0.08f, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
	QPen vor_pen (Qt::green, 0.08f, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
	QPen dme_pen (Qt::green, 0.08f, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
	QPen fix_pen (Qt::darkGreen, 0.08f, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
	// Scaled pens:
	QPen loc_pen (Qt::blue, pen_width (1.f), Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
	QPen hi_loc_pen (Qt::cyan, pen_width (1.f), Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);

	// TODO cache in object
	QPolygonF hexpoly = QPolygonF()
		<< QPointF (-0.5f, 0.f)
		<< QPointF (-0.25f, -0.44f)
		<< QPointF (+0.25f, -0.44f)
		<< QPointF (+0.5f, 0.f)
		<< QPointF (+0.25f, +0.44f)
		<< QPointF (-0.25f, +0.44f)
		<< QPointF (-0.5f, 0.f);

	// A bit bigger range to allow drawing objects currently positioned outside clipping path:
	NavaidStorage::Navaids navaids = _navaid_storage->get_navs (_position, std::max (_range + 20.f, 2.f * _range));
	// Sort navaids by type, draw LOCs first.
	NavaidStorage::Navaids loc_navaids;
	NavaidStorage::Navaids fix_navaids;
	NavaidStorage::Navaids other_navaids;
	for (auto& navaid: navaids)
	{
		switch (navaid.type())
		{
			case Navaid::LOC:
			case Navaid::LOCSA:
				if (_loc_visible)
					loc_navaids.insert (navaid);
				break;

			case Navaid::Fix:
				if (_fix_visible)
					fix_navaids.insert (navaid);
				break;

			default:
				other_navaids.insert (navaid);
				break;
		}
	}

	auto paint_loc = [&](Navaid const& navaid)
	{
		QPointF navaid_pos = EARTH_MEAN_RADIUS_NM * navaid.position().rotated (_position).project_flat();
		QPointF mapped_pos = _true_heading_transform.map (QPointF (nm_to_px (navaid_pos.x()), nm_to_px (navaid_pos.y())));

		QTransform centered_transform = _aircraft_center_transform;
		centered_transform.translate (mapped_pos.x(), mapped_pos.y());

		switch (navaid.type())
		{
			case Navaid::LOC:
			case Navaid::LOCSA:
			{
				float const line_1 = nm_to_px (navaid.range());
				float const line_2 = 1.03f * line_1;

				QTransform transform = _true_heading_transform * centered_transform;
				transform.rotate (navaid.true_bearing());
				QTransform rot_1; rot_1.rotate (-2.f);
				QTransform rot_2; rot_2.rotate (+2.f);
				QPointF pt_0 (0.f, line_1);
				QPointF pt_1 (rot_1.map (QPointF (0.f, line_2)));
				QPointF pt_2 (rot_2.map (QPointF (0.f, line_2)));

				painter.setTransform (transform);
				painter.setPen (navaid.identifier() == _highlighted_loc ? hi_loc_pen : loc_pen);
				painter.setBrush (Qt::NoBrush);
				if (_range < 16.f)
					painter.drawLine (QPointF (0.f, 0.f), pt_0);
				painter.drawLine (QPointF (0.f, 0.f), pt_1);
				painter.drawLine (QPointF (0.f, 0.f), pt_2);
				painter.drawLine (pt_0, pt_1);
				painter.drawLine (pt_0, pt_2);

				painter.resetTransform();
				QPointF text_offset (0.5f * font_metrics.width (navaid.identifier()), -0.35f * font_metrics.height());
				text_painter.drawText (transform.map (pt_0 + QPointF (0.f, 0.5f * q)) - text_offset, navaid.identifier());
				break;
			}

			default:
				break;
		}
	};

	auto paint_navaid = [&](Navaid const& navaid)
	{
		QPointF navaid_pos = EARTH_MEAN_RADIUS_NM * navaid.position().rotated (_position).project_flat();
		QPointF mapped_pos = _true_heading_transform.map (QPointF (nm_to_px (navaid_pos.x()), nm_to_px (navaid_pos.y())));

		QTransform centered_transform = _aircraft_center_transform;
		centered_transform.translate (mapped_pos.x(), mapped_pos.y());

		QTransform scaled_transform = centered_transform;
		scaled_transform.scale (0.7f * q, 0.7f * q);

		switch (navaid.type())
		{
			case Navaid::NDB:
				if (!_ndb_visible)
					break;
				painter.setTransform (scaled_transform);
				painter.setPen (ndb_pen);
				painter.setBrush (Qt::NoBrush);
				painter.drawEllipse (QRectF (-0.45f, -0.45f, 0.9f, 0.9f));
				painter.setBrush (Qt::cyan);
				painter.drawEllipse (QRectF (-0.07f, -0.07f, 0.14f, 0.14f));
				painter.setTransform (centered_transform);
				text_painter.drawText (QPointF (0.35 * q, 0.55f * q), navaid.identifier());
				break;

			case Navaid::VOR:
				if (!_vor_visible)
					break;
				painter.setTransform (scaled_transform);
				painter.setPen (vor_pen);
				painter.drawPolyline (hexpoly);
				painter.setBrush (Qt::green);
				painter.drawEllipse (QRectF (-0.07f, -0.07f, 0.14f, 0.14f));
				painter.setTransform (centered_transform);
				text_painter.drawText (QPointF (0.35f * q, 0.55f * q), navaid.identifier());
				break;

			case Navaid::DME:
				if (!_dme_visible)
					break;
				painter.setTransform (scaled_transform);
				painter.setPen (dme_pen);
				painter.drawRect (QRectF (-0.5f, -0.5f, 1.f, 1.f));
				break;

			case Navaid::Fix:
			{
				if (!_fix_visible)
					break;
				float const h = 0.5f;
				QPointF a (0.f, -0.66f * h);
				QPointF b (+0.5f * h, +0.33f * h);
				QPointF c (-0.5f * h, +0.33f * h);
				QPointF points[] = { a, b, c, a };
				painter.setTransform (scaled_transform);
				painter.setPen (fix_pen);
				painter.drawPolyline (points, sizeof (points) / sizeof (*points));
				painter.setTransform (centered_transform);
				text_painter.drawText (QPointF (0.f, 0.55f * q), navaid.identifier());
				break;
			}

			default:
				break;
		}
	};

	// Paint localisers:
	Navaid const* hi_loc = nullptr;
	for (auto& navaid: loc_navaids)
	{
		// Paint highlighted LOC at the end, so it's on top:
		if (navaid.identifier() == _highlighted_loc)
		{
			hi_loc = &navaid;
			continue;
		}
		paint_loc (navaid);
	}
	if (hi_loc)
		paint_loc (*hi_loc);

	// Paint Fixes:
	for (auto& navaid: fix_navaids)
		paint_navaid (navaid);

	// Other navaids:
	for (auto& navaid: other_navaids)
		paint_navaid (navaid);

	painter.restore();
}


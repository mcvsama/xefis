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
#include <vector>
#include <cmath>

// Qt:
#include <QtCore/QTimer>
#include <QtGui/QPainter>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/navaid.h>
#include <xefis/core/navaid_storage.h>
#include <xefis/utility/numeric.h>
#include <xefis/utility/text_painter.h>

// Local:
#include "hsi_widget.h"


using Xefis::Navaid;
using Xefis::NavaidStorage;


HSIWidget::HSIWidget (QWidget* parent):
	InstrumentWidget (parent, 0.5f, 1.1f, 1.f)
{ }


void
HSIWidget::update_more()
{
	// Clips:
	switch (_display_mode)
	{
		case DisplayMode::Expanded:
		{
			_q = 0.05f * height();
			_r = 0.80f * height();
			float const rx = nm_to_px (_range);

			_aircraft_center_transform.reset();
			_aircraft_center_transform.translate (0.5f * width(), 0.9f * height());

			_map_clip_rect = QRectF (-1.1f * _r, -1.1f * _r, 2.2f * _r, 2.2f * _r);
			_trend_vector_clip_rect = QRectF (-rx, -rx, 2.f * rx, rx);

			_inner_map_clip = QPainterPath();
			_inner_map_clip.addEllipse (QRectF (-0.85f * _r, -0.85f * _r, 1.7f * _r, 1.7f * _r));
			_outer_map_clip = QPainterPath();
			_outer_map_clip.addEllipse (QRectF (-rx, -rx, 2.f * rx, 2.f * rx));

			_radials_font = _font;
			_radials_font.setPixelSize (font_size (14.f));
			_radials_font.setBold (true);
			break;
		}

		case DisplayMode::Rose:
		{
			_q = 0.05f * height();
			_r = 0.40f * height();
			if (_r > 0.85f * wh())
				_r = 0.85f * wh();
			float const rx = nm_to_px (_range);

			_aircraft_center_transform.reset();
			_aircraft_center_transform.translate (0.5f * width(), 0.5 * height());

			_map_clip_rect = QRectF (-1.1f * _r, -1.1f * _r, 2.2f * _r, 2.2f * _r);
			_trend_vector_clip_rect = QRectF (-rx, -rx, 2.f * rx, rx);

			_inner_map_clip = QPainterPath();
			_inner_map_clip.addEllipse (QRectF (-0.85f * _r, -0.85f * _r, 1.7f * _r, 1.7f * _r));
			_outer_map_clip = QPainterPath();
			_outer_map_clip.addEllipse (QRectF (-rx, -rx, 2.f * rx, 2.f * rx));

			_radials_font = _font;
			_radials_font.setPixelSize (font_size (14.f));
			_radials_font.setBold (true);
			break;
		}

		case DisplayMode::Auxiliary:
		{
			_q = 0.1f * wh();
			_r = 6.5f * _q;
			float const rx = nm_to_px (_range);

			_aircraft_center_transform.reset();
			_aircraft_center_transform.translate (0.5f * width(), 0.705f * height());

			_map_clip_rect = QRectF (-1.1f * _r, -1.1f * _r, 2.2f * _r, 1.2f * _r);
			_trend_vector_clip_rect = QRectF (-rx, -rx, 2.f * rx, rx);

			QPainterPath clip1;
			clip1.addEllipse (QRectF (-0.85f * _r, -0.85f * _r, 1.7f * _r, 1.7f * _r));
			QPainterPath clip2;
			clip2.addEllipse (QRectF (-rx, -rx, 2.f * rx, 2.f * rx));
			QPainterPath clip3;
			clip3.addRect (QRectF (-rx, -rx, 2.f * rx, 1.23f * rx));

			_inner_map_clip = clip1 & clip3;
			_outer_map_clip = clip2 & clip3;

			_radials_font = _font;
			_radials_font.setPixelSize (font_size (13.f));
			_radials_font.setBold (true);
			break;
		}
	}

	// Navaids pens:
	_lo_loc_pen = QPen (Qt::blue, pen_width (0.8f), Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
	_hi_loc_pen = QPen (Qt::cyan, pen_width (0.8f), Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);

	// Unscaled pens:
	_ndb_pen = QPen (Qt::cyan, 0.09f, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
	_vor_pen = QPen (_navigation_color, 0.09f, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
	_vortac_pen = QPen (_navigation_color, 0.4f, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
	_dme_pen = QPen (_navigation_color, 0.09f, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
	_fix_pen = QPen (QColor (0, 132, 255), 0.1f, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);

	// Shapes:
	_dme_for_vor_shape = QPolygonF()
		<< QPointF (-0.5f, -0.5f)
		<< QPointF (-0.5f, +0.5f)
		<< QPointF (+0.5f, +0.5f)
		<< QPointF (+0.5f, -0.5f)
		<< QPointF (-0.5f, -0.5f);

	_vor_shape = QPolygonF()
		<< QPointF (-0.5f, 0.f)
		<< QPointF (-0.25f, -0.44f)
		<< QPointF (+0.25f, -0.44f)
		<< QPointF (+0.5f, 0.f)
		<< QPointF (+0.25f, +0.44f)
		<< QPointF (-0.25f, +0.44f)
		<< QPointF (-0.5f, 0.f);

	_aircraft_shape = QPolygonF()
		<< QPointF (0.f, 0.f)
		<< QPointF (+0.45f * _q, _q)
		<< QPointF (-0.45f * _q, _q)
		<< QPointF (0.f, 0.f);

	_ap_bug_shape = QPolygonF()
		<< QPointF (0.f, 0.f)
		<< QPointF (+0.45f * _q, _q)
		<< QPointF (+0.85f * _q, _q)
		<< QPointF (+0.85f * _q, 0.f)
		<< QPointF (-0.85f * _q, 0.f)
		<< QPointF (-0.85f * _q, _q)
		<< QPointF (-0.45f * _q, _q)
		<< QPointF (0.f, 0.f);
	for (auto& point: _ap_bug_shape)
	{
		point.rx() *= +0.5f;
		point.ry() *= -0.5f;
	}
}


void
HSIWidget::resizeEvent (QResizeEvent* event)
{
	InstrumentWidget::resizeEvent (event);
	update_more();
}


void
HSIWidget::paintEvent (QPaintEvent*)
{
	_true_track = floored_mod (_magnetic_track + (_true_heading - _magnetic_heading), 360_deg);

	_track =
		_heading_mode == HeadingMode::Magnetic
			? _magnetic_track
			: _true_track;

	_heading = _heading_mode == HeadingMode::Magnetic
		? _magnetic_heading
		: _true_heading;

	_rotation = _display_track ? _track : _heading;

	_heading_transform.reset();
	_heading_transform.rotate (-_heading.deg());

	_track_transform.reset();
	_track_transform.rotate (-_track.deg());

	_rotation_transform =
		_display_track
			? _track_transform
			: _heading_transform;

	_features_transform = _rotation_transform;
	if (_heading_mode == HeadingMode::Magnetic)
		_features_transform.rotate ((_magnetic_heading - _true_heading).deg());

	_ap_heading = _ap_magnetic_heading;
	if (_heading_mode == HeadingMode::True)
		_ap_heading += _true_heading - _magnetic_heading;
	_ap_heading = floored_mod (_ap_heading, 360_deg);

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

	paint_navaids (painter, text_painter);
	paint_altitude_reach (painter);
	paint_ap_settings (painter, text_painter);
	paint_track (painter, text_painter);
	paint_directions (painter, text_painter);
	paint_aircraft (painter, text_painter);
	paint_speeds_and_wind (painter, text_painter);
	paint_range (painter, text_painter);
	paint_hints (painter, text_painter);
	paint_trend_vector (painter, text_painter);
}


void
HSIWidget::paint_aircraft (QPainter& painter, TextPainter& text_painter)
{
	painter.setTransform (_aircraft_center_transform);
	painter.setClipping (false);

	// Aircraft triangle:
	painter.setPen (get_pen (Qt::white, 1.2f));
	painter.drawPolyline (_aircraft_shape);
	painter.translate (0.f, -_r);

	// MAG/TRUE heading
	if (_heading_visible)
	{
		int hdg = static_cast<int> ((_display_track ? _track : _heading).deg() + 0.5f) % 360;

		switch (_display_mode)
		{
			case DisplayMode::Auxiliary:
			{
				QString text_1 =
					QString (_heading_mode == HeadingMode::Magnetic ? "MAG" : "TRU") +
					QString (_display_track ? "  TRK" : "");
				QString text_2 = QString ("%1").arg (hdg);

				QFont font_1 (_font_13_bold);
				QFont font_2 (_font_16_bold);
				QFontMetricsF metrics_1 (font_1);
				QFontMetricsF metrics_2 (font_2);
				QRectF rect_1 (0.f, 0.f, metrics_1.width (text_1), metrics_1.height());
				QRectF rect_2 (0.f, 0.f, metrics_2.width ("000"), metrics_2.height());
				rect_1.translate (0.f, translate_descent (metrics_1, metrics_2));
				rect_2.moveLeft (rect_1.right() + metrics_1.width ("  "));

				painter.resetTransform();
				painter.translate (0.5f * _w + _q, _h - 1.125f * _q);
				painter.setPen (get_pen (_navigation_color, 1.f));
				painter.setFont (font_1);
				text_painter.drawText (rect_1, Qt::AlignLeft | Qt::AlignBottom, text_1);
				painter.setFont (font_2);
				text_painter.drawText (rect_2, Qt::AlignRight | Qt::AlignBottom, text_2);
				// True heading is boxed for emphasis:
				if (_heading_mode == HeadingMode::True)
				{
					painter.setBrush (Qt::NoBrush);
					painter.drawRect (rect_2.adjusted (-0.1f * _q, 0.f, +0.1f * _q, 0.f));
				}
				break;
			}

			default:
			{
				QString text_1 = _display_track ? "TRK" : "HDG";
				QString text_2 = _heading_mode == HeadingMode::Magnetic ? "MAG" : "TRU";
				QString text_v = QString ("%1").arg (hdg, 3, 10, QChar ('0'));

				float margin = 0.2f * _q;

				QFont font_1 (_font_16_bold);
				QFont font_2 (_font_20_bold);
				QFontMetricsF metrics_1 (font_1);
				QFontMetricsF metrics_2 (font_2);
				QRectF rect_v (0.f, 0.f, metrics_2.width (text_v), metrics_2.height());
				centrify (rect_v);
				rect_v.adjust (-margin, 0.f, +margin, 0.f);
				QRectF rect_1 (0.f, 0.f, metrics_1.width (text_1), metrics_1.height());
				centrify (rect_1);
				rect_1.moveRight (rect_v.left() - 0.2f * _q);
				QRectF rect_2 (0.f, 0.f, metrics_1.width (text_2), metrics_1.height());
				centrify (rect_2);
				rect_2.moveLeft (rect_v.right() + 0.2f * _q);

				painter.setTransform (_aircraft_center_transform);
				painter.translate (0.f, -_r - 1.05f * _q);
				painter.setPen (get_pen (Qt::white, 1.f));
				painter.setBrush (Qt::NoBrush);
				painter.setFont (font_2);
				painter.drawLine (rect_v.topLeft(), rect_v.bottomLeft());
				painter.drawLine (rect_v.topRight(), rect_v.bottomRight());
				painter.drawLine (rect_v.bottomLeft(), rect_v.bottomRight());
				text_painter.drawText (rect_v, Qt::AlignVCenter | Qt::AlignHCenter, text_v);
				painter.setPen (get_pen (_navigation_color, 1.f));
				painter.setFont (font_1);
				text_painter.drawText (rect_1, Qt::AlignVCenter | Qt::AlignHCenter, text_1);
				text_painter.drawText (rect_2, Qt::AlignVCenter | Qt::AlignHCenter, text_2);
				break;
			}
		}
	}
}


void
HSIWidget::paint_hints (QPainter& painter, TextPainter& text_painter)
{
	if (!_positioning_hint_visible)
		return;

	float vplus = translate_descent (QFontMetricsF (_font_13_bold), QFontMetricsF (_font_16_bold));
	float hplus = _display_mode == DisplayMode::Auxiliary ? 0.8f * _w : 0.75f * _w;
	painter.setFont (_font_13_bold);
	painter.setClipping (false);
	painter.resetTransform();
	painter.setPen (get_pen (_navigation_color, 1.f));
	text_painter.drawText (QPointF (hplus, _h - 1.125f * _q + vplus), Qt::AlignTop | Qt::AlignHCenter, _positioning_hint);
}


void
HSIWidget::paint_track (QPainter& painter, TextPainter& text_painter)
{
	Length trend_range = actual_trend_range();
	Length trend_start = actual_trend_start();
	if (2.f * trend_start > trend_range)
		trend_range = 0_nm;

	float start_point = _trend_vector_visible ? -nm_to_px (trend_range) - 0.25f * _q : 0.f;

	painter.setTransform (_aircraft_center_transform);
	painter.setClipPath (_outer_map_clip);

	QFont font = _font_13_bold;
	QFontMetricsF metrics (font);

	if (_track_visible)
	{
		// Scale and track line:
		painter.setPen (QPen (Qt::white, pen_width (1.3f)));
		painter.rotate ((_track - _rotation).deg());
		painter.drawLine (QPointF (0.f, start_point), QPointF (0.f, -_r));

		auto paint_range_tick = [&] (float ratio, bool draw_text) -> void
		{
			Length range;
			if (ratio == 0.5 && _range >= 2_nm)
				range = 1_nm * std::round (((10.f * ratio * _range) / 10.f).nm());
			else
				range = ratio * _range;
			float range_tick_vpx = nm_to_px (range);
			float range_tick_hpx = 0.1f * _q;
			int precision = 0;
			if (range < 1_nm)
				precision = 1;
			QString half_range_str = QString ("%1").arg (range.nm(), 0, 'f', precision);
			painter.drawLine (QPointF (-range_tick_hpx, -range_tick_vpx), QPointF (range_tick_hpx, -range_tick_vpx));

			if (draw_text)
			{
				QRectF half_range_rect (0.f, 0.f, metrics.width (half_range_str), metrics.height());
				centrify (half_range_rect);
				half_range_rect.moveRight (-2.f * range_tick_hpx);
				half_range_rect.translate (0.f, -range_tick_vpx);
				painter.setFont (font);
				text_painter.drawText (half_range_rect, Qt::AlignVCenter | Qt::AlignHCenter, half_range_str);
			}
		};

		paint_range_tick (0.5, true);
		if (_display_mode != DisplayMode::Auxiliary)
		{
			paint_range_tick (0.25, false);
			paint_range_tick (0.75, false);
		}
	}

	// Heading triangle:
	painter.setClipRect (_map_clip_rect);
	painter.setTransform (_aircraft_center_transform);
	painter.rotate ((_heading - _rotation).deg());

	painter.setPen (get_pen (Qt::white, 2.2f));
	painter.translate (0.f, -1.003f * _r);
	painter.scale (0.465f, -0.465f);
	painter.drawPolyline (_aircraft_shape);
}


void
HSIWidget::paint_altitude_reach (QPainter& painter)
{
	if (!_altitude_reach_visible || (_altitude_reach_distance < 0.05f * _range) || (0.8f * _range < _altitude_reach_distance))
		return;

	float len = nm_to_px (6_nm);
	float pos = nm_to_px (_altitude_reach_distance);
	QRectF rect (0.f, 0.f, len, len);
	centrify (rect);
	rect.moveTop (-pos);

	if (std::isfinite (pos))
	{
		painter.setTransform (_aircraft_center_transform);
		painter.setClipping (false);
		painter.setPen (get_pen (_navigation_color, 1.f));
		painter.drawArc (rect, arc_degs (40_deg), arc_span (-80_deg));
	}
}


void
HSIWidget::paint_trend_vector (QPainter& painter, TextPainter&)
{
	QPen est_pen = QPen (Qt::white, pen_width (1.f), Qt::SolidLine, Qt::SquareCap);

	painter.setTransform (_aircraft_center_transform);
	painter.setClipPath (_inner_map_clip);
	painter.setPen (est_pen);

	Length trend_range = actual_trend_range();
	Length trend_start = actual_trend_start();
	if (2.f * trend_start > trend_range)
		trend_range = 0_nm;

	if (_trend_vector_visible)
	{
		painter.setPen (est_pen);
		painter.setTransform (_aircraft_center_transform);
		painter.setClipRect (_trend_vector_clip_rect);

		Length const step = trend_range / 50.f;
		Angle const angle_per_step = step.nm() * _track_deviation;

		for (Length pos = 0_nm; pos < trend_range; pos += step)
		{
			float px = nm_to_px (step);
			painter.rotate (angle_per_step.deg());
			if (pos > trend_start)
				painter.drawLine (QPointF (0.f, 0.f), QPointF (0.f, -px));
			painter.translate (0.f, -px);
		}
	}
}


void
HSIWidget::paint_ap_settings (QPainter& painter, TextPainter& text_painter)
{
	if (!_ap_heading_visible)
		return;

	// A/P bug
	painter.setTransform (_aircraft_center_transform);
	painter.setClipRect (_map_clip_rect);

	Angle limited_rotation;
	switch (_display_mode)
	{
		case DisplayMode::Auxiliary:
			limited_rotation = limit (floored_mod (_ap_heading - _rotation + 180_deg, 360_deg) - 180_deg, -96_deg, +96_deg);
			break;

		default:
			limited_rotation = _ap_heading - _rotation;
			break;
	}

	QTransform transform = _aircraft_center_transform;
	transform.rotate (limited_rotation.deg());
	transform.translate (0.f, -_r);

	QPen pen_1 = _autopilot_pen_1;
	pen_1.setMiterLimit (0.2f);
	QPen pen_2 = _autopilot_pen_2;
	pen_2.setMiterLimit (0.2f);

	painter.setTransform (transform);
	painter.setPen (pen_1);
	painter.drawPolyline (_ap_bug_shape);
	painter.setPen (pen_2);
	painter.drawPolyline (_ap_bug_shape);

	// SEL HDG 000
	if (_display_mode == DisplayMode::Auxiliary)
	{
		painter.setTransform (_aircraft_center_transform);
		painter.setClipping (false);

		// AP heading always set as magnetic, but can be displayed as true:
		QString text_1 = "SEL  HDG";
		QString text_2 = QString ("%1").arg (static_cast<int> (_ap_heading.deg() + 0.5f));

		QFont font_1 (_font_13_bold);
		QFont font_2 (_font_16_bold);
		QFontMetricsF metrics_1 (font_1);
		QFontMetricsF metrics_2 (font_2);
		QRectF rect_1 (0.f, 0.f, metrics_1.width (text_1), metrics_1.height());
		QRectF rect_2 (0.f, 0.f, metrics_2.width ("000"), metrics_2.height());
		rect_1.translate (0.f, translate_descent (metrics_1, metrics_2));
		rect_1.moveLeft (-rect_1.right() - metrics_1.width ("  "));

		painter.resetTransform();
		painter.translate (0.5f * _w - metrics_2.width ("000") - _q, _h - 1.125f * _q);
		painter.setPen (_autopilot_pen_2);
		painter.setFont (font_1);
		text_painter.drawText (rect_1, Qt::AlignLeft | Qt::AlignBottom, text_1);
		painter.setFont (font_2);
		text_painter.drawText (rect_2, Qt::AlignRight | Qt::AlignBottom, text_2);
	}

	if (_ap_track_visible)
	{
		QPen pen (_autopilot_pen_2.color(), pen_width (1.f), Qt::DashLine, Qt::FlatCap);
		pen.setDashPattern (QVector<qreal>() << 7.5 << 12);

		painter.setTransform (_aircraft_center_transform);
		painter.setClipPath (_outer_map_clip);
		painter.setPen (pen);
		painter.setTransform (_aircraft_center_transform);
		painter.rotate ((_ap_heading - _rotation).deg());
		painter.drawLine (QPointF (0.f, 0.f), QPointF (0.f, -_r));
	}
}


void
HSIWidget::paint_directions (QPainter& painter, TextPainter& text_painter)
{
	if (!_heading_visible)
		return;

	QPen pen = QPen (QColor (255, 255, 255), pen_width (1.f), Qt::SolidLine, Qt::FlatCap);

	painter.setTransform (_aircraft_center_transform);
	painter.setClipRect (_map_clip_rect);
	painter.setPen (pen);
	painter.setFont (_radials_font);

	QTransform t = _rotation_transform * _aircraft_center_transform;

	for (int deg = 0; deg < 360; deg += 5)
	{
		painter.setTransform (t);
		painter.rotate (deg);
		painter.drawLine (QPointF (0.f, -_r),
						  deg % 10 == 0
							? QPointF (0.f, -0.935f * _r)
							: QPointF (0.f, -0.965f * _r));
		if (deg % 30 == 0)
		{
			text_painter.drawText (QRectF (-_q, -0.93f * _r, 2.f * _q, 0.5f * _q),
								   Qt::AlignVCenter | Qt::AlignHCenter, QString::number (deg / 10));
		}
	}

	switch (_display_mode)
	{
		case DisplayMode::Expanded:
			// Circle around radials:
			painter.setBrush (Qt::NoBrush);
			painter.drawEllipse (QRectF (-_r, -_r, 2.f * _r, 2.f * _r));
			break;

		case DisplayMode::Rose:
			painter.setClipping (false);
			painter.setTransform (_aircraft_center_transform);
			// 8 lines around the circle:
			for (int deg = 45; deg < 360; deg += 45)
			{
				painter.rotate (45);
				painter.drawLine (QPointF (0.f, -1.025f * _r), QPointF (0.f, -1.125f * _r));
			}
			break;

		case DisplayMode::Auxiliary:
			break;
	}
}


void
HSIWidget::paint_speeds_and_wind (QPainter& painter, TextPainter& text_painter)
{
	QPen pen = get_pen (QColor (255, 255, 255), 0.6f);
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

		painter.setFont (font_a);
		text_painter.drawText (str_rect, Qt::AlignLeft | Qt::AlignBottom, str);
		painter.setFont (font_b);
		text_painter.drawText (val_rect, Qt::AlignRight | Qt::AlignBottom, val);

		return str_rect.width() + val_rect.width();
	};

	float offset = 0;

	painter.resetTransform();
	painter.translate (0.2f * _q, 0.f);
	if (_display_mode == DisplayMode::Expanded || _display_mode == DisplayMode::Rose)
		painter.translate (0.f, 0.15f * _q);
	painter.setClipping (false);
	painter.setPen (pen);

	if (_ground_speed_visible)
		offset = paint_speed ("GS", QString::number (static_cast<int> (_ground_speed)));

	if (_true_air_speed_visible)
	{
		painter.translate (offset * 1.2f, 0.f);
		paint_speed ("TAS", QString::number (static_cast<int> (_true_air_speed)));
	}

	if (_wind_information_visible)
	{
		QString wind_str = QString ("%1°/%2")
			.arg (static_cast<long> (_wind_from_magnetic_heading.deg()), 3, 10, QChar ('0'))
			.arg (static_cast<long> (_wind_tas_speed), 3, 10, QChar (L'\u2007'));
		painter.resetTransform();
		painter.translate (0.2f * _q, metr_b.height());
		if (_display_mode == DisplayMode::Expanded || _display_mode == DisplayMode::Rose)
			painter.translate (0.f, 0.15f * _q);
		painter.setPen (get_pen (Qt::white, 1.2f));
		text_painter.drawText (QPointF (0.f, 0.f), Qt::AlignTop | Qt::AlignLeft, wind_str);
		painter.translate (0.8f * _q, 0.8f * _q + metr_b.height());
		painter.rotate ((_wind_from_magnetic_heading - _magnetic_heading + 180_deg).deg());
		QPointF a = QPointF (0.f, -0.7f * _q);
		QPointF b = QPointF (0.f, +0.7f * _q);
		painter.drawLine (a + QPointF (0.f, 0.03f * _q), b);
		painter.drawLine (a, a + QPointF (+0.15f * _q, +0.15f * _q));
		painter.drawLine (a, a + QPointF (-0.15f * _q, +0.15f * _q));
	}
}


void
HSIWidget::paint_range (QPainter& painter, TextPainter& text_painter)
{
	if (_display_mode == DisplayMode::Expanded || _display_mode == DisplayMode::Rose)
	{
		QFont font_a = _font_10_bold;
		font_a.setPixelSize (font_size (11.f));
		QFont font_b = _font_16_bold;
		QFontMetricsF metr_a (font_a);
		QFontMetricsF metr_b (font_b);
		QString s ("RANGE");
		QString r (QString ("%1").arg (_range.nm(), 0, 'f', 0));

		QRectF rect (0.f, 0.f, std::max (metr_a.width (s), metr_b.width (r)) + 0.4f * _q, metr_a.height() + metr_b.height());

		painter.setClipping (false);
		painter.resetTransform();
		painter.translate (4.5f * _q, 0.25f * _q);
		painter.setPen (get_pen (Qt::white, 1.0f));
		painter.drawRect (rect);
		painter.setFont (font_a);
		text_painter.drawText (rect.center() - QPointF (0.f, 0.05f * _q), Qt::AlignBottom | Qt::AlignHCenter, s);
		painter.setFont (font_b);
		text_painter.drawText (rect.center() - QPointF (0.f, 0.135f * _q), Qt::AlignTop | Qt::AlignHCenter, r);
	}
}


void
HSIWidget::paint_navaids (QPainter& painter, TextPainter& text_painter)
{
	if (!_navaids_visible)
		return;

	painter.setTransform (_aircraft_center_transform);
	painter.setClipPath (_outer_map_clip);
	painter.setFont (_font_10_bold);

	retrieve_navaids();

	paint_locs (painter, text_painter);

	auto paint_navaid = [&](Navaid const& navaid)
	{
		QPointF mapped_pos = get_navaid_xy (navaid.position());
		QTransform centered_transform = _aircraft_center_transform;
		centered_transform.translate (mapped_pos.x(), mapped_pos.y());

		QTransform scaled_transform = centered_transform;
		scaled_transform.scale (0.55f * _q, 0.55f * _q);

		switch (navaid.type())
		{
			case Navaid::NDB:
			{
				if (!_ndb_visible)
					break;
				QPainterPath b_point;
				b_point.addEllipse (QRectF (-0.07f, -0.07f, 0.14f, 0.14f));
				QPainterPath s_point;
				s_point.addEllipse (QRectF (-0.035f, -0.035f, 0.07f, 0.07f));
				QPainterPath point_1 = s_point.translated (0.f, -0.35f);
				QPainterPath point_2 = s_point.translated (0.f, -0.55f);

				painter.setTransform (scaled_transform);
				painter.setPen (_ndb_pen);
				painter.setBrush (_ndb_pen.color());
				painter.drawPath (b_point);
				for (int i = 0; i < 12; ++i)
				{
					painter.rotate (30.f);
					painter.drawPath (point_1);
				}
				painter.rotate (15.f);
				for (int i = 0; i < 18; ++i)
				{
					painter.rotate (20.f);
					painter.drawPath (point_2);
				}
				painter.setTransform (centered_transform);
				text_painter.drawText (QPointF (0.35 * _q, 0.55f * _q), navaid.identifier());
				break;
			}

			case Navaid::VOR:
				if (!_vor_visible)
					break;
				painter.setTransform (scaled_transform);
				painter.setPen (_vor_pen);
				painter.drawPolyline (_vor_shape);
				painter.setBrush (_navigation_color);
				painter.drawEllipse (QRectF (-0.07f, -0.07f, 0.14f, 0.14f));
				if (navaid.vor_type() == Navaid::VOR_DME)
					painter.drawPolyline (_dme_for_vor_shape);
				else if (navaid.vor_type() == Navaid::VORTAC)
				{
					float x = 0.28f;
					float y = 0.64f;
					painter.setPen (_vortac_pen);
					painter.drawLine (QPointF (-x, y), QPointF (+x, y));
					painter.rotate (120.f);
					painter.drawLine (QPointF (-x, y), QPointF (+x, y));
					painter.rotate (120.f);
					painter.drawLine (QPointF (-x, y), QPointF (+x, y));
				}
				painter.setTransform (centered_transform);
				text_painter.drawText (QPointF (0.35f * _q, 0.55f * _q), navaid.identifier());
				break;

			case Navaid::DME:
				if (!_dme_visible)
					break;
				painter.setTransform (scaled_transform);
				painter.setPen (_dme_pen);
				painter.drawRect (QRectF (-0.5f, -0.5f, 1.f, 1.f));
				break;

			case Navaid::Fix:
			{
				if (!_fix_visible)
					break;
				float const h = 0.75f;
				QPointF a (0.f, -0.66f * h);
				QPointF b (+0.5f * h, +0.33f * h);
				QPointF c (-0.5f * h, +0.33f * h);
				QPointF points[] = { a, b, c, a };
				painter.setTransform (scaled_transform);
				painter.setPen (_fix_pen);
				painter.drawPolyline (points, sizeof (points) / sizeof (*points));
				painter.setTransform (centered_transform);
				painter.translate (0.5f, 0.5f);
				text_painter.drawText (QPointF (0.25f * _q, 0.45f * _q), navaid.identifier());
				break;
			}

			default:
				break;
		}
	};

	for (auto& navaid: _fix_navs)
		paint_navaid (navaid);

	for (auto& navaid: _ndb_navs)
		paint_navaid (navaid);

	for (auto& navaid: _dme_navs)
		paint_navaid (navaid);

	for (auto& navaid: _vor_navs)
		paint_navaid (navaid);
}


void
HSIWidget::paint_locs (QPainter& painter, TextPainter& text_painter)
{
	QFontMetricsF font_metrics (painter.font());
	QTransform rot_1; rot_1.rotate (-2.f);
	QTransform rot_2; rot_2.rotate (+2.f);
	QPointF zero (0.f, 0.f);

	// Group painting lines and texts as separate tasks. For this,
	// cache texts that need to be drawn later along with their positions.
	std::vector<std::pair<QPointF, QString>> texts_to_paint;
	texts_to_paint.reserve (128);

	auto paint_texts_to_paint = [&]() -> void
	{
		painter.resetTransform();
		for (auto const& text_and_xy: texts_to_paint)
			text_painter.drawText (text_and_xy.first, text_and_xy.second);
		texts_to_paint.clear();
	};

	auto paint_loc = [&] (Navaid const& navaid) -> void
	{
		QPointF navaid_pos = get_navaid_xy (navaid.position());
		QTransform transform = _aircraft_center_transform;
		transform.translate (navaid_pos.x(), navaid_pos.y());
		transform = _features_transform * transform;
		transform.rotate (navaid.true_bearing().deg());

		float const line_1 = nm_to_px (navaid.range());
		float const line_2 = 1.03f * line_1;

		QPointF pt_0 (0.f, line_1);
		QPointF pt_1 (rot_1.map (QPointF (0.f, line_2)));
		QPointF pt_2 (rot_2.map (QPointF (0.f, line_2)));

		painter.setTransform (transform);
		if (_range < 16_nm)
			painter.drawLine (zero, pt_0);
		painter.drawLine (zero, pt_1);
		painter.drawLine (zero, pt_2);
		painter.drawLine (pt_0, pt_1);
		painter.drawLine (pt_0, pt_2);

		QPointF text_offset (0.5f * font_metrics.width (navaid.identifier()), -0.35f * font_metrics.height());
		texts_to_paint.emplace_back (transform.map (pt_0 + QPointF (0.f, 0.6f * _q)) - text_offset, navaid.identifier());
	};

	// Paint localizers:
	painter.setBrush (Qt::NoBrush);
	painter.setPen (_lo_loc_pen);
	Navaid const* hi_loc = nullptr;
	for (auto& navaid: _loc_navs)
	{
		// Paint highlighted LOC at the end, so it's on top:
		if (navaid.identifier() == _highlighted_loc)
			hi_loc = &navaid;
		else
			paint_loc (navaid);
	}

	// Paint identifiers:
	paint_texts_to_paint();

	// Highlighted localizer with text:
	if (hi_loc)
	{
		painter.setPen (_hi_loc_pen);
		paint_loc (*hi_loc);
		paint_texts_to_paint();
	}
}


void
HSIWidget::retrieve_navaids()
{
	if (!_navaid_storage)
		return;

	if (_navs_retrieved && _navs_retrieve_position.haversine_earth (_position) < 0.1f * _range && _range == _navs_retrieve_range)
		return;

	_loc_navs.clear();
	_ndb_navs.clear();
	_vor_navs.clear();
	_dme_navs.clear();
	_fix_navs.clear();

	for (Navaid const& navaid: _navaid_storage->get_navs (_position, std::max (_range + 20_nm, 2.f * _range)))
	{
		switch (navaid.type())
		{
			case Navaid::LOC:
			case Navaid::LOCSA:
				_loc_navs.push_back (navaid);
				break;

			case Navaid::NDB:
				_ndb_navs.push_back (navaid);
				break;

			case Navaid::VOR:
				_vor_navs.push_back (navaid);
				break;

			case Navaid::DME:
			case Navaid::DMESF:
				_dme_navs.push_back (navaid);
				break;

			case Navaid::Fix:
				_fix_navs.push_back (navaid);
				break;

			default:
				// Other types not drawn.
				break;
		}
	}

	_navs_retrieved = true;
	_navs_retrieve_position = _position;
	_navs_retrieve_range = _range;
}


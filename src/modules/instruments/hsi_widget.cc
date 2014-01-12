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
#include <xefis/core/window.h>
#include <xefis/utility/numeric.h>
#include <xefis/utility/painter.h>

// Local:
#include "hsi_widget.h"


using Xefis::Navaid;
using Xefis::NavaidStorage;


void
HSIWidget::Parameters::sanitize()
{
	using Xefis::limit;
	using Xefis::floored_mod;

	range = limit (range, 1_ft, 5000_nm);
	heading_magnetic = floored_mod (heading_magnetic, 360_deg);
	heading_true = floored_mod (heading_true, 360_deg);
	ap_magnetic_heading = floored_mod (ap_magnetic_heading, 360_deg);
	track_magnetic = floored_mod (track_magnetic, 360_deg);
	true_home_direction = floored_mod (true_home_direction, 360_deg);
	wind_from_magnetic_heading = floored_mod (wind_from_magnetic_heading, 360_deg);
}


HSIWidget::PaintWorkUnit::PaintWorkUnit (HSIWidget* hsi_widget):
	InstrumentWidget::PaintWorkUnit (hsi_widget),
	InstrumentAids (0.5f)
{ }


void
HSIWidget::PaintWorkUnit::pop_params()
{
	_params = _params_next;
	_locals = _locals_next;
}


void
HSIWidget::PaintWorkUnit::resized()
{
	InstrumentAids::update_sizes (size(), window_size());

	// Clips:
	switch (_params.display_mode)
	{
		case DisplayMode::Expanded:
		{
			_q = 0.0500f * size().height();
			_r = 0.7111f * size().height();
			float const rx = nm_to_px (_params.range);

			_aircraft_center_transform.reset();
			_aircraft_center_transform.translate (0.5f * size().width(), 0.8f * size().height());

			_map_clip_rect = QRectF (-1.1f * _r, -1.1f * _r, 2.2f * _r, 2.2f * _r);
			_trend_vector_clip_rect = QRectF (-rx, -rx, 2.f * rx, rx);

			_inner_map_clip = QPainterPath();
			_inner_map_clip.addEllipse (QRectF (-0.85f * _r, -0.85f * _r, 1.7f * _r, 1.7f * _r));
			_outer_map_clip = QPainterPath();
			if (_params.round_clip)
				_outer_map_clip.addEllipse (QRectF (-rx, -rx, 2.f * rx, 2.f * rx));
			else
				_outer_map_clip.addRect (QRectF (-rx, -rx, 2.f * rx, 2.f * rx));

			_radials_font = _font;
			_radials_font.setPixelSize (font_size (16.f));
			break;
		}

		case DisplayMode::Rose:
		{
			_q = 0.05f * size().height();
			_r = 0.40f * size().height();
			if (_r > 0.85f * wh())
				_r = 0.85f * wh();
			float const rx = nm_to_px (_params.range);

			_aircraft_center_transform.reset();
			_aircraft_center_transform.translate (0.5f * size().width(), 0.5 * size().height());

			_map_clip_rect = QRectF (-1.1f * _r, -1.1f * _r, 2.2f * _r, 2.2f * _r);
			_trend_vector_clip_rect = QRectF (-rx, -rx, 2.f * rx, rx);

			_inner_map_clip = QPainterPath();
			_inner_map_clip.addEllipse (QRectF (-0.85f * _r, -0.85f * _r, 1.7f * _r, 1.7f * _r));
			_outer_map_clip = QPainterPath();
			if (_params.round_clip)
				_outer_map_clip.addEllipse (QRectF (-rx, -rx, 2.f * rx, 2.f * rx));
			else
				_outer_map_clip.addRect (QRectF (-rx, -rx, 2.f * rx, 2.f * rx));

			_radials_font = _font;
			_radials_font.setPixelSize (font_size (16.f));
			break;
		}

		case DisplayMode::Auxiliary:
		{
			_q = 0.1f * wh();
			_r = 6.5f * _q;
			float const rx = nm_to_px (_params.range);

			_aircraft_center_transform.reset();
			_aircraft_center_transform.translate (0.5f * size().width(), 0.705f * size().height());

			_map_clip_rect = QRectF (-1.1f * _r, -1.1f * _r, 2.2f * _r, 1.11f * _r);
			_trend_vector_clip_rect = QRectF (-rx, -rx, 2.f * rx, rx);

			QPainterPath clip1;
			clip1.addEllipse (QRectF (-0.85f * _r, -0.85f * _r, 1.7f * _r, 1.7f * _r));
			QPainterPath clip2;
			if (_params.round_clip)
				clip2.addEllipse (QRectF (-rx, -rx, 2.f * rx, 2.f * rx));
			else
				clip2.addRect (QRectF (-rx, -rx, 2.f * rx, 2.f * rx));
			QPainterPath clip3;
			clip3.addRect (QRectF (-rx, -rx, 2.f * rx, 1.45f * rx));

			_inner_map_clip = clip1 & clip3;
			_outer_map_clip = clip2 & clip3;

			_radials_font = _font;
			_radials_font.setPixelSize (font_size (13.f));
			break;
		}
	}

	// Navaids pens:
	_lo_loc_pen = QPen (Qt::blue, pen_width (0.8f), Qt::SolidLine, Qt::RoundCap, Qt::BevelJoin);
	_hi_loc_pen = QPen (Qt::cyan, pen_width (0.8f), Qt::SolidLine, Qt::RoundCap, Qt::BevelJoin);

	// Unscaled pens:
	_ndb_pen = QPen (QColor (88, 88, 88), 0.09f, Qt::SolidLine, Qt::RoundCap, Qt::BevelJoin);
	_vor_pen = QPen (QColor (0, 132, 255), 0.09f, Qt::SolidLine, Qt::RoundCap, Qt::BevelJoin);
	_dme_pen = QPen (QColor (0, 132, 255), 0.09f, Qt::SolidLine, Qt::RoundCap, Qt::BevelJoin);
	_fix_pen = QPen (QColor (0, 132, 255), 0.1f, Qt::SolidLine, Qt::RoundCap, Qt::BevelJoin);
	_home_pen = QPen (Qt::green, 0.1f, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin);

	// Shapes:
	_ndb_shape = QPainterPath();
	{
		QPainterPath s_point;
		s_point.addEllipse (QRectF (-0.035f, -0.035f, 0.07f, 0.07f));
		QPainterPath point_1 = s_point.translated (0.f, -0.35f);
		QPainterPath point_2 = s_point.translated (0.f, -0.55f);
		QTransform t;

		_ndb_shape.addEllipse (QRectF (-0.07f, -0.07f, 0.14f, 0.14f));
		for (int i = 0; i < 12; ++i)
		{
			t.rotate (30.f);
			_ndb_shape.addPath (t.map (point_1));
		}
		t.rotate (15.f);
		for (int i = 0; i < 18; ++i)
		{
			t.rotate (20.f);
			_ndb_shape.addPath (t.map (point_2));
		}
	}

	_dme_for_vor_shape = QPolygonF()
		<< QPointF (-0.5f, -0.5f)
		<< QPointF (-0.5f, +0.5f)
		<< QPointF (+0.5f, +0.5f)
		<< QPointF (+0.5f, -0.5f)
		<< QPointF (-0.5f, -0.5f);

	QTransform t;
	_vortac_shape = QPolygonF();
	t.rotate (60.f);
	for (int i = 0; i < 4; ++i)
	{
		float const x = 0.18f;
		float const y1 = 0.28f;
		float const y2 = 0.48f;
		_vortac_shape << t.map (QPointF (-x, -y1));
		if (i == 3)
			break;
		_vortac_shape << t.map (QPointF (-x, -y2));
		_vortac_shape << t.map (QPointF (+x, -y2));
		_vortac_shape << t.map (QPointF (+x, -y1));
		t.rotate (120.f);
	}

	_vor_shape = QPolygonF()
		<< QPointF (-0.5f, 0.f)
		<< QPointF (-0.25f, -0.44f)
		<< QPointF (+0.25f, -0.44f)
		<< QPointF (+0.5f, 0.f)
		<< QPointF (+0.25f, +0.44f)
		<< QPointF (-0.25f, +0.44f)
		<< QPointF (-0.5f, 0.f);

	_home_shape = QPolygonF()
		<< QPointF (-0.4f, 0.f)
		<< QPointF (0.f, -0.5f)
		<< QPointF (+0.4f, 0.f)
		<< QPointF (0.f, +0.5f)
		<< QPointF (-0.4f, 0.f);

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
HSIWidget::PaintWorkUnit::paint (QImage& image)
{
	auto paint_token = get_token (&image);

	_current_datetime = QDateTime::currentDateTime();

	if (_recalculation_needed)
	{
		_recalculation_needed = false;
		resized();
	}

	_locals.track_true = Xefis::floored_mod (_params.track_magnetic + (_params.heading_true - _params.heading_magnetic), 360_deg);

	_locals.track =
		_params.heading_mode == HeadingMode::Magnetic
			? _params.track_magnetic
			: _locals.track_true;

	_locals.heading = _params.heading_mode == HeadingMode::Magnetic
		? _params.heading_magnetic
		: _params.heading_true;

	_locals.rotation = _params.center_on_track ? _locals.track : _locals.heading;

	_heading_transform.reset();
	_heading_transform.rotate (-_locals.heading.deg());

	_track_transform.reset();
	_track_transform.rotate (-_locals.track.deg());

	_rotation_transform =
		_params.center_on_track
			? _track_transform
			: _heading_transform;

	_features_transform = _rotation_transform;
	if (_params.heading_mode == HeadingMode::Magnetic)
		_features_transform.rotate ((_params.heading_magnetic - _params.heading_true).deg());

	_pointers_transform = _rotation_transform;
	if (_params.heading_mode == HeadingMode::True)
		_pointers_transform.rotate ((_params.heading_true - _params.heading_magnetic).deg());

	_locals.ap_heading = _params.ap_magnetic_heading;
	if (_params.heading_mode == HeadingMode::True)
		_locals.ap_heading += _params.heading_true - _params.heading_magnetic;
	_locals.ap_heading = Xefis::floored_mod (_locals.ap_heading, 360_deg);

	if (_params.course_setting_magnetic)
	{
		_locals.course_heading = *_params.course_setting_magnetic;
		if (_params.heading_mode == HeadingMode::True)
			_locals.course_heading += _params.heading_true - _params.heading_magnetic;
		_locals.course_heading = Xefis::floored_mod (_locals.course_heading, 360_deg);
	}

	painter().set_shadow_color (Qt::black);
	clear_background();

	paint_navaids (painter());
	paint_altitude_reach (painter());
	paint_track (painter(), false);
	paint_directions (painter());
	paint_track (painter(), true);
	paint_ap_settings (painter());
	paint_speeds_and_wind (painter());
	paint_home_direction (painter());
	paint_climb_glide_ratio (painter());
	paint_range (painter());
	paint_hints (painter());
	paint_trend_vector (painter());
	paint_tcas();
	paint_course (painter());
	paint_pointers (painter());
	paint_aircraft (painter());
}


void
HSIWidget::PaintWorkUnit::paint_aircraft (Xefis::Painter& painter)
{
	painter.setTransform (_aircraft_center_transform);
	painter.setClipping (false);

	// Aircraft triangle - shadow and triangle:
	painter.setPen (get_pen (Qt::white, 1.f));
	painter.add_shadow ([&]() {
		painter.drawPolyline (_aircraft_shape);
	});

	painter.translate (0.f, -_r);

	// AP info: SEL HDG 000
	if (_params.display_mode == DisplayMode::Auxiliary)
	{
		painter.setTransform (_aircraft_center_transform);
		painter.setClipping (false);

		int sel_hdg = static_cast<int> (_locals.ap_heading.deg() + 0.5f) % 360;
		if (sel_hdg == 0)
			sel_hdg = 360;

		// AP heading always set as magnetic, but can be displayed as true:
		QString text_1 = "SEL HDG";
		QString text_2 = QString ("%1").arg (sel_hdg, 3, 10, QChar ('0'));

		QFont font_1 (_font_13);
		QFont font_2 (_font_16);
		QFontMetricsF metrics_1 (font_1);
		QFontMetricsF metrics_2 (font_2);
		QRectF rect_1 (0.f, 0.f, metrics_1.width (text_1), metrics_1.height());
		QRectF rect_2 (0.f, 0.f, metrics_2.width ("000"), metrics_2.height());
		rect_1.translate (0.f, translate_descent (metrics_1, metrics_2));
		rect_1.moveLeft (-rect_1.right() - metrics_1.width (" "));

		painter.resetTransform();
		painter.translate (0.5f * _w - metrics_2.width ("000") - _q, _h - 1.125f * _q);
		// Background:
		painter.setPen (Qt::NoPen);
		painter.setBrush (Qt::black);
		float a = 0.1 * metrics_2.height();
		painter.drawRect (rect_1.united (rect_2).adjusted (-a, 0.0, a, 0.0));
		// Texts:
		painter.setPen (_autopilot_pen_2);
		painter.setFont (font_1);
		painter.fast_draw_text (rect_1, Qt::AlignLeft | Qt::AlignBottom, text_1);
		painter.setFont (font_2);
		painter.fast_draw_text (rect_2, Qt::AlignRight | Qt::AlignBottom, text_2);
	}

	// MAG/TRUE heading
	if (_params.heading_visible)
	{
		int hdg = static_cast<int> ((_params.center_on_track ? _locals.track : _locals.heading).deg() + 0.5f) % 360;
		if (hdg == 0)
			hdg = 360;

		switch (_params.display_mode)
		{
			case DisplayMode::Auxiliary:
			{
				QString text_1 =
					QString (_params.heading_mode == HeadingMode::Magnetic ? "MAG" : "TRU") +
					QString (_params.center_on_track ? " TRK" : "");
				QString text_2 = QString ("%1").arg (hdg, 3, 10, QChar ('0'));

				QFont font_1 (_font_13);
				QFont font_2 (_font_16);
				QFontMetricsF metrics_1 (font_1);
				QFontMetricsF metrics_2 (font_2);
				QRectF rect_1 (0.f, 0.f, metrics_1.width (text_1), metrics_1.height());
				QRectF rect_2 (0.f, 0.f, metrics_2.width ("000"), metrics_2.height());
				rect_1.translate (0.f, translate_descent (metrics_1, metrics_2));
				rect_2.moveLeft (rect_1.right() + metrics_1.width (" "));

				painter.resetTransform();
				painter.translate (0.5f * _w + _q, _h - 1.125f * _q);
				// Background:
				painter.setPen (Qt::NoPen);
				painter.setBrush (Qt::black);
				float a = 0.1 * metrics_2.height();
				painter.drawRect (rect_1.united (rect_2).adjusted (-a, 0.0, a, 0.0));
				// Texts:
				painter.setPen (get_pen (_navigation_color, 1.f));
				painter.setFont (font_1);
				painter.fast_draw_text (rect_1, Qt::AlignLeft | Qt::AlignBottom, text_1);
				painter.setFont (font_2);
				painter.fast_draw_text (rect_2, Qt::AlignRight | Qt::AlignBottom, text_2);
				// True heading is boxed for emphasis:
				if (_params.heading_mode == HeadingMode::True)
				{
					painter.setBrush (Qt::NoBrush);
					painter.add_shadow ([&]() {
						painter.drawRect (rect_2.adjusted (-0.1f * _q, 0.f, +0.1f * _q, 0.f));
					});
				}
				break;
			}

			default:
			{
				QString text_1 = _params.center_on_track ? "TRK" : "HDG";
				QString text_2 = _params.heading_mode == HeadingMode::Magnetic ? "MAG" : "TRU";
				QString text_v = QString ("%1").arg (hdg, 3, 10, QChar ('0'));

				float margin = 0.2f * _q;

				QFont font_1 (_font_16);
				QFont font_2 (_font_20);
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
				painter.fast_draw_text (rect_v, Qt::AlignVCenter | Qt::AlignHCenter, text_v);
				painter.setPen (get_pen (_navigation_color, 1.f));
				painter.setFont (font_1);
				painter.fast_draw_text (rect_1, Qt::AlignVCenter | Qt::AlignHCenter, text_1);
				painter.fast_draw_text (rect_2, Qt::AlignVCenter | Qt::AlignHCenter, text_2);
				break;
			}
		}
	}
}


void
HSIWidget::PaintWorkUnit::paint_hints (Xefis::Painter& painter)
{
	if (!_params.positioning_hint_visible || !_params.position)
		return;

	float vplus = translate_descent (QFontMetricsF (_font_13), QFontMetricsF (_font_16));
	float hplus = _params.display_mode == DisplayMode::Auxiliary ? 0.8f * _w : 0.75f * _w;
	painter.setFont (_font_13);
	QFontMetricsF metrics (painter.font());
	QPointF text_hook = QPointF (hplus, _h - 1.125f * _q + vplus);
	painter.setClipping (false);
	painter.resetTransform();
	// Background:
	QRectF text_box = painter.get_text_box (text_hook, Qt::AlignTop | Qt::AlignHCenter, _params.positioning_hint);
	painter.setPen (Qt::NoPen);
	painter.setBrush (Qt::black);
	float a = 0.1 * metrics.height();
	painter.drawRect (text_box.adjusted (-a, 0.0, a, 0.0));
	// Text:
	painter.setPen (get_pen (_navigation_color, 1.f));
	painter.fast_draw_text (text_hook, Qt::AlignTop | Qt::AlignHCenter, _params.positioning_hint);
	// Box for emphasis:
	if (_params.positioning_hint != "" && is_newly_set (_locals.positioning_hint_ts))
	{
		float v = 0.03f * _q;
		QRectF frame (0.f, 0.f, metrics.width (_params.positioning_hint), metrics.height());
		frame.moveTo (text_hook + QPointF (0.f, 0.5f * metrics.height()));
		centrify (frame);
		frame.adjust (-0.1f * _q, -v, +0.1f * _q, +0.5f * v);
		painter.setBrush (Qt::NoBrush);
		painter.add_shadow ([&]() {
			painter.drawRect (frame);
		});
	}
}


void
HSIWidget::PaintWorkUnit::paint_track (Xefis::Painter& painter, bool paint_heading_triangle)
{
	Length trend_range = actual_trend_range();
	Length trend_start = actual_trend_start();
	if (2.f * trend_start > trend_range)
		trend_range = 0_nm;

	float start_point = _params.trend_vector_visible ? -nm_to_px (trend_range) - 0.25f * _q : 0.f;

	painter.setTransform (_aircraft_center_transform);
	painter.setClipping (false);

	QFont font = _font_13;
	QFontMetricsF metrics (font);

	if (!paint_heading_triangle && _params.track_visible)
	{
		// Scale and track line:
		painter.setPen (QPen (_silver, pen_width (1.3f), Qt::SolidLine, Qt::RoundCap));
		painter.rotate ((_locals.track - _locals.rotation).deg());
		float extension = 0.0;
		if (_params.display_mode != DisplayMode::Auxiliary && _params.center_on_track)
			extension = 0.6 * _q;
		painter.draw_outlined_line (QPointF (0.f, start_point), QPointF (0.f, -_r - extension));
		painter.setPen (QPen (Qt::white, pen_width (1.3f), Qt::SolidLine, Qt::RoundCap));

		auto paint_range_tick = [&] (float ratio, bool draw_text) -> void
		{
			Length range;
			if (ratio == 0.5 && _params.range >= 2_nm)
				range = 1_nm * std::round (((10.f * ratio * _params.range) / 10.f).nm());
			else
				range = ratio * _params.range;
			float range_tick_vpx = nm_to_px (range);
			float range_tick_hpx = 0.1f * _q;
			int precision = 0;
			if (range < 1_nm)
				precision = 1;
			QString half_range_str = QString ("%1").arg (range.nm(), 0, 'f', precision);
			painter.draw_outlined_line (QPointF (-range_tick_hpx, -range_tick_vpx), QPointF (range_tick_hpx, -range_tick_vpx));

			if (draw_text)
			{
				QRectF half_range_rect (0.f, 0.f, metrics.width (half_range_str), metrics.height());
				centrify (half_range_rect);
				half_range_rect.moveRight (-2.f * range_tick_hpx);
				half_range_rect.translate (0.f, -range_tick_vpx);
				painter.setFont (font);
				painter.fast_draw_text (half_range_rect, Qt::AlignVCenter | Qt::AlignHCenter, half_range_str);
			}
		};

		paint_range_tick (0.5, true);
		if (_params.display_mode != DisplayMode::Auxiliary)
		{
			paint_range_tick (0.25, false);
			paint_range_tick (0.75, false);
		}
	}

	if (_params.heading_visible && paint_heading_triangle)
	{
		// Heading triangle:
		painter.setClipRect (_map_clip_rect);
		painter.setTransform (_aircraft_center_transform);
		painter.rotate ((_locals.heading - _locals.rotation).deg());

		painter.setPen (get_pen (Qt::white, 2.2f));
		painter.translate (0.f, -1.003f * _r);
		painter.scale (0.465f, -0.465f);
		painter.add_shadow ([&]() {
			painter.drawPolyline (_aircraft_shape);
		});
	}
}


void
HSIWidget::PaintWorkUnit::paint_altitude_reach (Xefis::Painter& painter)
{
	if (!_params.altitude_reach_visible || (_params.altitude_reach_distance < 0.005f * _params.range) || (0.8f * _params.range < _params.altitude_reach_distance))
		return;

	float len = Xefis::limit (nm_to_px (6_nm), 2.f * _q, 7.f * _q);
	float pos = nm_to_px (_params.altitude_reach_distance);
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
HSIWidget::PaintWorkUnit::paint_trend_vector (Xefis::Painter& painter)
{
	QPen est_pen = QPen (Qt::white, pen_width (1.f), Qt::SolidLine, Qt::RoundCap);

	painter.setTransform (_aircraft_center_transform);
	painter.setClipPath (_inner_map_clip);
	painter.setPen (est_pen);

	Length trend_range = actual_trend_range();
	Length trend_start = actual_trend_start();
	if (2.f * trend_start > trend_range)
		trend_range = 0_nm;

	if (_params.trend_vector_visible)
	{
		painter.setPen (est_pen);
		painter.setTransform (_aircraft_center_transform);
		painter.setClipRect (_trend_vector_clip_rect);

		Length initial_step = trend_range / 150.f;
		Length normal_step = trend_range / 10.f;
		Length step = initial_step;
		Angle const angle_per_step = step.nm() * _params.track_lateral_delta;

		QTransform transform;
		QPolygonF polygon;

		for (Length pos = 0_nm; pos < trend_range; pos += step)
		{
			step = pos > trend_range ? normal_step : initial_step;
			float px = nm_to_px (step);
			transform.rotate (angle_per_step.deg());
			if (pos > trend_start)
				polygon << transform.map (QPointF (0.f, -px));
			transform.translate (0.f, -px);
		}

		painter.add_shadow ([&]() {
			painter.drawPolyline (polygon);
		});
	}
}


void
HSIWidget::PaintWorkUnit::paint_ap_settings (Xefis::Painter& painter)
{
	if (!_params.ap_heading_visible)
		return;

	// AP dashed line:
	if (_params.ap_track_visible)
	{
		double pink_pen_width = 1.5f;
		double shadow_pen_width = 2.5f;
		if (_params.display_mode == DisplayMode::Auxiliary)
		{
			pink_pen_width = 1.2f;
			shadow_pen_width = 2.2f;
		}

		float const shadow_scale = shadow_pen_width / pink_pen_width;

		QPen pen (_autopilot_pen_2.color(), pen_width (pink_pen_width), Qt::DashLine, Qt::RoundCap);
		pen.setDashPattern (QVector<qreal>() << 7.5 << 12);

		QPen shadow_pen (painter.shadow_color(), pen_width (shadow_pen_width), Qt::DashLine, Qt::RoundCap);
		shadow_pen.setDashPattern (QVector<qreal>() << 7.5 / shadow_scale << 12 / shadow_scale);

		painter.setTransform (_aircraft_center_transform);
		painter.setClipPath (_outer_map_clip);
		painter.rotate ((_locals.ap_heading - _locals.rotation).deg());

		for (auto const& p: { shadow_pen, pen })
		{
			painter.setPen (p);
			painter.drawLine (QPointF (0.f, 0.f), QPointF (0.f, -_r));
		}
	}

	// A/P bug
	if (_params.heading_visible)
	{
		Angle limited_rotation;
		switch (_params.display_mode)
		{
			case DisplayMode::Auxiliary:
				limited_rotation = Xefis::floored_mod (_locals.ap_heading - _locals.rotation + 180_deg, 360_deg) - 180_deg;
				break;

			default:
				limited_rotation = _locals.ap_heading - _locals.rotation;
				break;
		}

		QTransform transform = _aircraft_center_transform;
		transform.rotate (limited_rotation.deg());
		transform.translate (0.f, -_r);

		QPen pen_1 = _autopilot_pen_1;
		pen_1.setMiterLimit (0.2f);
		QPen pen_2 = _autopilot_pen_2;
		pen_2.setMiterLimit (0.2f);

		painter.setTransform (_aircraft_center_transform);
		painter.setClipRect (_map_clip_rect);
		painter.setTransform (transform);
		painter.setPen (pen_1);
		painter.drawPolyline (_ap_bug_shape);
		painter.setPen (pen_2);
		painter.drawPolyline (_ap_bug_shape);
	}
}


void
HSIWidget::PaintWorkUnit::paint_directions (Xefis::Painter& painter)
{
	if (!_params.heading_visible)
		return;

	QPen pen (Qt::white, pen_width (1.f), Qt::SolidLine, Qt::RoundCap);

	painter.setTransform (_aircraft_center_transform);
	painter.setClipRect (_map_clip_rect);
	painter.setPen (pen);
	painter.setFont (_radials_font);
	painter.setBrush (Qt::NoBrush);

	QTransform t = _rotation_transform * _aircraft_center_transform;

	painter.add_shadow ([&]() {
		painter.setTransform (_aircraft_center_transform);

		QPointF line_long;
		QPointF line_short;
		float radial_ypos;

		if (_params.display_mode == DisplayMode::Auxiliary)
		{
			line_long = QPointF (0.f, -0.935f * _r);
			line_short = QPointF (0.f, -0.965f * _r);
			radial_ypos = -0.925f * _r;
		}
		else
		{
			line_long = QPointF (0.f, -0.955f * _r);
			line_short = QPointF (0.f, -0.980f * _r);
			radial_ypos = -0.945f * _r;
		}

		for (int deg = 5; deg <= 360; deg += 5)
		{
			QPointF sp = deg % 10 == 0 ? line_long : line_short;
			painter.setTransform (t);
			painter.rotate (deg);
			painter.drawLine (QPointF (0.f, -_r + 0.025 * _q), sp);

			if (!painter.painting_shadow())
			{
				if (deg % 30 == 0)
					painter.fast_draw_text (QRectF (-_q, radial_ypos, 2.f * _q, 0.5f * _q),
											Qt::AlignVCenter | Qt::AlignHCenter, QString::number (deg / 10));
			}
		}

		// Circle around radials:
		if (_params.display_mode == DisplayMode::Expanded)
			painter.drawEllipse (QRectF (-_r, -_r, 2.f * _r, 2.f * _r));
	});

	if (_params.display_mode == DisplayMode::Rose)
	{
		painter.setClipping (false);
		painter.setTransform (_aircraft_center_transform);
		// 8 lines around the circle:
		for (int deg = 45; deg < 360; deg += 45)
		{
			painter.rotate (45);
			painter.draw_outlined_line (QPointF (0.f, -1.025f * _r), QPointF (0.f, -1.125f * _r));
		}
	}
}


void
HSIWidget::PaintWorkUnit::paint_speeds_and_wind (Xefis::Painter& painter)
{
	QPen pen = get_pen (Qt::white, 0.6f);
	QFont font_a = _font_13;
	QFont font_b = _font_18;
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
		painter.fast_draw_text (str_rect, Qt::AlignLeft | Qt::AlignBottom, str);
		painter.setFont (font_b);
		painter.fast_draw_text (val_rect, Qt::AlignRight | Qt::AlignBottom, val);

		return str_rect.width() + val_rect.width();
	};

	float offset = 0;

	painter.resetTransform();
	painter.translate (0.2f * _q, 0.f);
	if (_params.display_mode == DisplayMode::Expanded || _params.display_mode == DisplayMode::Rose)
		painter.translate (0.f, 0.15f * _q);
	painter.setClipping (false);
	painter.setPen (pen);

	if (_params.ground_speed_visible)
		offset = paint_speed ("GS", QString::number (static_cast<int> (_params.ground_speed.kt())));

	if (_params.true_air_speed_visible)
	{
		painter.translate (offset * 1.2f, 0.f);
		paint_speed ("TAS", QString::number (static_cast<int> (_params.true_air_speed.kt())));
	}

	if (_params.wind_information_visible)
	{
		QString wind_str = QString ("%1°/%2")
			.arg (static_cast<long> (_params.wind_from_magnetic_heading.deg()), 3, 10, QChar ('0'))
			.arg (static_cast<long> (_params.wind_tas_speed.kt()), 3, 10, QChar (L'\u2007'));
		painter.resetTransform();
		painter.translate (0.2f * _q, metr_b.height());
		if (_params.display_mode == DisplayMode::Expanded || _params.display_mode == DisplayMode::Rose)
			painter.translate (0.f, 0.15f * _q);
		painter.setPen (get_pen (Qt::white, 1.0f));
		painter.fast_draw_text (QPointF (0.f, 0.f), Qt::AlignTop | Qt::AlignLeft, wind_str);

		painter.translate (0.8f * _q, 0.8f * _q + metr_b.height());
		painter.rotate ((_params.wind_from_magnetic_heading - _params.heading_magnetic + 180_deg).deg());
		painter.setPen (get_pen (Qt::white, 1.0));
		painter.add_shadow ([&]() {
			QPointF a = QPointF (0.f, -0.7f * _q);
			QPointF b = QPointF (0.f, +0.7f * _q);
			painter.drawLine (a + QPointF (0.f, 0.05f * _q), b);
			painter.drawLine (a, a + QPointF (+0.15f * _q, +0.15f * _q));
			painter.drawLine (a, a + QPointF (-0.15f * _q, +0.15f * _q));
		});
	}
}


void
HSIWidget::PaintWorkUnit::paint_home_direction (Xefis::Painter& painter)
{
	if (_params.display_mode != DisplayMode::Auxiliary)
		return;

	if (!_params.position || !_params.home)
		return;

	QTransform base_transform;
	base_transform.translate (_w - 0.2 * _q, 0.5f * _h);

	painter.resetTransform();
	painter.setClipping (false);

	if (_params.home_direction_visible)
	{
		bool at_home = _params.home->haversine_earth (*_params.position) < 10_m;
		float z = 0.75f * _q;

		painter.setTransform (base_transform);
		painter.translate (-z - 0.1f * _q, _q);
		if (at_home)
		{
			painter.setPen (get_pen (_navigation_color, 1.25));
			float v = 0.35f * z;
			painter.setBrush (Qt::black);
			painter.drawEllipse (QRectF (-v, -v, 2.f * v, 2.f * v));
		}
		else
		{
			painter.setPen (get_pen (_navigation_color, 1.0));
			QPolygonF home_arrow = QPolygonF()
				<< QPointF (0.f, z)
				<< QPointF (0.f, 0.2f * -z)
				<< QPointF (-0.2f * z, 0.6f * -z)
				<< QPointF (0.f, -z)
				<< QPointF (+0.2f * z, 0.6f * -z)
				<< QPointF (0.f, 0.2f * -z);
			painter.rotate ((_params.true_home_direction - _params.heading_true).deg());
			painter.add_shadow ([&]() {
				painter.drawPolyline (home_arrow);
			});
		}
	}

	if (_params.dist_to_home_ground_visible || _params.dist_to_home_vlos_visible || _params.dist_to_home_vert_visible)
	{
		float z = 0.99f * _q;
		float h = 0.66f * _q;
		QPolygonF distance_triangle = QPolygonF()
			<< QPointF (z, 0.f)
			<< QPointF (0.f, 0.f)
			<< QPointF (z, -h);

		painter.setPen (get_pen (Qt::white, 1.0));
		painter.setFont (_font_13);
		painter.setTransform (base_transform);

		if (_params.dist_to_home_vlos_visible)
		{
			QString s = QString ("%1").arg (_params.dist_to_home_vlos.nm(), 0, 'f', 2, QChar ('0'));
			// Cut out the "0.":
			if (s.left (2) == "0.")
				s = s.mid (1);
			painter.fast_draw_text (QPointF (0.f, -1.75f * _q), Qt::AlignRight | Qt::AlignBottom, s);
		}

		if (_params.dist_to_home_ground_visible)
		{
			QString s = QString ("%1").arg (_params.dist_to_home_ground.nm(), 0, 'f', 2, QChar ('0'));
			// Cut out the "0.":
			if (s.left (2) == "0.")
				s = s.mid (1);
			painter.fast_draw_text (QPointF (0.f, -0.75f * _q), Qt::AlignRight | Qt::AlignTop, s);
		}

		if (_params.dist_to_home_vert_visible)
		{
			QString s = QString ("%1\u2008↑").arg (static_cast<int> (_params.dist_to_home_vert.ft()));
			painter.fast_draw_text (QPointF (0.f, -2.4 * _q), Qt::AlignRight | Qt::AlignBottom, s);
		}

		painter.translate (-z - 0.1f * _q, -_q);
		painter.setPen (get_pen (Qt::gray, 1.0));
		painter.add_shadow ([&]() {
			painter.drawPolyline (distance_triangle);
		});
	}
}


void
HSIWidget::PaintWorkUnit::paint_course (Xefis::Painter& painter)
{
	if (!_params.heading_visible || !_params.course_setting_magnetic)
		return;

	painter.setTransform (_aircraft_center_transform);
	painter.setClipPath (_outer_map_clip);
	painter.setTransform (_aircraft_center_transform);
	painter.rotate ((_locals.course_heading - _locals.rotation).deg());

	double k = 1.0;
	double z = 1.0;
	double pink_pen_width = 1.5f;
	double shadow_pen_width = 2.5;
	QFont font = _font_20;

	switch (_params.display_mode)
	{
		case DisplayMode::Expanded:
			k = _r / 15.0;
			z = _q / 6.0;
			break;

		case DisplayMode::Rose:
			k = _r / 10.0;
			z = _q / 7.0;
			break;

		case DisplayMode::Auxiliary:
			k = _r / 10.0;
			z = _q / 7.0;
			pink_pen_width = 1.2f;
			shadow_pen_width = 2.2f;
			font = _font_16;
			break;
	}

	double shadow_scale = shadow_pen_width / pink_pen_width;
	float dev_1_deg_px = 1.5f * k;

	// Front pink line:
	QPen front_pink_pen = get_pen (_autopilot_pen_2.color(), pink_pen_width);
	QPen front_shadow_pen = get_pen (painter.shadow_color(), shadow_pen_width);
	for (auto const& p: { front_shadow_pen, front_pink_pen })
	{
		painter.setPen (p);
		painter.drawLine (QPointF (0.0, -3.5 * k), QPointF (0.0, -0.99 * _r));
	}

	// Back pink line:
	QPen back_pink_pen = get_pen (_autopilot_pen_2.color(), pink_pen_width, Qt::DashLine);
	back_pink_pen.setDashPattern (QVector<qreal>() << 7.5 << 12);

	QPen back_shadow_pen = get_pen (painter.shadow_color(), shadow_pen_width);
	back_shadow_pen.setDashPattern (QVector<qreal>() << 7.5 / shadow_scale << 12 / shadow_scale);

	for (auto const& p: { back_shadow_pen, back_pink_pen })
	{
		painter.setPen (p);
		painter.drawLine (QPointF (0.0, 3.5 * k - z), QPointF (0.0, 0.99 * _r));
	}

	// White bars:
	painter.setPen (get_pen (Qt::white, 1.2f));
	QPolygonF top_bar = QPolygonF()
		<< QPointF (0.0, -3.5 * k)
		<< QPointF (-z, -3.5 * k + z)
		<< QPointF (-z, -2.5 * k)
		<< QPointF (+z, -2.5 * k)
		<< QPointF (+z, -3.5 * k + z)
		<< QPointF (0.0, -3.5 * k);
	QPolygonF bottom_bar = QPolygonF()
		<< QPointF (-z, 2.5 * k)
		<< QPointF (-z, 3.5 * k - z)
		<< QPointF (+z, 3.5 * k - z)
		<< QPointF (+z, 2.5 * k)
		<< QPointF (-z, 2.5 * k);
	painter.add_shadow ([&]() {
		painter.drawPolyline (top_bar);
		painter.drawPolyline (bottom_bar);
	});

	// Deviation bar:
	if (_params.course_deviation)
	{
		bool filled = false;
		Angle deviation = Xefis::limit (*_params.course_deviation, -2.5_deg, +2.5_deg);
		if (std::abs (_params.course_deviation->deg()) <= std::abs (deviation.deg()))
			filled = true;

		double pw = pen_width (1.75f);
		QRectF bar (-z, -2.5f * k + pw, 2.f * z, 5.f * k - 2.f * pw);
		bar.translate (dev_1_deg_px * deviation.deg(), 0.f);

		painter.setPen (get_pen (Qt::black, 2.0));
		painter.setBrush (Qt::NoBrush);
		painter.drawRect (bar);

		painter.setPen (_autopilot_pen_2);
		if (filled)
			painter.setBrush (_autopilot_pen_2.color());
		else
			painter.setBrush (Qt::NoBrush);
		painter.drawRect (bar);
	}

	// Deviation scale:
	QRectF elli (0.f, 0.f, 0.25f * _q, 0.25f * _q);
	elli.translate (-elli.width() / 2.f, -elli.height() / 2.f);

	painter.setPen (get_pen (Qt::white, 2.f));
	painter.setBrush (Qt::NoBrush);
	painter.add_shadow ([&]() {
		for (float x: { -2.f, -1.f, +1.f, +2.f })
			painter.drawEllipse (elli.translated (dev_1_deg_px * x, 0.f));
	});

	// TO/FROM flag - always on the right, regardless of rotation.
	if (_params.course_to_flag)
	{
		QString text = *_params.course_to_flag ? "TO" : "FROM";
		Qt::Alignment flags = Qt::AlignLeft | Qt::AlignVCenter;
		QPointF position (4.0f * k, 0.f);

		painter.setTransform (_aircraft_center_transform);
		painter.setPen (get_pen (Qt::white, 1.f));
		painter.setFont (_font_20);
		painter.fast_draw_text (position, flags, text);
	}
}


void
HSIWidget::PaintWorkUnit::paint_pointers (Xefis::Painter& painter)
{
	if (!_params.heading_visible)
		return;

	painter.resetTransform();
	painter.setClipping (false);

	struct Opts
	{
		bool			primary;
		QColor			color;
		Optional<Angle>	angle;
	};

	QColor cyan (0x00, 0xdd, 0xff);

	for (Opts const& opts: { Opts { true, Qt::green, _params.pointer_green_primary },
							 Opts { false, Qt::green, _params.pointer_green_secondary },
							 Opts { true, cyan, _params.pointer_cyan_primary },
							 Opts { false, cyan, _params.pointer_cyan_secondary } })
	{
		if (!opts.angle)
			continue;

		float width = 1.5f;
		if (_params.display_mode == DisplayMode::Auxiliary)
			width = 1.2f;

		painter.setPen (get_pen (opts.color, width));
		painter.setTransform (_aircraft_center_transform);
		painter.setClipRect (_map_clip_rect);
		painter.setTransform (_pointers_transform * _aircraft_center_transform);
		painter.rotate (opts.angle->deg());

		if (opts.primary)
		{
			double z = 0.13 * _q;
			double delta = 0.5 * z;

			double to_top = -_r - 3.0 * z;
			double to_bottom = -_r + 12.0 * z;

			double from_top = _r - 11.0 * z;
			double from_bottom = _r + 3.0 * z;

			painter.add_shadow ([&]() {
				painter.drawLine (QPointF (0.0, to_top + delta), QPointF (0.0, to_bottom));
				painter.drawLine (QPointF (0.0, to_top), QPointF (+z, to_top + 1.4 * z));
				painter.drawLine (QPointF (0.0, to_top), QPointF (-z, to_top + 1.4 * z));
				painter.drawLine (QPointF (-2.0 * z, to_bottom - 0.5 * z), QPointF (+2.0 * z, to_bottom - 0.5 * z));

				painter.drawLine (QPointF (0.0, from_top), QPointF (0.0, from_bottom));
				painter.drawLine (QPointF (-2.0 * z, from_bottom - 1.3 * z), QPointF (0.0, from_bottom - 2.0 * z));
				painter.drawLine (QPointF (+2.0 * z, from_bottom - 1.3 * z), QPointF (0.0, from_bottom - 2.0 * z));
			});
		}
		else
		{
			double z = 0.13 * _q;

			double to_top = -_r - 3.0 * z;
			double to_bottom = -_r + 11.0 * z;
			QPolygonF top_arrow = QPolygonF()
				<< QPointF (0.0, to_top)
				<< QPointF (+z, to_top + 1.2 * z)
				<< QPointF (+z, to_bottom)
				<< QPointF (+2.5 * z, to_bottom)
				<< QPointF (+2.5 * z, to_bottom + 1.7 * z)
				<< QPointF (-2.5 * z, to_bottom + 1.7 * z)
				<< QPointF (-2.5 * z, to_bottom)
				<< QPointF (-z, to_bottom)
				<< QPointF (-z, to_top + 1.2 * z)
				<< QPointF (0.0, to_top);

			double from_top = _r - 12.0 * z;
			double from_bottom = _r;
			QPolygonF bottom_arrow = QPolygonF()
				<< QPointF (0.0, from_top)
				<< QPointF (+z, from_top + 1.2 * z)
				<< QPointF (+z, from_bottom)
				<< QPointF (+2.5 * z, from_bottom + z)
				<< QPointF (+2.5 * z, from_bottom + 2.7 * z)
				<< QPointF (0.0, from_bottom + 1.7 * z)
				<< QPointF (-2.5 * z, from_bottom + 2.7 * z)
				<< QPointF (-2.5 * z, from_bottom + z)
				<< QPointF (-z, from_bottom)
				<< QPointF (-z, from_top + 1.2 * z)
				<< QPointF (0.0, from_top);

			painter.add_shadow ([&]() {
				painter.drawPolyline (top_arrow);
				painter.drawPolyline (bottom_arrow);
			});
		}
	}
}


void
HSIWidget::PaintWorkUnit::paint_climb_glide_ratio (Xefis::Painter& painter)
{
	if (!_params.climb_glide_ratio_visible)
		return;

	_params.climb_glide_ratio = Xefis::limit<float> (_params.climb_glide_ratio, -99, 99);

	QPen pen = get_pen (Qt::white, 0.6f);
	QFont font_a = _font_13;
	QFont font_b = _font_18;
	QFontMetricsF metr_a (font_a);
	QFontMetricsF metr_b (font_b);

	painter.resetTransform();
	painter.translate (-0.2f * _q, 0.f);
	painter.setClipping (false);
	painter.setPen (pen);

	if (_params.display_mode == DisplayMode::Expanded || _params.display_mode == DisplayMode::Rose)
		painter.translate (0.f, 0.15f * _q);

	QString str, arr, val;

	if (_params.climb_glide_ratio > 0.0)
	{
		str = "CR";
		arr = "↑";
	}
	else if (_params.climb_glide_ratio < -0.0)
	{
		str = "GR";
		arr = "↓";
	}
	else
	{
		str = "CGR";
		arr = "";
	}

	if (std::abs (_params.climb_glide_ratio) > 0.0)
		val = arr + "\u2009" + QString ("%1\u2009˸1").arg (std::abs (static_cast<int> (_params.climb_glide_ratio)), 2, 10, QChar (L'\u2007'));
	else
		val = "––";

	painter.setFont (font_a);
	painter.fast_draw_text (QPointF (_w - metr_b.width ("\u20090000:1"), translate_descent (metr_a, metr_b)), Qt::AlignTop | Qt::AlignRight, str);
	painter.setFont (font_b);
	painter.fast_draw_text (QPointF (_w, metr_b.height()), Qt::AlignBottom | Qt::AlignRight, val);
}


void
HSIWidget::PaintWorkUnit::paint_range (Xefis::Painter& painter)
{
	if (_params.display_mode == DisplayMode::Expanded || _params.display_mode == DisplayMode::Rose)
	{
		QFont font_a = _font_10;
		font_a.setPixelSize (font_size (11.f));
		QFont font_b = _font_16;
		QFontMetricsF metr_a (font_a);
		QFontMetricsF metr_b (font_b);
		QString s ("RANGE");
		QString r (QString ("%1").arg (_params.range.nm(), 0, 'f', 0));

		QRectF rect (0.f, 0.f, std::max (metr_a.width (s), metr_b.width (r)) + 0.4f * _q, metr_a.height() + metr_b.height());

		painter.setClipping (false);
		painter.resetTransform();
		painter.translate (5.5f * _q, 0.25f * _q);
		painter.setPen (get_pen (Qt::white, 1.0f));
		painter.setBrush (Qt::black);
		painter.drawRect (rect);
		painter.setFont (font_a);
		painter.fast_draw_text (rect.center() - QPointF (0.f, 0.05f * _q), Qt::AlignBottom | Qt::AlignHCenter, s);
		painter.setFont (font_b);
		painter.fast_draw_text (rect.center() - QPointF (0.f, 0.135f * _q), Qt::AlignTop | Qt::AlignHCenter, r);
	}
}


void
HSIWidget::PaintWorkUnit::paint_navaids (Xefis::Painter& painter)
{
	if (!_params.navaids_visible || !_params.position)
		return;

	float scale = 0.55f * _q;

	painter.setTransform (_aircraft_center_transform);
	painter.setClipPath (_outer_map_clip);
	painter.setFont (_font_10);

	retrieve_navaids();
	paint_locs (painter);

	// Return feature position on screen relative to _aircraft_center_transform.
	auto position_feature = [&](LonLat const& position, bool* limit_to_range = nullptr) -> QPointF
	{
		QPointF mapped_pos = get_navaid_xy (position);

		if (limit_to_range)
		{
			double range = 0.95f * _r;
			double rpx = std::sqrt (mapped_pos.x() * mapped_pos.x() + mapped_pos.y() * mapped_pos.y());
			*limit_to_range = rpx >= range;
			if (*limit_to_range)
			{
				QTransform rot;
				rot.rotate ((1_rad * std::atan2 (mapped_pos.y(), mapped_pos.x())).deg());
				mapped_pos = rot.map (QPointF (range, 0.0));
			}
		}

		return mapped_pos;
	};

	auto paint_navaid = [&](Navaid const& navaid)
	{
		QTransform feature_centered_transform = _aircraft_center_transform;
		QPointF translation = position_feature (navaid.position());
		feature_centered_transform.translate (translation.x(), translation.y());

		QTransform feature_scaled_transform = feature_centered_transform;
		feature_scaled_transform.scale (scale, scale);

		switch (navaid.type())
		{
			case Navaid::NDB:
			{
				painter.setTransform (feature_scaled_transform);
				painter.setPen (_ndb_pen);
				painter.setBrush (_ndb_pen.color());
				painter.drawPath (_ndb_shape);
				painter.setTransform (feature_centered_transform);
				painter.fast_draw_text (QPointF (0.35 * _q, 0.55f * _q), navaid.identifier());
				break;
			}

			case Navaid::VOR:
				painter.setTransform (feature_scaled_transform);
				painter.setPen (_vor_pen);
				painter.setBrush (_navigation_color);
				if (navaid.vor_type() == Navaid::VOROnly)
				{
					painter.drawEllipse (QRectF (-0.07f, -0.07f, 0.14f, 0.14f));
					painter.drawPolyline (_vor_shape);
				}
				else if (navaid.vor_type() == Navaid::VOR_DME)
				{
					painter.drawEllipse (QRectF (-0.07f, -0.07f, 0.14f, 0.14f));
					painter.drawPolyline (_vor_shape);
					painter.drawPolyline (_dme_for_vor_shape);
				}
				else if (navaid.vor_type() == Navaid::VORTAC)
					painter.drawPolyline (_vortac_shape);
				painter.setTransform (feature_centered_transform);
				painter.fast_draw_text (QPointF (0.35f * _q, 0.55f * _q), navaid.identifier());
				break;

			case Navaid::DME:
				painter.setTransform (feature_scaled_transform);
				painter.setPen (_dme_pen);
				painter.drawRect (QRectF (-0.5f, -0.5f, 1.f, 1.f));
				break;

			case Navaid::Fix:
			{
				float const h = 0.75f;
				QPointF a (0.f, -0.66f * h);
				QPointF b (+0.5f * h, +0.33f * h);
				QPointF c (-0.5f * h, +0.33f * h);
				QPointF points[] = { a, b, c, a };
				painter.setTransform (feature_scaled_transform);
				painter.setPen (_fix_pen);
				painter.drawPolyline (points, sizeof (points) / sizeof (*points));
				painter.setTransform (feature_centered_transform);
				painter.translate (0.5f, 0.5f);
				painter.fast_draw_text (QPointF (0.25f * _q, 0.45f * _q), navaid.identifier());
				break;
			}

			default:
				break;
		}
	};

	if (_params.fix_visible)
		for (auto& navaid: _fix_navs)
			paint_navaid (navaid);

	if (_params.ndb_visible)
		for (auto& navaid: _ndb_navs)
			paint_navaid (navaid);

	if (_params.dme_visible)
		for (auto& navaid: _dme_navs)
			paint_navaid (navaid);

	if (_params.vor_visible)
		for (auto& navaid: _vor_navs)
			paint_navaid (navaid);

	if (_params.home)
	{
		// Whether the feature is in configured HSI range:
		bool outside_range = false;
		QPointF translation = position_feature (*_params.home, &outside_range);
		QTransform feature_centered_transform = _aircraft_center_transform;
		feature_centered_transform.translate (translation.x(), translation.y());

		// Line from aircraft to the HOME feature:
		if (_params.home_track_visible)
		{
			float green_pen_width = 1.5f;
			float shadow_pen_width = 2.5f;
			if (_params.display_mode == DisplayMode::Auxiliary)
			{
				green_pen_width = 1.2f;
				shadow_pen_width = 2.2f;
			}

			float const shadow_scale = shadow_pen_width / green_pen_width;

			QPen home_line_pen (_home_pen.color(), pen_width (green_pen_width), Qt::DashLine, Qt::RoundCap);
			home_line_pen.setDashPattern (QVector<qreal>() << 7.5 << 12);

			QPen shadow_pen (painter.shadow_color(), pen_width (shadow_pen_width), Qt::DashLine, Qt::RoundCap);
			shadow_pen.setDashPattern (QVector<qreal>() << 7.5 / shadow_scale << 12 / shadow_scale);

			painter.setTransform (_aircraft_center_transform);

			for (auto const& p: { shadow_pen, home_line_pen })
			{
				painter.setPen (p);
				painter.drawLine (QPointF (0.f, 0.f), translation);
			}
		}

		painter.setTransform (feature_centered_transform);
		painter.scale (scale, scale);

		if (outside_range)
		{
			painter.setPen (_home_pen);
			painter.setBrush (Qt::black);
			painter.drawPolygon (_home_shape);
		}
		else
		{
			painter.setPen (_home_pen);
			painter.setBrush (_home_pen.color());
			painter.drawPolygon (_home_shape);
		}
	}
}


void
HSIWidget::PaintWorkUnit::paint_locs (Xefis::Painter& painter)
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
			painter.fast_draw_text (text_and_xy.first, text_and_xy.second);
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
		if (_params.range < 16_nm)
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
		if (navaid.identifier() == _params.highlighted_loc)
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
HSIWidget::PaintWorkUnit::paint_tcas()
{
	if (!_params.tcas_on)
		return;

	if (!*_params.tcas_on)
	{
		paint_bottom_text (true, 3, _warning_color_2, "TCAS");
		paint_bottom_text (true, 2, _warning_color_2, "OFF ");
	}

	painter().setTransform (_aircraft_center_transform);
	painter().setClipping (false);
	painter().setPen (get_pen (Qt::white, 1.f));

	if (_params.tcas_range)
	{
		double z = 0.075 * _q;
		double v = 0.025 * _q;
		double r = nm_to_px (*_params.tcas_range);

		// Don't draw too small range points:
		if (r > 15.0)
		{
			QRectF big_point (-z, -z, 2.0 * z, 2.0 * z);
			QRectF small_point (-v, -v, 2.0 * v, 2.0 * v);

			for (int angle = 0; angle < 360; angle += 30)
			{
				painter().translate (0.0, r);

				if (angle % 90 == 0)
				{
					painter().setBrush (Qt::NoBrush);
					painter().add_shadow ([&]() {
						painter().drawEllipse (big_point);
					});
				}
				else
				{
					painter().setBrush (Qt::white);
					painter().add_shadow ([&]() {
						painter().drawEllipse (small_point);
					});
				}

				painter().translate (0.0, -r);
				painter().rotate (30);
			}
		}
	}
}


void
HSIWidget::PaintWorkUnit::paint_bottom_text (bool left, unsigned int line_from_bottom, QColor color, QString text)
{
	painter().resetTransform();
	painter().setClipping (false);
	painter().setFont (_font_16);
	// Positioning:
	double z = 0.2f * _q;
	double h_corr = 0.8;
	Qt::Alignment alignment = Qt::AlignVCenter | (left ? Qt::AlignLeft : Qt::AlignRight);
	QFontMetricsF metrics (painter().font());
	QPointF text_hook = { (left ? z : _w - z), _h - z - line_from_bottom * h_corr * metrics.height() - 0.5 * metrics.height() };
	QRectF text_box = painter().get_text_box (text_hook, alignment, text);
	// Paint:
	painter().setPen (Qt::NoPen);
	painter().setBrush (Qt::black);
	painter().drawRect (text_box);
	painter().setPen (get_pen (color, 1.f));
	painter().fast_draw_text (text_hook, alignment, text);
}


void
HSIWidget::PaintWorkUnit::retrieve_navaids()
{
	if (!_navaid_storage || !_params.position)
		return;

	if (_navs_retrieved && _navs_retrieve_position.haversine_earth (*_params.position) < 0.1f * _params.range && _params.range == _navs_retrieve_range)
		return;

	_loc_navs.clear();
	_ndb_navs.clear();
	_vor_navs.clear();
	_dme_navs.clear();
	_fix_navs.clear();

	for (Navaid const& navaid: _navaid_storage->get_navs (*_params.position, std::max (_params.range + 20_nm, 2.f * _params.range)))
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
	_navs_retrieve_position = *_params.position;
	_navs_retrieve_range = _params.range;
}


HSIWidget::HSIWidget (QWidget* parent, Xefis::WorkPerformer* work_performer):
	InstrumentWidget (parent, work_performer),
	_local_paint_work_unit (this)
{
	set_painter (&_local_paint_work_unit);
}


HSIWidget::~HSIWidget()
{
	wait_for_painter();
}


void
HSIWidget::set_params (Parameters const& new_params)
{
	_params = new_params;
	_params.sanitize();
	request_repaint();
}

void
HSIWidget::resizeEvent (QResizeEvent* event)
{
	InstrumentWidget::resizeEvent (event);

	auto xw = dynamic_cast<Xefis::Window*> (window());
	if (xw)
		_local_paint_work_unit.set_scaling (xw->pen_scale(), xw->font_scale());
}


void
HSIWidget::push_params()
{
	QDateTime now = QDateTime::currentDateTime();

	Parameters old = _local_paint_work_unit._params_next;

	if (_params.display_mode != old.display_mode)
		_local_paint_work_unit._recalculation_needed = true;

	if (_params.positioning_hint != old.positioning_hint)
		_locals.positioning_hint_ts = now;

	if (_params.positioning_hint_visible != old.positioning_hint_visible)
		_locals.positioning_hint_ts = now;

	_local_paint_work_unit._params_next = _params;
	_local_paint_work_unit._locals_next = _locals;
}


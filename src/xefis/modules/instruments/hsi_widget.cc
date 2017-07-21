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
#include <utility>
#include <vector>
#include <cmath>

// Lib:
#include <boost/format.hpp>

// Qt:
#include <QtCore/QTimer>
#include <QtGui/QPainter>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v1/window.h>
#include <xefis/support/air/wind_triangle.h>
#include <xefis/support/navigation/earth.h>
#include <xefis/support/navigation/navaid.h>
#include <xefis/support/navigation/navaid_storage.h>
#include <xefis/utility/numeric.h>
#include <xefis/utility/painter.h>
#include <xefis/utility/text_layout.h>

// Local:
#include "hsi_widget.h"


using xf::Navaid;
using xf::NavaidStorage;


void
HSIWidget::Parameters::sanitize()
{
	using xf::clamped;
	using xf::floored_mod;

	range = clamped<Length> (range, 1_ft, 5000_nmi);
	heading_magnetic = floored_mod (heading_magnetic, 360_deg);
	heading_true = floored_mod (heading_true, 360_deg);
	if (ap_heading_magnetic)
		ap_heading_magnetic = floored_mod (*ap_heading_magnetic, 360_deg);
	if (ap_track_magnetic)
		ap_track_magnetic = floored_mod (*ap_track_magnetic, 360_deg);
	track_magnetic = floored_mod (track_magnetic, 360_deg);
	if (true_home_direction)
		true_home_direction = floored_mod (*true_home_direction, 360_deg);
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
			float const rx = to_px (_params.range);

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
			float const rx = to_px (_params.range);

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
			float const rx = to_px (_params.range);

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
	_ndb_pen = QPen (QColor (99, 99, 99), 0.09f, Qt::SolidLine, Qt::RoundCap, Qt::BevelJoin);
	_vor_pen = QPen (Qt::green, 0.09f, Qt::SolidLine, Qt::RoundCap, Qt::BevelJoin);
	_dme_pen = QPen (Qt::green, 0.09f, Qt::SolidLine, Qt::RoundCap, Qt::BevelJoin);
	_fix_pen = QPen (QColor (0, 132, 255), 0.1f, Qt::SolidLine, Qt::RoundCap, Qt::BevelJoin);
	_arpt_pen = QPen (Qt::white, 0.1f, Qt::SolidLine, Qt::RoundCap, Qt::BevelJoin);
	_home_pen = QPen (Qt::green, 0.1f, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin);

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

	_margin = 0.15 * _q;
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

	_locals.track_true = xf::floored_mod (_params.track_magnetic + (_params.heading_true - _params.heading_magnetic), 360_deg);

	_locals.track =
		_params.heading_mode == HeadingMode::Magnetic
			? _params.track_magnetic
			: _locals.track_true;

	_locals.heading = _params.heading_mode == HeadingMode::Magnetic
		? _params.heading_magnetic
		: _params.heading_true;

	_locals.rotation = _params.center_on_track ? _locals.track : _locals.heading;

	_heading_transform.reset();
	_heading_transform.rotate (-_locals.heading.quantity<Degree>());

	_track_transform.reset();
	_track_transform.rotate (-_locals.track.quantity<Degree>());

	_rotation_transform =
		_params.center_on_track
			? _track_transform
			: _heading_transform;

	_features_transform = _rotation_transform;
	if (_params.heading_mode == HeadingMode::Magnetic)
		_features_transform.rotate ((_params.heading_magnetic - _params.heading_true).quantity<Degree>());

	_pointers_transform = _rotation_transform;
	if (_params.heading_mode == HeadingMode::True)
		_pointers_transform.rotate ((_params.heading_true - _params.heading_magnetic).quantity<Degree>());

	_locals.ap_use_trk = _params.ap_use_trk;
	// If use_trk is not nil, use proper heading or track information to position cmd bug.
	if (_locals.ap_use_trk)
	{
		if (*_locals.ap_use_trk)
			_locals.ap_bug_magnetic = _params.ap_track_magnetic;
		else
			_locals.ap_bug_magnetic = _params.ap_heading_magnetic;
	}
	// If use_trk is unavailable (nil), then use the only heading/magnetic property
	// that is set. If both or neither is set, fail.
	else
	{
		if (!_params.ap_heading_magnetic != !_params.ap_track_magnetic)
		{
			if (_params.ap_heading_magnetic)
			{
				_locals.ap_bug_magnetic = _params.ap_heading_magnetic;
				_locals.ap_use_trk = false;
			}
			else
			{
				_locals.ap_bug_magnetic = _params.ap_track_magnetic;
				_locals.ap_use_trk = true;
			}
		}
		else
		{
			_locals.ap_bug_magnetic.reset();
			_locals.ap_use_trk.reset();
		}
	}

	// Finish up cmd bug setting:
	if (_locals.ap_bug_magnetic)
	{
		if (_params.heading_mode == HeadingMode::True)
			*_locals.ap_bug_magnetic += _params.heading_true - _params.heading_magnetic;
		_locals.ap_bug_magnetic = xf::floored_mod (*_locals.ap_bug_magnetic, 360_deg);
	}

	if (_params.course_setting_magnetic)
	{
		_locals.course_heading = *_params.course_setting_magnetic;
		if (_params.heading_mode == HeadingMode::True)
			_locals.course_heading += _params.heading_true - _params.heading_magnetic;
		_locals.course_heading = xf::floored_mod (_locals.course_heading, 360_deg);
	}

	_locals.navaid_selected_visible = !_params.navaid_selected_reference.isEmpty() || !_params.navaid_selected_identifier.isEmpty() ||
									  _params.navaid_selected_distance || _params.navaid_selected_eta;

	_locals.navaid_left_visible = !_params.navaid_left_reference.isEmpty() || !_params.navaid_left_identifier.isEmpty() ||
								  _params.navaid_left_distance || _params.navaid_left_initial_bearing_magnetic;

	_locals.navaid_right_visible = !_params.navaid_right_reference.isEmpty() || !_params.navaid_right_identifier.isEmpty() ||
								  _params.navaid_right_distance || _params.navaid_right_initial_bearing_magnetic;

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
	paint_range (painter());
	paint_hints (painter());
	paint_trend_vector (painter());
	paint_tcas();
	paint_course (painter());
	paint_selected_navaid_info();
	paint_tcas_and_navaid_info();
	paint_pointers (painter());
	paint_aircraft (painter());
	paint_navperf (painter());
}


void
HSIWidget::PaintWorkUnit::paint_aircraft (xf::Painter& painter)
{
	painter.setTransform (_aircraft_center_transform);
	painter.setClipping (false);

	// Aircraft triangle - shadow and triangle:
	painter.setPen (get_pen (Qt::white, 1.f));
	painter.add_shadow ([&] {
		painter.drawPolyline (_aircraft_shape);
	});

	painter.resetTransform();
	painter.setClipping (false);

	// AP info: SEL HDG/TRK 000
	if (_params.display_mode == DisplayMode::Auxiliary && _locals.ap_bug_magnetic && _locals.ap_use_trk)
	{
		int sel_hdg = static_cast<int> (_locals.ap_bug_magnetic->quantity<Degree>() + 0.5f) % 360;
		if (sel_hdg == 0)
			sel_hdg = 360;

		QString str = *_locals.ap_use_trk ? "SEL TRK " : "SEL HDG ";
		// AP heading always set as magnetic, but can be displayed as true:
		xf::TextLayout layout;
		layout.set_background (Qt::black, { _margin, 0.0 });
		layout.add_fragment (str, _font_13, _autopilot_pen_2.color());
		layout.add_fragment (QString ("%1").arg (sel_hdg, 3, 10, QChar ('0')), _font_16, _autopilot_pen_2.color());
		layout.paint (QPointF (0.5 * _w - _q, _h - 0.1 * layout.height()), Qt::AlignBottom | Qt::AlignRight, painter);
	}

	// MAG/TRUE heading
	if (_params.heading_visible)
	{
		int hdg = static_cast<int> ((_params.center_on_track ? _locals.track : _locals.heading).quantity<Degree>() + 0.5f) % 360;
		if (hdg == 0)
			hdg = 360;

		switch (_params.display_mode)
		{
			case DisplayMode::Auxiliary:
			{
				QString text_1 =
					QString (_params.heading_mode == HeadingMode::Magnetic ? "MAG" : "TRU") +
					QString (_params.center_on_track ? " TRK" : "");
				QPen box_pen = Qt::NoPen;
				// True heading is boxed for emphasis:
				if (_params.heading_mode == HeadingMode::True)
					box_pen = get_pen (_navigation_color, 1.0);

				xf::TextLayout layout;
				layout.set_background (Qt::black, { _margin, 0.0 });
				layout.add_fragment (text_1 + " ", _font_13, _navigation_color);
				layout.add_fragment (QString ("%1").arg (hdg, 3, 10, QChar ('0')), _font_16, _navigation_color, box_pen);
				layout.paint (QPointF (0.5 * _w + _q, _h - 0.1 * layout.height()), Qt::AlignBottom | Qt::AlignLeft, painter);
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
HSIWidget::PaintWorkUnit::paint_navperf (xf::Painter& painter)
{
	if (_params.display_mode != DisplayMode::Auxiliary)
	{
		if (_params.navigation_required_performance ||
			_params.navigation_actual_performance)
		{
			double x = 0.045 * _w;

			if (_params.navigation_required_performance)
			{
				painter.resetTransform();
				painter.setClipping (false);
				painter.translate (0.5 * _w, _h);

				auto val = QString ("%1").arg (_params.navigation_required_performance->quantity<Meter>(), 0, 'f', 2);

				xf::TextLayout layout;
				layout.set_background (Qt::black, { _margin, 0.0 });
				layout.set_alignment (Qt::AlignHCenter);
				layout.add_fragment ("RNP", _font_13, _navigation_color);
				layout.add_new_line();
				layout.add_fragment (val, _font_13, _navigation_color);
				layout.paint (QPointF (-x, 0.0), Qt::AlignBottom | Qt::AlignHCenter, painter);
			}

			if (_params.navigation_actual_performance)
			{
				painter.resetTransform();
				painter.setClipping (false);
				painter.translate (0.5 * _w, _h);

				auto val = QString ("%1").arg (_params.navigation_actual_performance->quantity<Meter>(), 0, 'f', 2);

				QColor text_color = _navigation_color;
				if (_params.navigation_required_performance)
					if (*_params.navigation_required_performance < *_params.navigation_actual_performance)
						text_color = _warning_color_1;

				xf::TextLayout layout;
				layout.set_background (Qt::black, { _margin, 0.0 });
				layout.set_alignment (Qt::AlignHCenter);
				layout.add_fragment ("ANP", _font_13, text_color);
				layout.add_new_line();
				layout.add_fragment (val, _font_13, text_color);
				layout.paint (QPointF (+x, 0.0), Qt::AlignBottom | Qt::AlignHCenter, painter);
			}
		}
	}
}


void
HSIWidget::PaintWorkUnit::paint_hints (xf::Painter& painter)
{
	if (!_params.positioning_hint_visible || !_params.position)
		return;

	painter.resetTransform();
	painter.setClipping (false);

	double x = _params.display_mode == DisplayMode::Auxiliary ? 0.775f * _w : 0.725f * _w;
	QString hint = _params.positioning_hint;

	// Box for emphasis:
	QPen box_pen = Qt::NoPen;
	if (is_newly_set (_locals.positioning_hint_ts))
	{
		if (hint == "")
			hint = "---";
		box_pen = get_pen (_navigation_color, 1.0);
	}

	xf::TextLayout layout;
	layout.set_background (Qt::black, { _margin, 0.0 });
	layout.add_fragment (hint, _font_13, _navigation_color, box_pen);
	layout.paint (QPointF (x, _h), Qt::AlignBottom | Qt::AlignHCenter, painter);
}


void
HSIWidget::PaintWorkUnit::paint_track (xf::Painter& painter, bool paint_heading_triangle)
{
	Length trend_range = actual_trend_range();

	float start_point = _params.track_lateral_rotation ? -to_px (trend_range) - 0.25f * _q : 0.f;

	painter.setTransform (_aircraft_center_transform);
	painter.setClipping (false);

	QFont font = _font_13;
	QFontMetricsF metrics (font);

	if (!paint_heading_triangle && _params.track_visible)
	{
		// Scale and track line:
		painter.setPen (QPen (_silver, pen_width (1.3f), Qt::SolidLine, Qt::RoundCap));
		painter.rotate ((_locals.track - _locals.rotation).quantity<Degree>());
		float extension = 0.0;
		if (_params.display_mode != DisplayMode::Auxiliary && _params.center_on_track)
			extension = 0.6 * _q;
		painter.draw_outlined_line (QPointF (0.f, start_point), QPointF (0.f, -_r - extension));
		painter.setPen (QPen (Qt::white, pen_width (1.3f), Qt::SolidLine, Qt::RoundCap));
	}

	if (!paint_heading_triangle)
	{
		// Scale ticks:
		auto paint_range_tick = [&] (float ratio, bool draw_text) -> void
		{
			Length range;
			if (ratio == 0.5 && _params.range >= 2_nmi)
				range = 1_nmi * std::round (((10.f * ratio * _params.range) / 10.f).quantity<NauticalMile>());
			else
				range = ratio * _params.range;
			float range_tick_vpx = to_px (range);
			float range_tick_hpx = 0.1f * _q;
			int precision = 0;
			if (range < 1_nmi)
				precision = 1;
			QString half_range_str = QString ("%1").arg (range.quantity<NauticalMile>(), 0, 'f', precision);
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
		painter.rotate ((_locals.heading - _locals.rotation).quantity<Degree>());

		painter.setPen (get_pen (Qt::white, 2.2f));
		painter.translate (0.f, -1.003f * _r);
		painter.scale (0.465f, -0.465f);
		painter.add_shadow ([&] {
			painter.drawPolyline (_aircraft_shape);
		});
	}
}


void
HSIWidget::PaintWorkUnit::paint_altitude_reach (xf::Painter& painter)
{
	if (!_params.altitude_reach_visible || (_params.altitude_reach_distance < 0.005f * _params.range) || (0.8f * _params.range < _params.altitude_reach_distance))
		return;

	float len = xf::clamped (to_px (6_nmi), 2.f * _q, 7.f * _q);
	float pos = to_px (_params.altitude_reach_distance);
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
HSIWidget::PaintWorkUnit::paint_trend_vector (xf::Painter& painter)
{
	using std::abs;

	QPen est_pen = QPen (Qt::white, pen_width (1.f), Qt::SolidLine, Qt::RoundCap);

	painter.setTransform (_aircraft_center_transform);
	painter.setClipPath (_inner_map_clip);
	painter.setPen (est_pen);

	if (_params.track_lateral_rotation && _params.ground_speed &&
		2.f * trend_time_gap() < _params.trend_vector_times[2] && _params.range <= _params.trend_vector_max_range)
	{
		painter.setPen (est_pen);
		painter.setTransform (_aircraft_center_transform);
		painter.setClipRect (_trend_vector_clip_rect);

		Time step = *std::min_element (_params.trend_vector_times.begin(), _params.trend_vector_times.end()) / 100.0;
		Angle const angle_per_step = step * *_params.track_lateral_rotation;
		Angle total_angle = 0_deg;

		QTransform transform;
		QPolygonF polygon;

		// Initially rotate the transform to match HDG or TRK setting:
		transform.rotate ((_locals.track - _locals.rotation).quantity<Degree>());

		// Take wind into consideration if track info is available:
		Optional<xf::WindTriangle> wt;
		if (_params.true_air_speed && _params.heading_visible && _params.track_visible)
		{
			wt = xf::WindTriangle();
			wt->set_air_vector (*_params.true_air_speed, _params.heading_magnetic);
			wt->set_ground_vector (*_params.ground_speed, _params.track_magnetic);
			wt->compute_wind_vector();
		}

		for (Time t = 0_s; t < _params.trend_vector_times[2]; t += step)
		{
			transform.rotate (angle_per_step.quantity<Degree>());
			total_angle += angle_per_step;

			Speed ground_speed = wt
				? wt->get_ground_speed (_locals.track + total_angle)
				: *_params.ground_speed;

			float px = to_px (ground_speed * step);

			// If the turn is too tight, stop drawing trend vectors:
			if (abs (total_angle) >= 180_deg)
			{
				polygon.clear();
				break;
			}

			if ((_params.trend_vector_min_ranges[0] <= _params.range && trend_time_gap() <= t && t < _params.trend_vector_times[0]) ||
				(_params.trend_vector_min_ranges[1] <= _params.range && trend_time_gap() + _params.trend_vector_times[0] <= t && t < _params.trend_vector_times[1]) ||
				(_params.trend_vector_min_ranges[2] <= _params.range && trend_time_gap() + _params.trend_vector_times[1] <= t && t < _params.trend_vector_times[2]))
			{
				polygon << transform.map (QPointF (0.f, -px));
			}
			else if (!polygon.empty())
			{
				painter.add_shadow ([&] {
					painter.drawPolyline (polygon);
				});
				polygon.clear();
			}

			transform.translate (0.f, -px);
		}

		if (!polygon.empty())
		{
			painter.add_shadow ([&] {
				painter.drawPolyline (polygon);
			});
		}
	}
}


void
HSIWidget::PaintWorkUnit::paint_ap_settings (xf::Painter& painter)
{
	if (!_params.ap_visible)
		return;

	// AP dashed line:
	if (_params.ap_line_visible && _locals.ap_bug_magnetic)
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
		painter.rotate ((*_locals.ap_bug_magnetic - _locals.rotation).quantity<Degree>());

		for (auto const& p: { shadow_pen, pen })
		{
			painter.setPen (p);
			painter.drawLine (QPointF (0.f, 0.f), QPointF (0.f, -_r));
		}
	}

	// A/P bug
	if (_params.heading_visible && _locals.ap_bug_magnetic)
	{
		Angle limited_rotation;
		switch (_params.display_mode)
		{
			case DisplayMode::Auxiliary:
				limited_rotation = xf::floored_mod (*_locals.ap_bug_magnetic - _locals.rotation + 180_deg, 360_deg) - 180_deg;
				break;

			default:
				limited_rotation = *_locals.ap_bug_magnetic - _locals.rotation;
				break;
		}

		QTransform transform = _aircraft_center_transform;
		transform.rotate (limited_rotation.quantity<Degree>());
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
HSIWidget::PaintWorkUnit::paint_directions (xf::Painter& painter)
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

	painter.add_shadow ([&] {
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
HSIWidget::PaintWorkUnit::paint_speeds_and_wind (xf::Painter& painter)
{
	QFont font_a = _font_13;
	QFont font_b = _font_18;
	QFontMetricsF metr_a (font_a);
	QFontMetricsF metr_b (font_b);

	xf::TextLayout layout;
	layout.set_alignment (Qt::AlignLeft);

	// GS
	layout.add_fragment ("GS", _font_13, Qt::white);
	QString gs_str = "---";
	if (_params.ground_speed)
		gs_str = QString::number (static_cast<int> (_params.ground_speed->quantity<Knot>()));
	layout.add_fragment (gs_str, _font_18, Qt::white);

	layout.add_fragment (" ", _font_13, Qt::white);

	// TAS
	layout.add_fragment ("TAS", _font_13, Qt::white);
	QString tas_str = "---";
	if (_params.true_air_speed)
		tas_str = QString::number (static_cast<int> (_params.true_air_speed->quantity<Knot>()));
	layout.add_fragment (tas_str, _font_18, Qt::white);

	// Wind data (direction/strength):
	if (_params.wind_information_visible)
	{
		QString wind_str = QString ("%1°/%2")
			.arg (static_cast<long> (_params.wind_from_magnetic_heading.quantity<Degree>()), 3, 10, QChar ('0'))
			.arg (static_cast<long> (_params.wind_tas_speed.quantity<Knot>()), 3, 10, QChar (L'\u2007'));
		layout.add_new_line();
		layout.add_fragment (wind_str, _font_16, Qt::white);
	}

	painter.resetTransform();
	painter.setClipping (false);
	layout.paint (_rect.topLeft() + QPointF (_margin, 0.0), Qt::AlignTop | Qt::AlignLeft, painter);

	// Wind arrow:
	if (_params.wind_information_visible)
	{
		painter.setPen (get_pen (Qt::white, 0.6f));
		painter.translate (0.8f * _q + _margin, 0.8f * _q + layout.height());
		painter.rotate ((_params.wind_from_magnetic_heading - _params.heading_magnetic + 180_deg).quantity<Degree>());
		painter.setPen (get_pen (Qt::white, 1.0));
		painter.add_shadow ([&] {
			QPointF a = QPointF (0.f, -0.7f * _q);
			QPointF b = QPointF (0.f, +0.7f * _q);
			painter.drawLine (a + QPointF (0.f, 0.05f * _q), b);
			painter.drawLine (a, a + QPointF (+_margin, +_margin));
			painter.drawLine (a, a + QPointF (-_margin, +_margin));
		});
	}
}


void
HSIWidget::PaintWorkUnit::paint_home_direction (xf::Painter& painter)
{
	if (_params.display_mode != DisplayMode::Auxiliary)
		return;

	if (!_params.position || !_params.home)
		return;

	QTransform base_transform;
	base_transform.translate (_w - _margin, 0.55f * _h);

	painter.resetTransform();
	painter.setClipping (false);

	// Home direction arrow:
	if (_params.true_home_direction)
	{
		bool at_home = xf::haversine_earth (*_params.home, *_params.position) < 10_m;
		float z = 0.75f * _q;

		painter.setTransform (base_transform);
		painter.translate (-z - 0.1f * _q, _q);
		if (at_home)
		{
			painter.setPen (get_pen (Qt::white, 1.25));
			float v = 0.35f * z;
			painter.setBrush (Qt::black);
			painter.drawEllipse (QRectF (-v, -v, 2.f * v, 2.f * v));
		}
		else
		{
			painter.setPen (get_pen (Qt::white, 1.0));
			QPolygonF home_arrow = QPolygonF()
				<< QPointF (0.0, z)
				<< QPointF (0.0, -0.8 * z)
				<< QPointF (-0.2 * z, -0.8 * z)
				<< QPointF (0.0, -z)
				<< QPointF (+0.2 * z, -0.8 * z)
				<< QPointF (0.0, -0.8 * z);
			painter.rotate ((*_params.true_home_direction - _params.heading_true).quantity<Degree>());
			painter.add_shadow ([&] {
				painter.drawPolyline (home_arrow);
			});
		}
	}

	// Height/VLOS distance/ground distance:
	if (_params.dist_to_home_ground_visible || _params.dist_to_home_vlos_visible || _params.dist_to_home_vert_visible)
	{
		float z = 0.99f * _q;
		float h = 0.66f * _q;
		QPolygonF distance_triangle = QPolygonF()
			<< QPointF (z, 0.f)
			<< QPointF (0.f, 0.f)
			<< QPointF (z, -h);

		xf::TextLayout layout;
		layout.set_background (Qt::black, { _margin, 0.0 });
		layout.set_alignment (Qt::AlignRight);

		std::string vert_str = "---";
		if (_params.dist_to_home_vert_visible)
			vert_str = (boost::format ("%+d") % static_cast<int> (_params.dist_to_home_vert.quantity<Foot>())).str();
		layout.add_fragment ("↑", _font_16, Qt::gray);
		layout.add_fragment (vert_str, _font_16, Qt::white);
		layout.add_fragment ("FT", _font_13, Qt::white);
		layout.add_new_line();

		QString vlos_str = "---";
		if (_params.dist_to_home_vlos_visible)
			vlos_str = QString ("%1").arg (_params.dist_to_home_vlos.quantity<NauticalMile>(), 0, 'f', 2, QChar ('0'));
		layout.add_fragment ("VLOS ", _font_13, Qt::white);
		layout.add_fragment (vlos_str, _font_16, Qt::white);
		layout.add_fragment ("NM", _font_13, Qt::white);
		layout.add_new_line();

		QString ground_str = "---";
		if (_params.dist_to_home_ground_visible)
			ground_str = QString ("%1").arg (_params.dist_to_home_ground.quantity<NauticalMile>(), 0, 'f', 2, QChar ('0'));
		layout.add_fragment (ground_str, _font_16, Qt::white);
		layout.add_fragment ("NM", _font_13, Qt::white);

		painter.setTransform (base_transform);
		layout.paint (QPointF (0.0, 0.0), Qt::AlignRight | Qt::AlignBottom, painter);
	}
}


void
HSIWidget::PaintWorkUnit::paint_course (xf::Painter& painter)
{
	using std::abs;

	if (!_params.heading_visible || !_params.course_setting_magnetic || !_params.course_visible)
		return;

	painter.setTransform (_aircraft_center_transform);
	painter.setClipPath (_outer_map_clip);
	painter.setTransform (_aircraft_center_transform);
	painter.rotate ((_locals.course_heading - _locals.rotation).quantity<Degree>());

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
	painter.add_shadow ([&] {
		painter.drawPolyline (top_bar);
		painter.drawPolyline (bottom_bar);
	});

	// Deviation bar:
	if (_params.course_deviation)
	{
		bool filled = false;
		Angle deviation = xf::clamped<Angle> (*_params.course_deviation, -2.5_deg, +2.5_deg);
		if (abs (*_params.course_deviation) <= abs (deviation))
			filled = true;

		double pw = pen_width (1.75f);
		QRectF bar (-z, -2.5f * k + pw, 2.f * z, 5.f * k - 2.f * pw);
		bar.translate (dev_1_deg_px * deviation.quantity<Degree>(), 0.f);

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
	painter.add_shadow ([&] {
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
HSIWidget::PaintWorkUnit::paint_selected_navaid_info()
{
	if (!_locals.navaid_selected_visible)
		return;

	painter().resetTransform();
	painter().setClipping (false);

	std::string course_str = "/---°";
	if (_params.navaid_selected_course_magnetic)
	{
		int course_int = xf::symmetric_round (_params.navaid_selected_course_magnetic->quantity<Degree>());
		if (course_int == 0)
			course_int = 360;
		course_str = (boost::format ("/%03d°") % course_int).str();
	}

	std::string eta_min = "--";
	std::string eta_sec = "--";
	if (_params.navaid_selected_eta)
	{
		int s_int = _params.navaid_selected_eta->quantity<Second>();
		eta_min = (boost::format ("%02d") % (s_int / 60)).str();
		eta_sec = (boost::format ("%02d") % (s_int % 60)).str();
	}

	std::string distance_str = "---";
	if (_params.navaid_selected_distance)
		distance_str = (boost::format ("%3.1f") % _params.navaid_selected_distance->quantity<NauticalMile>()).str();

	xf::TextLayout layout;
	layout.set_background (Qt::black, { _margin, 0.0 });
	layout.set_background_mode (xf::TextLayout::PerLine);
	layout.set_alignment (Qt::AlignRight);
	// If reference name is not empty, format is:
	//   <reference:green> <identifier>/<course>°
	// Otherwise:
	//   <identifier:magenta>/<course>°
	if (!_params.navaid_selected_reference.isEmpty())
	{
		layout.add_fragment (_params.navaid_selected_reference, _font_18, Qt::green);
		layout.add_fragment (" ", _font_10, Qt::white);
		layout.add_fragment (_params.navaid_selected_identifier, _font_18, Qt::white);
	}
	else
		layout.add_fragment (_params.navaid_selected_identifier, _font_18, _autopilot_pen_2.color());
	layout.add_fragment (course_str, _font_13, Qt::white);
	layout.add_new_line();
	layout.add_fragment ("ETA ", _font_13, Qt::white);
	layout.add_fragment (eta_min, _font_18, Qt::white);
	layout.add_fragment ("M", _font_13, Qt::white);
	layout.add_fragment (eta_sec, _font_18, Qt::white);
	layout.add_fragment ("S", _font_13, Qt::white);
	layout.add_new_line();
	layout.add_fragment (distance_str, _font_18, Qt::white);
	layout.add_fragment ("NM", _font_13, Qt::white);
	layout.paint (_rect.topRight() - QPointF (_margin, 0.0), Qt::AlignTop | Qt::AlignRight, painter());
}


void
HSIWidget::PaintWorkUnit::paint_tcas_and_navaid_info()
{
	painter().resetTransform();
	painter().setClipping (false);

	auto configure_layout = [&](xf::TextLayout& layout, QColor const& color, QString const& reference, QString const& identifier, Optional<Length> const& distance) -> void
	{
		if (!reference.isEmpty())
			layout.add_fragment (reference, _font_16, color);
		layout.add_skips (_font_16, 1);
		layout.add_fragment (identifier.isEmpty() ? "---" : identifier, _font_16, color);
		layout.add_new_line();
		layout.add_fragment ("DME ", _font_13, color);
		layout.add_fragment (distance ? (boost::format ("%.1f") % distance->quantity<NauticalMile>()).str() : std::string ("---"), _font_16, color);
	};

	xf::TextLayout left_layout;
	left_layout.set_alignment (Qt::AlignLeft);
	left_layout.set_background (Qt::black, { _margin, 0.0 });

	if (_params.loc_visible)
		left_layout.add_fragment ("LOC", _font_13, _cyan);
	left_layout.add_skips (_font_13, 1);

	if (_params.arpt_visible)
		left_layout.add_fragment ("ARPT", _font_13, _cyan);
	left_layout.add_skips (_font_13, 1);

	if (_params.fix_visible)
		left_layout.add_fragment ("WPT", _font_13, _cyan);
	left_layout.add_skips (_font_13, 1);

	if (_params.vor_visible || _params.dme_visible || _params.ndb_visible)
		left_layout.add_fragment ("STA", _font_13, _cyan);
	left_layout.add_skips (_font_13, 2);

	if (_params.tcas_on && !*_params.tcas_on)
	{
		left_layout.add_fragment ("TCAS", _font_16, _warning_color_2);
		left_layout.add_new_line();
		left_layout.add_fragment ("OFF", _font_16, _warning_color_2);
		left_layout.add_new_line();
	}
	else
		left_layout.add_skips (_font_16, 2);

	if (_locals.navaid_left_visible)
		configure_layout (left_layout, (_params.navaid_left_type == 0) ? Qt::green : _cyan, _params.navaid_left_reference, _params.navaid_left_identifier, _params.navaid_left_distance);
	else
		left_layout.add_skips (_font_16, 2);

	xf::TextLayout right_layout;
	right_layout.set_alignment (Qt::AlignRight);
	right_layout.set_background (Qt::black, { _margin, 0.0 });

	if (_locals.navaid_right_visible)
		configure_layout (right_layout, (_params.navaid_right_type == 0) ? Qt::green : _cyan, _params.navaid_right_reference, _params.navaid_right_identifier, _params.navaid_right_distance);

	left_layout.paint (_rect.bottomLeft() + QPointF (_margin, 0.0), Qt::AlignBottom | Qt::AlignLeft, painter());
	right_layout.paint (_rect.bottomRight() - QPointF (_margin, 0.0), Qt::AlignBottom | Qt::AlignRight, painter());
}


void
HSIWidget::PaintWorkUnit::paint_pointers (xf::Painter& painter)
{
	if (!_params.heading_visible)
		return;

	painter.resetTransform();
	painter.setClipping (false);

	struct Opts
	{
		bool			is_primary;
		QColor			color;
		Optional<Angle>	angle;
		bool			visible;
	};

	for (Opts const& opts: { Opts { true, (_params.navaid_left_type == 0 ? Qt::green : _cyan), _params.navaid_left_initial_bearing_magnetic, _locals.navaid_left_visible },
							 Opts { false, (_params.navaid_right_type == 0 ? Qt::green : _cyan), _params.navaid_right_initial_bearing_magnetic, _locals.navaid_right_visible } })
	{
		if (!opts.angle || !opts.visible)
			continue;

		float width = 1.5f;
		if (_params.display_mode == DisplayMode::Auxiliary)
			width = 1.2f;

		painter.setPen (get_pen (opts.color, width));
		painter.setTransform (_aircraft_center_transform);
		painter.setClipRect (_map_clip_rect);
		painter.setTransform (_pointers_transform * _aircraft_center_transform);
		painter.rotate (opts.angle->quantity<Degree>());

		if (opts.is_primary)
		{
			double z = 0.13 * _q;
			double delta = 0.5 * z;

			double to_top = -_r - 3.0 * z;
			double to_bottom = -_r + 12.0 * z;

			double from_top = _r - 11.0 * z;
			double from_bottom = _r + 3.0 * z;

			painter.add_shadow ([&] {
				painter.drawLine (QPointF (0.0, to_top + delta), QPointF (0.0, to_bottom));
				painter.drawLine (QPointF (0.0, to_top), QPointF (+z, to_top + 1.4 * z));
				painter.drawLine (QPointF (0.0, to_top), QPointF (-z, to_top + 1.4 * z));
				painter.drawLine (QPointF (-2.0 * z, to_bottom - 0.5 * z), QPointF (+2.0 * z, to_bottom - 0.5 * z));

				painter.drawLine (QPointF (0.0, from_top), QPointF (0.0, from_bottom));
				painter.drawLine (QPointF (-2.0 * z, from_bottom - 1.2 * z), QPointF (0.0, from_bottom - 2.05 * z));
				painter.drawLine (QPointF (+2.0 * z, from_bottom - 1.2 * z), QPointF (0.0, from_bottom - 2.05 * z));
			});
		}
		else
		{
			double z = 0.13 * _q;

			double to_top = -_r - 3.0 * z;
			double to_bottom = -_r + 10.7 * z;
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
			double from_bottom = _r + 0.3 * z;
			QPolygonF bottom_arrow = QPolygonF()
				<< QPointF (0.0, from_top)
				<< QPointF (+z, from_top + 1.2 * z)
				<< QPointF (+z, from_bottom)
				<< QPointF (+2.5 * z, from_bottom + 0.7 * z)
				<< QPointF (+2.5 * z, from_bottom + 2.7 * z)
				<< QPointF (0.0, from_bottom + 1.7 * z)
				<< QPointF (-2.5 * z, from_bottom + 2.7 * z)
				<< QPointF (-2.5 * z, from_bottom + 0.7 * z)
				<< QPointF (-z, from_bottom)
				<< QPointF (-z, from_top + 1.2 * z)
				<< QPointF (0.0, from_top);

			painter.add_shadow ([&] {
				painter.drawPolyline (top_arrow);
				painter.drawPolyline (bottom_arrow);
			});
		}
	}
}


void
HSIWidget::PaintWorkUnit::paint_range (xf::Painter& painter)
{
	if (_params.display_mode == DisplayMode::Expanded || _params.display_mode == DisplayMode::Rose)
	{
		QFont font_a = _font_10;
		font_a.setPixelSize (font_size (11.f));
		QFont font_b = _font_16;
		QFontMetricsF metr_a (font_a);
		QFontMetricsF metr_b (font_b);
		QString s ("RANGE");
		QString r;
		if (_params.range < 1_nmi)
			r = QString::fromStdString ((boost::format ("%.1f") % _params.range.quantity<NauticalMile>()).str());
		else
			r = QString::fromStdString ((boost::format ("%d") % _params.range.quantity<NauticalMile>()).str());

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
HSIWidget::PaintWorkUnit::paint_navaids (xf::Painter& painter)
{
	if (!_params.navaids_visible || !_params.position)
		return;

	float scale = 0.55f * _q;

	painter.setTransform (_aircraft_center_transform);
	painter.setClipPath (_outer_map_clip);
	painter.setFont (_font_10);

	retrieve_navaids();
	paint_locs();

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
				rot.rotate ((1_rad * std::atan2 (mapped_pos.y(), mapped_pos.x())).quantity<Degree>());
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
				painter.drawEllipse (QRectF (-0.1f, -0.1f, 0.2f, 0.2f));
				painter.setTransform (feature_centered_transform);
				painter.fast_draw_text (QPointF (0.15 * _q, 0.10 * _q), Qt::AlignLeft | Qt::AlignTop, navaid.identifier());
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

			case Navaid::FIX:
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

			case Navaid::ARPT:
			{
				if (_params.range > _params.arpt_runways_range_threshold)
				{
					// Draw circles for airports:
					double v = 1.1;
					painter.setTransform (feature_scaled_transform);
					painter.setPen (_arpt_pen);
					painter.setBrush (Qt::NoBrush);
					painter.drawEllipse (QRectF (QPointF (-0.5 * v, -0.5 * v), QSizeF (1.0 * v, 1.0 * v)));
					// Label:
					painter.setTransform (feature_centered_transform);
					painter.fast_draw_text (QPointF (0.46 * scale, 0.46 * scale), Qt::AlignTop | Qt::AlignLeft, navaid.identifier());
				}
				else if (_params.range > _params.arpt_map_range_threshold)
				{
					// Draw airport runways:
					for (xf::Navaid::Runway const& runway: navaid.runways())
					{
						// Make the drawn runway somewhat more wide:
						double half_width = 1.5 * to_px (runway.width());
						QTransform tr_l; tr_l.translate (-half_width, 0.0);
						QTransform tr_r; tr_r.translate (+half_width, 0.0);
						// Find runway's true bearing from pos_1 to pos_2 and runway
						// length in pixels:
						Angle true_bearing = xf::initial_bearing (runway.pos_1(), runway.pos_2());
						double length_px = to_px (xf::haversine_earth (runway.pos_1(), runway.pos_2()));
						double extended_length_px = to_px (_params.arpt_runway_extension_length);
						// Create transform so that the first end of the runway
						// is at (0, 0) and runway extends to the top.
						QPointF point_1 = get_navaid_xy (runway.pos_1());
						QTransform transform = _aircraft_center_transform;
						transform.translate (point_1.x(), point_1.y());
						transform = _features_transform * transform;
						transform.rotate (true_bearing.quantity<Degree>());

						painter.setTransform (transform);
						// The runway:
						painter.setPen (get_pen (Qt::white, 1.0));
						painter.drawLine (tr_l.map (QPointF (0.0, 0.0)), tr_l.map (QPointF (0.0, -length_px)));
						painter.drawLine (tr_r.map (QPointF (0.0, 0.0)), tr_r.map (QPointF (0.0, -length_px)));
						// Extended runway:
						double m_px = xf::clamped<double> (to_px (1_m), 0.02, 0.04);
						QPen dashed_pen = get_pen (Qt::white, 1.0, Qt::DashLine);
						dashed_pen.setDashPattern (QVector<qreal>() << 300 * m_px << 200 * m_px);
						painter.setPen (dashed_pen);
						painter.drawLine (QPointF (0.0, 0.0), QPointF (0.0, extended_length_px));
						painter.drawLine (QPointF (0.0, -length_px), QPointF (0.0, -length_px - extended_length_px));
					}
				}
				else
				{
					// TODO airport map
				}
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

	if (_params.arpt_visible)
		for (auto& navaid: _arpt_navs)
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
HSIWidget::PaintWorkUnit::paint_locs()
{
	if (!_params.loc_visible)
		return;

	QFontMetricsF font_metrics (painter().font());
	QTransform rot_1; rot_1.rotate (-2.f);
	QTransform rot_2; rot_2.rotate (+2.f);
	QPointF zero (0.f, 0.f);

	// Group painting lines and texts as separate tasks. For this,
	// cache texts that need to be drawn later along with their positions.
	std::vector<std::pair<QPointF, QString>> texts_to_paint;
	texts_to_paint.reserve (128);

	auto paint_texts_to_paint = [&]() -> void
	{
		painter().resetTransform();
		for (auto const& text_and_xy: texts_to_paint)
			painter().fast_draw_text (text_and_xy.first, text_and_xy.second);
		texts_to_paint.clear();
	};

	auto paint_loc = [&] (Navaid const& navaid) -> void
	{
		QPointF navaid_pos = get_navaid_xy (navaid.position());
		QTransform transform = _aircraft_center_transform;
		transform.translate (navaid_pos.x(), navaid_pos.y());
		transform = _features_transform * transform;
		transform.rotate (navaid.true_bearing().quantity<Degree>());

		float const line_1 = to_px (navaid.range());
		float const line_2 = 1.03f * line_1;

		QPointF pt_0 (0.f, line_1);
		QPointF pt_1 (rot_1.map (QPointF (0.f, line_2)));
		QPointF pt_2 (rot_2.map (QPointF (0.f, line_2)));

		painter().setTransform (transform);
		if (_params.range < 16_nmi)
			painter().drawLine (zero, pt_0);
		painter().drawLine (zero, pt_1);
		painter().drawLine (zero, pt_2);
		painter().drawLine (pt_0, pt_1);
		painter().drawLine (pt_0, pt_2);

		QPointF text_offset (0.5f * font_metrics.width (navaid.identifier()), -0.35f * font_metrics.height());
		texts_to_paint.emplace_back (transform.map (pt_0 + QPointF (0.f, 0.6f * _q)) - text_offset, navaid.identifier());
	};

	// Paint localizers:
	painter().setBrush (Qt::NoBrush);
	painter().setPen (_lo_loc_pen);
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
		painter().setPen (_hi_loc_pen);
		paint_loc (*hi_loc);
		paint_texts_to_paint();
	}
}


void
HSIWidget::PaintWorkUnit::paint_tcas()
{
	if (!_params.tcas_on)
		return;

	painter().setTransform (_aircraft_center_transform);
	painter().setClipping (false);
	painter().setPen (get_pen (Qt::white, 1.f));

	if (_params.tcas_range)
	{
		double z = 0.075 * _q;
		double v = 0.025 * _q;
		double r = to_px (*_params.tcas_range);

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
					painter().add_shadow ([&] {
						painter().drawEllipse (big_point);
					});
				}
				else
				{
					painter().setBrush (Qt::white);
					painter().add_shadow ([&] {
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
HSIWidget::PaintWorkUnit::retrieve_navaids()
{
	if (!_navaid_storage || !_params.position)
		return;

	if (_navs_retrieved && xf::haversine_earth (_navs_retrieve_position, *_params.position) < 0.1f * _params.range && _params.range == _navs_retrieve_range)
		return;

	_fix_navs.clear();
	_vor_navs.clear();
	_dme_navs.clear();
	_ndb_navs.clear();
	_loc_navs.clear();
	_arpt_navs.clear();

	for (Navaid const& navaid: _navaid_storage->get_navs (*_params.position, std::max (_params.range + 20_nmi, 2.f * _params.range)))
	{
		switch (navaid.type())
		{
			case Navaid::LOC:
				_loc_navs.push_back (navaid);
				break;

			case Navaid::NDB:
				_ndb_navs.push_back (navaid);
				break;

			case Navaid::VOR:
				_vor_navs.push_back (navaid);
				break;

			case Navaid::DME:
				_dme_navs.push_back (navaid);
				break;

			case Navaid::FIX:
				_fix_navs.push_back (navaid);
				break;

			case Navaid::ARPT:
				_arpt_navs.push_back (navaid);
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


inline QPointF
HSIWidget::PaintWorkUnit::get_navaid_xy (LonLat const& navaid_position)
{
	if (!_params.position)
		return QPointF();
	QPointF navaid_pos = xf::kEarthMeanRadius.quantity<si::NauticalMile>() * navaid_position.rotated (*_params.position).project_flat();
	return _features_transform.map (QPointF (to_px (1_nmi * navaid_pos.x()), to_px (1_nmi * navaid_pos.y())));
}


inline Length
HSIWidget::PaintWorkUnit::actual_trend_range() const
{
	if (_params.ground_speed && _params.range <= _params.trend_vector_max_range)
	{
		Time time = 0_s;

		if (_params.range >= _params.trend_vector_min_ranges[2])
			time = _params.trend_vector_times[2];
		else if (_params.range >= _params.trend_vector_min_ranges[1])
			time = _params.trend_vector_times[1];
		else if (_params.range >= _params.trend_vector_min_ranges[0])
			time = _params.trend_vector_times[0];

		return *_params.ground_speed * time;
	}
	else
		return 0_m;
}


inline Length
HSIWidget::PaintWorkUnit::trend_gap() const
{
	switch (_params.display_mode)
	{
		case DisplayMode::Expanded:
			return 0.015f * _params.range;
		case DisplayMode::Rose:
			return 0.030f * _params.range;
		case DisplayMode::Auxiliary:
			return 0.0375f * _params.range;
	}
	return 0_nmi;
}


inline Time
HSIWidget::PaintWorkUnit::trend_time_gap() const
{
	if (_params.ground_speed)
		return trend_gap() / *_params.ground_speed;
	else
		return 0_s;
}


inline float
HSIWidget::PaintWorkUnit::to_px (Length miles)
{
	return miles / _params.range * _r;
}


inline bool
HSIWidget::PaintWorkUnit::is_newly_set (QDateTime const& timestamp, Time time) const
{
	return timestamp.secsTo (_current_datetime) < time.quantity<Second>();
}


HSIWidget::HSIWidget (QWidget* parent, xf::WorkPerformer* work_performer):
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

	auto xw = dynamic_cast<v1::Window*> (window());
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


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

// Local:
#include "hsi.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/earth/earth.h>
#include <xefis/support/earth/navigation/wind_triangle.h>
#include <xefis/support/instrument/text_layout.h>

// Neutrino:
#include <neutrino/numeric.h>

// Qt:
#include <QRadialGradient>

// Standard:
#include <cstddef>


namespace hsi_detail {

void
Parameters::sanitize()
{
	using xf::floored_mod;

	range = xf::clamped<si::Length> (range, 1_ft, 5000_nmi);

	if (heading_magnetic)
		heading_magnetic = floored_mod (*heading_magnetic, 360_deg);

	if (heading_true)
		heading_true = floored_mod (*heading_true, 360_deg);

	if (ap_heading_magnetic)
		ap_heading_magnetic = floored_mod (*ap_heading_magnetic, 360_deg);

	if (ap_track_magnetic)
		ap_track_magnetic = floored_mod (*ap_track_magnetic, 360_deg);

	if (track_magnetic)
		track_magnetic = floored_mod (*track_magnetic, 360_deg);

	if (true_home_direction)
		true_home_direction = floored_mod (*true_home_direction, 360_deg);

	if (wind_from_magnetic_heading)
		wind_from_magnetic_heading = floored_mod (*wind_from_magnetic_heading, 360_deg);
}


PaintingWork::PaintingWork (xf::PaintRequest const& paint_request, xf::InstrumentSupport const& instrument_support, xf::NavaidStorage const& navaid_storage, Parameters const& parameters, ResizeCache& resize_cache, CurrentNavaids& current_navaids, Mutable& mutable_, xf::Logger const& logger):
	_logger (logger),
	_paint_request (paint_request),
	_navaid_storage (navaid_storage),
	_p (parameters),
	_c (resize_cache),
	_current_navaids (current_navaids),
	_mutable (mutable_),
	_painter (instrument_support.get_painter (paint_request)),
	_aids_ptr (instrument_support.get_aids (paint_request)),
	_aids (*_aids_ptr)
{
	if (_p.track_magnetic && _p.heading_magnetic && _p.heading_true)
		_track_true = xf::floored_mod (*_p.track_magnetic + (*_p.heading_true - *_p.heading_magnetic), 360_deg);
	else
		_track_true.reset();

	_track =
		_p.heading_mode == hsi::HeadingMode::Magnetic
			? _p.track_magnetic
			: _track_true;

	_heading =
		_p.heading_mode == hsi::HeadingMode::Magnetic
			? _p.heading_magnetic
			: _p.heading_true;

	_rotation = _p.center_on_track ? _track : _heading;

	if (_heading)
		_heading_transform.rotate (-_heading->in<si::Degree>());

	if (_track)
		_track_transform.rotate (-_track->in<si::Degree>());

	_rotation_transform =
		_p.center_on_track
			? _track_transform
			: _heading_transform;

	_features_transform = _rotation_transform;

	if (_p.heading_mode == hsi::HeadingMode::Magnetic)
		if (_p.heading_magnetic && _p.heading_true)
			_features_transform.rotate ((*_p.heading_magnetic - *_p.heading_true).in<si::Degree>());

	_pointers_transform = _rotation_transform;

	if (_p.heading_mode == hsi::HeadingMode::True)
		if (_p.heading_magnetic && _p.heading_true)
			_pointers_transform.rotate ((*_p.heading_true - *_p.heading_magnetic).in<si::Degree>());

	_ap_use_trk = _p.ap_use_trk;

	// If use_trk is not nil, use proper heading or track information to position cmd bug.
	if (_ap_use_trk)
	{
		if (*_ap_use_trk)
			_ap_bug_magnetic = _p.ap_track_magnetic;
		else
			_ap_bug_magnetic = _p.ap_heading_magnetic;
	}
	// If use_trk is unavailable (nil), then use the only heading/magnetic socket
	// that is set. If both or neither is set, fail.
	else
	{
		if (!_p.ap_heading_magnetic != !_p.ap_track_magnetic)
		{
			if (_p.ap_heading_magnetic)
			{
				_ap_bug_magnetic = _p.ap_heading_magnetic;
				_ap_use_trk = false;
			}
			else
			{
				_ap_bug_magnetic = _p.ap_track_magnetic;
				_ap_use_trk = true;
			}
		}
		else
		{
			_ap_bug_magnetic.reset();
			_ap_use_trk.reset();
		}
	}

	// Finish up cmd bug setting:
	if (_ap_bug_magnetic && _p.heading_magnetic && _p.heading_true)
	{
		if (_p.heading_mode == hsi::HeadingMode::True)
			*_ap_bug_magnetic += *_p.heading_true - *_p.heading_magnetic;

		_ap_bug_magnetic = xf::floored_mod (*_ap_bug_magnetic, 360_deg);
	}

	if (_p.course_setting_magnetic && _p.heading_magnetic && _p.heading_true)
	{
		_course_heading = *_p.course_setting_magnetic;

		if (_p.heading_mode == hsi::HeadingMode::True)
			*_course_heading += *_p.heading_true - *_p.heading_magnetic;

		_course_heading = xf::floored_mod (*_course_heading, 360_deg);
	}

	_navaid_selected_visible =
		!_p.navaid_selected_reference.isEmpty() || !_p.navaid_selected_identifier.isEmpty() ||
		_p.navaid_selected_distance || _p.navaid_selected_eta;

	_navaid_left_visible =
		!_p.navaid_left_reference.isEmpty() || !_p.navaid_left_identifier.isEmpty() ||
		_p.navaid_left_distance || _p.navaid_left_initial_bearing_magnetic;

	_navaid_right_visible =
		!_p.navaid_right_reference.isEmpty() || !_p.navaid_right_identifier.isEmpty() ||
		_p.navaid_right_distance || _p.navaid_right_initial_bearing_magnetic;

	if (_p.display_mode != _mutable.prev_display_mode || _paint_request.size_changed())
	{
		auto const size = _paint_request.metric().canvas_size();
		auto const ld = _aids.lesser_dimension();

		// Clippings:
		switch (_p.display_mode)
		{
			case hsi::DisplayMode::Expanded:
			{
				_c.q = 0.0500f * size.height();
				_c.r = 0.7111f * size.height();
				float const rx = to_px (_p.range);

				_c.aircraft_center_transform.reset();
				_c.aircraft_center_transform.translate (0.5f * size.width(), 0.8f * size.height());

				_c.map_clip_rect = QRectF (-1.1f * _c.r, -1.1f * _c.r, 2.2f * _c.r, 2.2f * _c.r);
				_c.trend_vector_clip_rect = QRectF (-rx, -rx, 2.f * rx, rx);

				_c.inner_map_clip = QPainterPath();
				_c.inner_map_clip.addEllipse (QRectF (-0.85f * _c.r, -0.85f * _c.r, 1.7f * _c.r, 1.7f * _c.r));
				_c.outer_map_clip = QPainterPath();

				if (_p.round_clip)
					_c.outer_map_clip.addEllipse (QRectF (-rx, -rx, 2.f * rx, 2.f * rx));
				else
					_c.outer_map_clip.addRect (QRectF (-rx, -rx, 2.f * rx, 2.f * rx));

				_c.radials_font = _aids.scaled_default_font (1.6f);
				break;
			}

			case hsi::DisplayMode::Rose:
			{
				_c.q = 0.05f * size.height();
				_c.r = 0.40f * size.height();

				if (_c.r > 0.85f * ld)
					_c.r = 0.85f * ld;

				float const rx = to_px (_p.range);

				_c.aircraft_center_transform.reset();
				_c.aircraft_center_transform.translate (0.5f * size.width(), 0.5f * size.height());

				_c.map_clip_rect = QRectF (-1.1f * _c.r, -1.1f * _c.r, 2.2f * _c.r, 2.2f * _c.r);
				_c.trend_vector_clip_rect = QRectF (-rx, -rx, 2.f * rx, rx);

				_c.inner_map_clip = QPainterPath();
				_c.inner_map_clip.addEllipse (QRectF (-0.85f * _c.r, -0.85f * _c.r, 1.7f * _c.r, 1.7f * _c.r));
				_c.outer_map_clip = QPainterPath();

				if (_p.round_clip)
					_c.outer_map_clip.addEllipse (QRectF (-rx, -rx, 2.f * rx, 2.f * rx));
				else
					_c.outer_map_clip.addRect (QRectF (-rx, -rx, 2.f * rx, 2.f * rx));

				_c.radials_font = _aids.scaled_default_font (1.6f);
				break;
			}

			case hsi::DisplayMode::Auxiliary:
			{
				_c.q = 0.1f * ld;
				_c.r = 6.5f * _c.q;
				float const rx = to_px (_p.range);

				_c.aircraft_center_transform.reset();
				_c.aircraft_center_transform.translate (0.5f * size.width(), 0.705f * size.height());

				_c.map_clip_rect = QRectF (-1.1f * _c.r, -1.1f * _c.r, 2.2f * _c.r, 1.11f * _c.r);
				_c.trend_vector_clip_rect = QRectF (-rx, -rx, 2.f * rx, rx);

				QPainterPath clip1;
				clip1.addEllipse (QRectF (-0.85f * _c.r, -0.85f * _c.r, 1.7f * _c.r, 1.7f * _c.r));
				QPainterPath clip2;

				if (_p.round_clip)
					clip2.addEllipse (QRectF (-rx, -rx, 2.f * rx, 2.f * rx));
				else
					clip2.addRect (QRectF (-rx, -rx, 2.f * rx, 2.f * rx));

				QPainterPath clip3;
				clip3.addRect (QRectF (-rx, -rx, 2.f * rx, 1.45f * rx));

				_c.inner_map_clip = clip1 & clip3;
				_c.outer_map_clip = clip2 & clip3;

				_c.radials_font = _aids.scaled_default_font (1.3f);
				break;
			}
		}

		// Navaids pens:
		_c.lo_loc_pen = _aids.get_pen (Qt::blue, 0.8f, Qt::SolidLine, Qt::RoundCap, Qt::BevelJoin);
		_c.hi_loc_pen = _aids.get_pen (Qt::cyan, 0.8f, Qt::SolidLine, Qt::RoundCap, Qt::BevelJoin);

		// Unscaled pens:
		_c.ndb_pen = QPen (QColor (99, 99, 99), 0.09f, Qt::SolidLine, Qt::RoundCap, Qt::BevelJoin);
		_c.vor_pen = QPen (Qt::green, 0.09f, Qt::SolidLine, Qt::RoundCap, Qt::BevelJoin);
		_c.dme_pen = QPen (Qt::green, 0.09f, Qt::SolidLine, Qt::RoundCap, Qt::BevelJoin);
		_c.fix_pen = QPen (QColor (0, 132, 255), 0.1f, Qt::SolidLine, Qt::RoundCap, Qt::BevelJoin);
		_c.arpt_pen = QPen (Qt::white, 0.1f, Qt::SolidLine, Qt::RoundCap, Qt::BevelJoin);
		_c.home_pen = QPen (Qt::green, 0.1f, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin);

		_c.dme_for_vor_shape = QPolygonF()
			<< QPointF (-0.5f, -0.5f)
			<< QPointF (-0.5f, +0.5f)
			<< QPointF (+0.5f, +0.5f)
			<< QPointF (+0.5f, -0.5f)
			<< QPointF (-0.5f, -0.5f);

		QTransform t;
		_c.vortac_shape = QPolygonF();
		t.rotate (60.f);

		for (int i = 0; i < 4; ++i)
		{
			float const x = 0.18f;
			float const y1 = 0.28f;
			float const y2 = 0.48f;
			_c.vortac_shape << t.map (QPointF (-x, -y1));

			if (i == 3)
				break;

			_c.vortac_shape << t.map (QPointF (-x, -y2));
			_c.vortac_shape << t.map (QPointF (+x, -y2));
			_c.vortac_shape << t.map (QPointF (+x, -y1));
			t.rotate (120.f);
		}

		_c.vor_shape = QPolygonF()
			<< QPointF (-0.5f, 0.f)
			<< QPointF (-0.25f, -0.44f)
			<< QPointF (+0.25f, -0.44f)
			<< QPointF (+0.5f, 0.f)
			<< QPointF (+0.25f, +0.44f)
			<< QPointF (-0.25f, +0.44f)
			<< QPointF (-0.5f, 0.f);

		_c.home_shape = QPolygonF()
			<< QPointF (-0.4f, 0.f)
			<< QPointF (0.f, -0.5f)
			<< QPointF (+0.4f, 0.f)
			<< QPointF (0.f, +0.5f)
			<< QPointF (-0.4f, 0.f);

		_c.aircraft_shape = QPolygonF()
			<< QPointF (0.f, 0.f)
			<< QPointF (+0.45f * _c.q, _c.q)
			<< QPointF (-0.45f * _c.q, _c.q)
			<< QPointF (0.f, 0.f);

		_c.ap_bug_shape = QPolygonF()
			<< QPointF (0.f, 0.f)
			<< QPointF (+0.45f * _c.q, _c.q)
			<< QPointF (+0.85f * _c.q, _c.q)
			<< QPointF (+0.85f * _c.q, 0.f)
			<< QPointF (-0.85f * _c.q, 0.f)
			<< QPointF (-0.85f * _c.q, _c.q)
			<< QPointF (-0.45f * _c.q, _c.q)
			<< QPointF (0.f, 0.f);

		for (auto& point: _c.ap_bug_shape)
		{
			point.rx() *= +0.5f;
			point.ry() *= -0.5f;
		}

		_c.hmargin = 0.15f * _c.q;
		_c.vmargin = 0.02f * _c.q;

		_c.black_shadow = _aids.default_shadow();
		_c.black_shadow.set_color (Qt::black);
	}

	if (_p.range != _mutable.prev_range || _paint_request.size_changed()) // TODO or input socket radio_range_warning/_critical changes by more than… say, 100_m?
		update_radio_range_heat_map();

	_mutable.prev_display_mode = _p.display_mode;
	_mutable.prev_range = _p.range;
}


void
PaintingWork::paint()
{
	paint_radio_range_map();
	paint_navaids();
	paint_flight_ranges();
	paint_altitude_reach();
	paint_track (false);
	paint_directions();
	paint_track (true);
	paint_ap_settings();
	paint_speeds_and_wind();
	paint_home_direction();
	paint_range();
	paint_hints();
	paint_trend_vector();
	paint_tcas();
	paint_course();
	paint_selected_navaid_info();
	paint_tcas_and_navaid_info();
	paint_pointers();
	paint_aircraft();
	paint_navperf();
}


void
PaintingWork::paint_aircraft()
{
	_painter.setTransform (_c.aircraft_center_transform);
	_painter.setClipping (false);

	// Aircraft triangle - shadow and triangle:
	_painter.setPen (_aids.get_pen (Qt::white, 1.f));
	_painter.paint (_c.black_shadow, [&] {
		_painter.drawPolyline (_c.aircraft_shape);
	});

	_painter.resetTransform();
	_painter.setClipping (false);

	// AP info: SEL HDG/TRK 000
	if (_p.display_mode == hsi::DisplayMode::Auxiliary && _ap_bug_magnetic && _ap_use_trk)
	{
		int sel_hdg = static_cast<int> (_ap_bug_magnetic->in<si::Degree>() + 0.5f) % 360;

		if (sel_hdg == 0)
			sel_hdg = 360;

		QString str = *_ap_use_trk ? "SEL TRK " : "SEL HDG ";
		// AP heading always set as magnetic, but can be displayed as true:
		xf::TextLayout layout;
		layout.set_background (Qt::black, { _c.hmargin, 0.0f });
		layout.add_fragment (str, _aids.font_2.font, _aids.autopilot_pen_2.color());
		layout.add_fragment (QString ("%1").arg (sel_hdg, 3, 10, QChar ('0')), _aids.font_3.font, _aids.autopilot_pen_2.color());
		layout.paint (QPointF (0.5f * _aids.width() - _c.q, _aids.height() - _c.vmargin), Qt::AlignBottom | Qt::AlignRight, _painter);
	}

	// MAG/TRUE heading
	if (_p.heading_magnetic && _p.heading_true)
	{
		int hdg = 0;

		if (_p.center_on_track)
		{
			if (_track)
				hdg = _track->in<si::Degree>() + 0.5f;
		}
		else
		{
			if (_heading)
				hdg = _heading->in<si::Degree>() + 0.5f;
		}

		hdg %= 360;

		if (hdg == 0)
			hdg = 360;

		switch (_p.display_mode)
		{
			case hsi::DisplayMode::Auxiliary:
			{
				QString text_1 =
					QString (_p.heading_mode == hsi::HeadingMode::Magnetic ? "MAG" : "TRU") +
					QString (_p.center_on_track ? " TRK" : "");
				QPen box_pen = Qt::NoPen;

				// True heading is boxed for emphasis:
				if (_p.heading_mode == hsi::HeadingMode::True)
					box_pen = _aids.get_pen (xf::InstrumentAids::kNavigationColor, 1.0f);

				xf::TextLayout layout;
				layout.set_background (Qt::black, { _c.hmargin, 0.0f });
				layout.add_fragment (text_1 + " ", _aids.font_2.font, xf::InstrumentAids::kNavigationColor);
				layout.add_fragment (QString ("%1").arg (hdg, 3, 10, QChar ('0')), _aids.font_3.font, xf::InstrumentAids::kNavigationColor, box_pen);
				layout.paint (QPointF (0.5f * _aids.width() + _c.q, _aids.height() - _c.vmargin), Qt::AlignBottom | Qt::AlignLeft, _painter);
				break;
			}

			default:
			{
				QString text_1 = _p.center_on_track ? "TRK" : "HDG";
				QString text_2 = _p.heading_mode == hsi::HeadingMode::Magnetic ? "MAG" : "TRU";
				QString text_v = QString ("%1").arg (hdg, 3, 10, QChar ('0'));

				float margin = 0.2f * _c.q;

				QFont font_1 (_aids.font_3.font);
				QFont font_2 (_aids.font_5.font);
				QFontMetricsF metrics_1 (font_1);
				QFontMetricsF metrics_2 (font_2);
				QRectF rect_v (0.f, 0.f, metrics_2.width (text_v), metrics_2.height());
				_aids.centrify (rect_v);
				rect_v.adjust (-margin, 0.f, +margin, 0.f);
				QRectF rect_1 (0.f, 0.f, metrics_1.width (text_1), metrics_1.height());
				_aids.centrify (rect_1);
				rect_1.moveRight (rect_v.left() - 0.2f * _c.q);
				QRectF rect_2 (0.f, 0.f, metrics_1.width (text_2), metrics_1.height());
				_aids.centrify (rect_2);
				rect_2.moveLeft (rect_v.right() + 0.2f * _c.q);

				_painter.setTransform (_c.aircraft_center_transform);
				_painter.translate (0.f, -_c.r - 1.05f * _c.q);
				_painter.setPen (_aids.get_pen (Qt::white, 1.f));
				_painter.setBrush (Qt::NoBrush);
				_painter.setFont (font_2);
				_painter.drawLine (rect_v.topLeft(), rect_v.bottomLeft());
				_painter.drawLine (rect_v.topRight(), rect_v.bottomRight());
				_painter.drawLine (rect_v.bottomLeft(), rect_v.bottomRight());
				_painter.fast_draw_text (rect_v, Qt::AlignVCenter | Qt::AlignHCenter, text_v);
				_painter.setPen (_aids.get_pen (xf::InstrumentAids::kNavigationColor, 1.f));
				_painter.setFont (font_1);
				_painter.fast_draw_text (rect_1, Qt::AlignVCenter | Qt::AlignHCenter, text_1);
				_painter.fast_draw_text (rect_2, Qt::AlignVCenter | Qt::AlignHCenter, text_2);
				break;
			}
		}
	}
}


void
PaintingWork::paint_navperf()
{
	auto const size = _paint_request.metric().canvas_size();

	if (_p.display_mode != hsi::DisplayMode::Auxiliary)
	{
		if (_p.navigation_required_performance ||
			_p.navigation_actual_performance)
		{
			auto const font = _aids.font_2.font;
			float const x = 0.045f * size.width();

			if (_p.navigation_required_performance)
			{
				_painter.resetTransform();
				_painter.setClipping (false);
				_painter.translate (0.5f * size.width(), size.height());

				auto val = QString ("%1").arg (_p.navigation_required_performance->in<si::Meter>(), 0, 'f', 2);

				xf::TextLayout layout;
				layout.set_background (Qt::black, { _c.hmargin, 0.0f });
				layout.set_alignment (Qt::AlignHCenter);
				layout.add_fragment ("RNP", font, xf::InstrumentAids::kNavigationColor);
				layout.add_new_line();
				layout.add_fragment (val, font, xf::InstrumentAids::kNavigationColor);
				layout.paint (QPointF (-x, 0.0f), Qt::AlignBottom | Qt::AlignHCenter, _painter);
			}

			if (_p.navigation_actual_performance)
			{
				_painter.resetTransform();
				_painter.setClipping (false);
				_painter.translate (0.5f * size.width(), size.height());

				auto val = QString ("%1").arg (_p.navigation_actual_performance->in<si::Meter>(), 0, 'f', 2);

				QColor text_color = xf::InstrumentAids::kNavigationColor;
				if (_p.navigation_required_performance)
					if (*_p.navigation_required_performance < *_p.navigation_actual_performance)
						text_color = xf::InstrumentAids::kWarningColor;

				xf::TextLayout layout;
				layout.set_background (Qt::black, { _c.hmargin, 0.0f });
				layout.set_alignment (Qt::AlignHCenter);
				layout.add_fragment ("ANP", font, text_color);
				layout.add_new_line();
				layout.add_fragment (val, font, text_color);
				layout.paint (QPointF (+x, 0.0f), Qt::AlignBottom | Qt::AlignHCenter, _painter);
			}
		}
	}
}


void
PaintingWork::paint_hints()
{
	if (!*_p.positioning_hint || !_p.position)
		return;

	auto const size = _paint_request.metric().canvas_size();

	_painter.resetTransform();
	_painter.setClipping (false);

	float const x = _p.display_mode == hsi::DisplayMode::Auxiliary ? 0.775f * size.width() : 0.725f * size.width();
	QString hint = _p.positioning_hint->value_or ("");

	// Box for emphasis:
	QPen box_pen = Qt::NoPen;

	if (_p.update_time < _p.positioning_hint.update_time() + 10_s)
	{
		if (hint == "")
			hint = "---";

		box_pen = _aids.get_pen (xf::InstrumentAids::kNavigationColor, 1.0f);
	}

	xf::TextLayout layout;
	layout.set_background (Qt::black, { 2 * _c.hmargin, 0.0f });
	// This is needed for correct v-alignment with other layouts that use mixed font_2/font_3 sizes:
	layout.add_fragment ("", _aids.font_3.font, xf::InstrumentAids::kNavigationColor);
	layout.add_fragment (hint, _aids.font_2.font, xf::InstrumentAids::kNavigationColor, box_pen);
	layout.paint (QPointF (x, size.height() - _c.vmargin), Qt::AlignBottom | Qt::AlignHCenter, _painter);
}


void
PaintingWork::paint_ap_settings()
{
	if (!_p.ap_visible || !_rotation)
		return;

	// AP dashed line:
	if (_p.ap_line_visible && _ap_bug_magnetic)
	{
		float pink_pen_width = 1.5f;
		float shadow_pen_width = 2.5f;

		if (_p.display_mode == hsi::DisplayMode::Auxiliary)
		{
			pink_pen_width = 1.2f;
			shadow_pen_width = 2.2f;
		}

		float const shadow_scale = shadow_pen_width / pink_pen_width;

		QPen pen = _aids.get_pen (xf::InstrumentAids::kAutopilotColor, pink_pen_width, Qt::DashLine, Qt::RoundCap);
		pen.setDashPattern (QVector<qreal>() << 7.5 << 12);

		QPen shadow_pen = _aids.get_pen (_c.black_shadow.color(), shadow_pen_width, Qt::DashLine, Qt::RoundCap);
		shadow_pen.setDashPattern (QVector<qreal>() << 7.5 / shadow_scale << 12 / shadow_scale);

		_painter.setTransform (_c.aircraft_center_transform);
		_painter.setClipPath (_c.outer_map_clip);
		_painter.rotate ((*_ap_bug_magnetic - *_rotation).in<si::Degree>());

		for (auto const& p: { shadow_pen, pen })
		{
			_painter.setPen (p);
			_painter.drawLine (QPointF (0.f, 0.f), QPointF (0.f, -_c.r));
		}
	}

	// A/P bug
	if (_p.heading_magnetic && _p.heading_true && _ap_bug_magnetic)
	{
		si::Angle limited_rotation;

		switch (_p.display_mode)
		{
			case hsi::DisplayMode::Auxiliary:
				limited_rotation = xf::floored_mod (*_ap_bug_magnetic - *_rotation + 180_deg, 360_deg) - 180_deg;
				break;

			default:
				limited_rotation = *_ap_bug_magnetic - *_rotation;
				break;
		}

		QTransform transform = _c.aircraft_center_transform;
		transform.rotate (limited_rotation.in<si::Degree>());
		transform.translate (0.f, -_c.r);

		QPen pen_1 = _aids.autopilot_pen_1;
		pen_1.setMiterLimit (0.2f);
		QPen pen_2 = _aids.autopilot_pen_2;
		pen_2.setMiterLimit (0.2f);

		_painter.setTransform (_c.aircraft_center_transform);
		_painter.setClipRect (_c.map_clip_rect);
		_painter.setTransform (transform);
		_painter.setPen (pen_1);
		_painter.drawPolyline (_c.ap_bug_shape);
		_painter.setPen (pen_2);
		_painter.drawPolyline (_c.ap_bug_shape);
	}
}


void
PaintingWork::paint_directions()
{
	if (!_p.heading_magnetic || !_p.heading_true)
		return;

	QPen pen = _aids.get_pen (Qt::white, 1.f, Qt::SolidLine, Qt::RoundCap);

	_painter.setTransform (_c.aircraft_center_transform);
	_painter.setClipRect (_c.map_clip_rect);
	_painter.setPen (pen);
	_painter.setFont (_c.radials_font);
	_painter.setBrush (Qt::NoBrush);

	QTransform t = _rotation_transform * _c.aircraft_center_transform;

	_painter.paint (_c.black_shadow, [&] (bool painting_shadow) {
		_painter.setTransform (_c.aircraft_center_transform);

		QPointF line_long;
		QPointF line_short;
		float radial_ypos;

		if (_p.display_mode == hsi::DisplayMode::Auxiliary)
		{
			line_long = QPointF (0.f, -0.935f * _c.r);
			line_short = QPointF (0.f, -0.965f * _c.r);
			radial_ypos = -0.925f * _c.r;
		}
		else
		{
			line_long = QPointF (0.f, -0.955f * _c.r);
			line_short = QPointF (0.f, -0.980f * _c.r);
			radial_ypos = -0.945f * _c.r;
		}

		for (int deg = 5; deg <= 360; deg += 5)
		{
			QPointF sp = deg % 10 == 0 ? line_long : line_short;
			_painter.setTransform (t);
			_painter.rotate (deg);
			_painter.drawLine (QPointF (0.f, -_c.r + 0.025 * _c.q), sp);

			if (!painting_shadow)
			{
				if (deg % 30 == 0)
					_painter.fast_draw_text (QRectF (-_c.q, radial_ypos, 2.f * _c.q, 0.5f * _c.q),
											 Qt::AlignVCenter | Qt::AlignHCenter, QString::number (deg / 10));
			}
		}

		// Circle around radials:
		if (_p.display_mode == hsi::DisplayMode::Expanded)
			_painter.drawEllipse (QRectF (-_c.r, -_c.r, 2.f * _c.r, 2.f * _c.r));
	});

	if (_p.display_mode == hsi::DisplayMode::Rose)
	{
		_painter.setClipping (false);
		_painter.setTransform (_c.aircraft_center_transform);
		// 8 lines around the circle:
		for (int deg = 45; deg < 360; deg += 45)
		{
			_painter.rotate (45);
			_painter.paint (_c.black_shadow, [&] {
				_painter.drawLine (QPointF (0.f, -1.025f * _c.r), QPointF (0.f, -1.125f * _c.r));
			});
		}
	}
}


void
PaintingWork::paint_track (bool paint_heading_triangle)
{
	si::Length const trend_range = actual_trend_range();
	float const start_point = _p.track_lateral_rotation ? -to_px (trend_range) - 0.25f * _c.q : 0.f;

	_painter.setTransform (_c.aircraft_center_transform);
	_painter.setClipping (false);

	QFont font = _aids.font_2.font;
	QFontMetricsF metrics (font);

	if (!paint_heading_triangle && _p.track_visible && _track && _rotation)
	{
		// Scale and track line:
		_painter.setPen (_aids.get_pen (xf::InstrumentAids::kSilver, 1.3f, Qt::SolidLine, Qt::RoundCap));
		_painter.rotate ((*_track - *_rotation).in<si::Degree>());
		float extension = 0.0;

		if (_p.display_mode != hsi::DisplayMode::Auxiliary && _p.center_on_track)
			extension = 0.6 * _c.q;

		_painter.paint (_c.black_shadow, [&] {
			_painter.drawLine (QPointF (0.f, start_point), QPointF (0.f, -_c.r - extension));
		});
		_painter.setPen (_aids.get_pen (Qt::white, 1.3f, Qt::SolidLine, Qt::RoundCap));
	}

	if (!paint_heading_triangle)
	{
		// Scale ticks:
		auto paint_range_tick = [&] (float ratio, bool draw_text) -> void
		{
			si::Length range;

			if (ratio == 0.5 && _p.range >= 2_nmi)
				range = 1_nmi * std::round (((10.f * ratio * _p.range) / 10.f).in<si::NauticalMile>());
			else
				range = ratio * _p.range;

			float range_tick_vpx = to_px (range);
			float range_tick_hpx = 0.1f * _c.q;
			int precision = 0;

			if (range < 1_nmi)
				precision = 1;

			QString half_range_str = QString ("%1").arg (range.in<si::NauticalMile>(), 0, 'f', precision);

			_painter.paint (_c.black_shadow, [&] {
				_painter.drawLine (QPointF (-range_tick_hpx, -range_tick_vpx), QPointF (range_tick_hpx, -range_tick_vpx));
			});

			if (draw_text)
			{
				QRectF half_range_rect (0.f, 0.f, metrics.width (half_range_str), metrics.height());
				_aids.centrify (half_range_rect);
				half_range_rect.moveRight (-2.f * range_tick_hpx);
				half_range_rect.translate (0.f, -range_tick_vpx);
				_painter.setFont (font);
				_painter.fast_draw_text (half_range_rect, Qt::AlignVCenter | Qt::AlignHCenter, half_range_str);
			}
		};

		paint_range_tick (0.5, true);
		if (_p.display_mode != hsi::DisplayMode::Auxiliary)
		{
			paint_range_tick (0.25, false);
			paint_range_tick (0.75, false);
		}
	}

	if (_p.heading_magnetic && _p.heading_true && paint_heading_triangle && _heading && _rotation)
	{
		// Heading triangle:
		_painter.setClipRect (_c.map_clip_rect);
		_painter.setTransform (_c.aircraft_center_transform);
		_painter.rotate ((*_heading - *_rotation).in<si::Degree>());

		_painter.setPen (_aids.get_pen (Qt::white, 2.2f));
		_painter.translate (0.f, -1.003f * _c.r);
		_painter.scale (0.465f, -0.465f);
		_painter.paint (_c.black_shadow, [&] {
			_painter.drawPolyline (_c.aircraft_shape);
		});
	}
}


void
PaintingWork::paint_altitude_reach()
{
	using namespace xf::literals;

	if (!_p.altitude_reach_distance || (*_p.altitude_reach_distance < 0.005f * _p.range) || (0.8f * _p.range < *_p.altitude_reach_distance))
		return;

	float len = xf::clamped (to_px (6_nmi), 2.f * _c.q, 7.f * _c.q);
	float pos = to_px (*_p.altitude_reach_distance);
	QRectF rect (0.f, 0.f, len, len);
	_aids.centrify (rect);
	rect.moveTop (-pos);

	if (std::isfinite (pos))
	{
		_painter.setTransform (_c.aircraft_center_transform);
		_painter.setClipping (false);
		_painter.setPen (_aids.get_pen (xf::InstrumentAids::kNavigationColor, 1.f));
		_painter.drawArc (rect, 50_qdeg, 80_qdeg);
	}
}


void
PaintingWork::paint_trend_vector()
{
	using std::abs;

	QPen est_pen = _aids.get_pen (Qt::white, 1.f, Qt::SolidLine, Qt::RoundCap);

	_painter.setTransform (_c.aircraft_center_transform);
	_painter.setClipPath (_c.inner_map_clip);
	_painter.setPen (est_pen);

	if (_p.track_lateral_rotation && _p.ground_speed &&
		_track && _rotation &&
		2.f * trend_time_gap() < _p.trend_vector_durations[2] && _p.range <= _p.trend_vector_max_range)
	{
		_painter.setPen (est_pen);
		_painter.setTransform (_c.aircraft_center_transform);
		_painter.setClipRect (_c.trend_vector_clip_rect);

		si::Time step = *std::min_element (_p.trend_vector_durations.begin(), _p.trend_vector_durations.end()) / 100.0;
		si::Angle const angle_per_step = step * *_p.track_lateral_rotation;
		si::Angle total_angle = 0_deg;

		QTransform transform;
		QPolygonF polygon;

		// Initially rotate the transform to match HDG or TRK setting:
		transform.rotate ((*_track - *_rotation).in<si::Degree>());

		// Take wind into consideration if track info is available:
		std::optional<xf::WindTriangle> wt;

		if (_p.true_air_speed && _p.heading_magnetic && _p.heading_true && _p.track_visible && _p.track_magnetic)
		{
			wt = xf::WindTriangle();
			wt->set_air_vector (*_p.true_air_speed, *_p.heading_magnetic);
			wt->set_ground_vector (*_p.ground_speed, *_p.track_magnetic);
			wt->compute_wind_vector();
		}

		for (si::Time t = 0_s; t < _p.trend_vector_durations[2]; t += step)
		{
			transform.rotate (angle_per_step.in<si::Degree>());
			total_angle += angle_per_step;

			si::Speed ground_speed = wt
				? wt->get_ground_speed (*_track + total_angle)
				: *_p.ground_speed;

			float px = to_px (ground_speed * step);

			// If the turn is too tight, stop drawing trend vectors:
			if (abs (total_angle) >= 180_deg)
			{
				polygon.clear();
				break;
			}

			if ((_p.trend_vector_min_ranges[0] <= _p.range && trend_time_gap() <= t && t < _p.trend_vector_durations[0]) ||
				(_p.trend_vector_min_ranges[1] <= _p.range && trend_time_gap() + _p.trend_vector_durations[0] <= t && t < _p.trend_vector_durations[1]) ||
				(_p.trend_vector_min_ranges[2] <= _p.range && trend_time_gap() + _p.trend_vector_durations[1] <= t && t < _p.trend_vector_durations[2]))
			{
				polygon << transform.map (QPointF (0.f, -px));
			}
			else if (!polygon.empty())
			{
				_painter.paint (_c.black_shadow, [&] {
					_painter.drawPolyline (polygon);
				});
				polygon.clear();
			}

			transform.translate (0.f, -px);
		}

		if (!polygon.empty())
		{
			_painter.paint (_c.black_shadow, [&] {
				_painter.drawPolyline (polygon);
			});
		}
	}
}


void
PaintingWork::paint_speeds_and_wind()
{
	QFont const font_a = _aids.font_2.font;
	QFont const font_b = _aids.font_4.font;
	QFontMetricsF const metr_a (font_a);
	QFontMetricsF const metr_b (font_b);

	xf::TextLayout layout;
	layout.set_alignment (Qt::AlignLeft);

	// GS
	layout.add_fragment ("GS", font_a, Qt::white);
	QString gs_str = "---";

	if (_p.ground_speed)
		gs_str = QString::number (static_cast<int> (_p.ground_speed->in<si::Knot>()));

	layout.add_fragment (gs_str, font_b, Qt::white);
	layout.add_fragment (" ", font_a, Qt::white);

	// TAS
	layout.add_fragment ("TAS", font_a, Qt::white);
	QString tas_str = "---";

	if (_p.true_air_speed)
		tas_str = QString::number (static_cast<int> (_p.true_air_speed->in<si::Knot>()));

	layout.add_fragment (tas_str, font_b, Qt::white);

	// Wind data (direction/strength):
	if (_p.wind_from_magnetic_heading || _p.wind_tas_speed)
	{
		QString s_dir ("---°");

		if (_p.wind_from_magnetic_heading)
			s_dir = QString ("%1°").arg (static_cast<long> (_p.wind_from_magnetic_heading->in<si::Degree>()), 3, 10, QChar ('0'));

		QString s_tas ("---");

		if (_p.wind_tas_speed)
			s_tas = QString ("%1").arg (static_cast<long> (_p.wind_tas_speed->in<si::Knot>()), 3, 10, QChar (L'\u2007'));

		QString wind_str = s_dir + "/" + s_tas;
		layout.add_new_line();
		layout.add_fragment (wind_str, _aids.font_3.font, Qt::white);
	}

	_painter.resetTransform();
	_painter.setClipping (false);
	layout.paint (QPointF (_c.hmargin, 0.0), Qt::AlignTop | Qt::AlignLeft, _painter);

	// Wind arrow:
	if (_p.wind_from_magnetic_heading && _p.heading_magnetic)
	{
		_painter.setPen (_aids.get_pen (Qt::white, 0.6f));
		_painter.translate (0.8f * _c.q + _c.hmargin, 0.8f * _c.q + layout.height());
		_painter.rotate ((*_p.wind_from_magnetic_heading - *_p.heading_magnetic + 180_deg).in<si::Degree>());
		_painter.setPen (_aids.get_pen (Qt::white, 1.0));
		_painter.paint (_c.black_shadow, [&] {
			auto const a = QPointF (0.f, -0.7f * _c.q);
			auto const b = QPointF (0.f, +0.7f * _c.q);
			auto const r = 0.15f * _c.q;

			_painter.drawLine (a + QPointF (0.f, 0.05f * _c.q), b);
			_painter.drawLine (a, a + QPointF (+r, +r));
			_painter.drawLine (a, a + QPointF (-r, +r));
		});
	}
}


void
PaintingWork::paint_home_direction()
{
	if (_p.display_mode != hsi::DisplayMode::Auxiliary)
		return;

	if (!_p.position || !_p.home)
		return;

	auto const size = _paint_request.metric().canvas_size();
	QTransform base_transform;
	base_transform.translate (size.width() - _c.hmargin, 0.55f * size.height());

	_painter.resetTransform();
	_painter.setClipping (false);

	// Home direction arrow:
	if (_p.true_home_direction && _p.heading_true)
	{
		bool at_home = xf::haversine_earth (*_p.home, *_p.position) < 10_m;
		float z = 0.75f * _c.q;

		_painter.setTransform (base_transform);
		_painter.translate (-z - 0.1f * _c.q, _c.q);

		if (at_home)
		{
			_painter.setPen (_aids.get_pen (Qt::white, 1.25));
			float const v = 0.35f * z;
			_painter.setBrush (Qt::black);
			_painter.drawEllipse (QRectF (-v, -v, 2.f * v, 2.f * v));
		}
		else
		{
			_painter.setPen (_aids.get_pen (Qt::white, 1.0));
			QPolygonF home_arrow = QPolygonF()
				<< QPointF (0.0, z)
				<< QPointF (0.0, -0.8 * z)
				<< QPointF (-0.2 * z, -0.8 * z)
				<< QPointF (0.0, -z)
				<< QPointF (+0.2 * z, -0.8 * z)
				<< QPointF (0.0, -0.8 * z);
			_painter.rotate ((*_p.true_home_direction - *_p.heading_true).in<si::Degree>());
			_painter.paint (_c.black_shadow, [&] {
				_painter.drawPolyline (home_arrow);
			});
		}
	}

	// Height/VLOS distance/ground distance:
	if (_p.dist_to_home_ground || _p.dist_to_home_vlos || _p.dist_to_home_vert)
	{
		auto const font_a = _aids.font_2.font;
		auto const font_b = _aids.font_3.font;
		float const z = 0.99f * _c.q;
		float const h = 0.66f * _c.q;
		QPolygonF distance_triangle = QPolygonF()
			<< QPointF (z, 0.f)
			<< QPointF (0.f, 0.f)
			<< QPointF (z, -h);

		xf::TextLayout layout;
		layout.set_background (Qt::black, { _c.hmargin, 0.0 });
		layout.set_alignment (Qt::AlignRight);

		std::string vert_str = "---";
		if (_p.dist_to_home_vert)
			vert_str = (boost::format ("%+d") % static_cast<int> (_p.dist_to_home_vert->in<si::Foot>())).str();
		layout.add_fragment ("↑", font_b, Qt::gray);
		layout.add_fragment (vert_str, font_b, Qt::white);
		layout.add_fragment ("FT", font_a, xf::InstrumentAids::kCyan);
		layout.add_new_line();

		QString vlos_str = "---";
		if (_p.dist_to_home_vlos)
			vlos_str = QString ("%1").arg (_p.dist_to_home_vlos->in<si::NauticalMile>(), 0, 'f', 2, QChar ('0'));
		layout.add_fragment ("VLOS ", font_a, xf::InstrumentAids::kCyan);
		layout.add_fragment (vlos_str, font_b, Qt::white);
		layout.add_fragment ("NM", font_a, xf::InstrumentAids::kCyan);
		layout.add_new_line();

		QString ground_str = "---";
		if (_p.dist_to_home_ground)
			ground_str = QString ("%1").arg (_p.dist_to_home_ground->in<si::NauticalMile>(), 0, 'f', 2, QChar ('0'));
		layout.add_fragment (ground_str, font_b, Qt::white);
		layout.add_fragment ("NM", font_a, xf::InstrumentAids::kCyan);

		_painter.setTransform (base_transform);
		layout.paint (QPointF (0.0, 0.0), Qt::AlignRight | Qt::AlignBottom, _painter);
	}
}


void
PaintingWork::paint_course()
{
	using std::abs;

	if (!_p.heading_magnetic || !_p.heading_true || !_p.course_setting_magnetic || !_p.course_visible ||
		!_course_heading || !_rotation)
	{
		return;
	}

	_painter.setTransform (_c.aircraft_center_transform);
	_painter.setClipPath (_c.outer_map_clip);
	_painter.setTransform (_c.aircraft_center_transform);
	_painter.rotate ((*_course_heading - *_rotation).in<si::Degree>());

	float k = 1.0;
	float z = 1.0;
	float pink_pen_width = 1.5f;
	float shadow_pen_width = 2.5;
	QFont font = _aids.font_5.font;

	switch (_p.display_mode)
	{
		case hsi::DisplayMode::Expanded:
			k = _c.r / 15.0;
			z = _c.q / 6.0;
			break;

		case hsi::DisplayMode::Rose:
			k = _c.r / 10.0;
			z = _c.q / 7.0;
			break;

		case hsi::DisplayMode::Auxiliary:
			k = _c.r / 10.0;
			z = _c.q / 7.0;
			pink_pen_width = 1.2f;
			shadow_pen_width = 2.2f;
			font = _aids.font_3.font;
			break;
	}

	float const shadow_scale = shadow_pen_width / pink_pen_width;
	float const dev_1_deg_px = 1.5f * k;

	_painter.paint (_c.black_shadow, [&] {
		_painter.drawLine (QPointF (0.0, -3.5 * k), QPointF (0.0, -0.99 * _c.r));
	});

	// Back pink line:
	QPen back_pink_pen = _aids.get_pen (xf::InstrumentAids::kAutopilotColor, pink_pen_width, Qt::DashLine);
	back_pink_pen.setDashPattern (QVector<qreal>() << 7.5 << 12);

	QPen back_shadow_pen = _aids.get_pen (_c.black_shadow.color(), shadow_pen_width);
	back_shadow_pen.setDashPattern (QVector<qreal>() << 7.5 / shadow_scale << 12 / shadow_scale);

	for (auto const& p: { back_shadow_pen, back_pink_pen })
	{
		_painter.setPen (p);
		_painter.drawLine (QPointF (0.0, 3.5 * k - z), QPointF (0.0, 0.99 * _c.r));
	}

	// White bars:
	_painter.setPen (_aids.get_pen (Qt::white, 1.2f));
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
	_painter.paint (_c.black_shadow, [&] {
		_painter.drawPolyline (top_bar);
		_painter.drawPolyline (bottom_bar);
	});

	// Deviation bar:
	if (_p.course_deviation)
	{
		bool filled = false;
		si::Angle deviation = xf::clamped<si::Angle> (*_p.course_deviation, -2.5_deg, +2.5_deg);

		if (abs (*_p.course_deviation) <= abs (deviation))
			filled = true;

		float const pw = _aids.pen_width (1.75f);
		QRectF bar (-z, -2.5f * k + pw, 2.f * z, 5.f * k - 2.f * pw);
		bar.translate (dev_1_deg_px * deviation.in<si::Degree>(), 0.f);

		_painter.setPen (_aids.get_pen (Qt::black, 2.0));
		_painter.setBrush (Qt::NoBrush);
		_painter.drawRect (bar);

		_painter.setPen (_aids.get_pen (xf::InstrumentAids::kAutopilotColor, 1.0));

		if (filled)
			_painter.setBrush (xf::InstrumentAids::kAutopilotColor);
		else
			_painter.setBrush (Qt::NoBrush);

		_painter.drawRect (bar);
	}

	// Deviation scale:
	QRectF elli (0.f, 0.f, 0.25f * _c.q, 0.25f * _c.q);
	elli.translate (-elli.width() / 2.f, -elli.height() / 2.f);

	_painter.setPen (_aids.get_pen (Qt::white, 2.f));
	_painter.setBrush (Qt::NoBrush);
	_painter.paint (_c.black_shadow, [&] {
		for (float x: { -2.f, -1.f, +1.f, +2.f })
			_painter.drawEllipse (elli.translated (dev_1_deg_px * x, 0.f));
	});

	// TO/FROM flag - always on the right, regardless of rotation.
	if (_p.course_to_flag)
	{
		QString text = *_p.course_to_flag ? "TO" : "FROM";
		Qt::Alignment flags = Qt::AlignLeft | Qt::AlignVCenter;
		QPointF position (4.0f * k, 0.f);

		_painter.setTransform (_c.aircraft_center_transform);
		_painter.setPen (_aids.get_pen (Qt::white, 1.f));
		_painter.setFont (font);
		_painter.fast_draw_text (position, flags, text);
	}
}


void
PaintingWork::paint_selected_navaid_info()
{
	if (!_navaid_selected_visible)
		return;

	_painter.resetTransform();
	_painter.setClipping (false);

	std::string course_str = "/---°";
	if (_p.navaid_selected_course_magnetic)
	{
		int course_int = xf::symmetric_round (_p.navaid_selected_course_magnetic->in<si::Degree>());
		if (course_int == 0)
			course_int = 360;
		course_str = (boost::format ("/%03d°") % course_int).str();
	}

	std::string eta_min = "--";
	std::string eta_sec = "--";
	if (_p.navaid_selected_eta)
	{
		int s_int = _p.navaid_selected_eta->in<si::Second>();
		eta_min = (boost::format ("%02d") % (s_int / 60)).str();
		eta_sec = (boost::format ("%02d") % (s_int % 60)).str();
	}

	std::string distance_str = "---";
	if (_p.navaid_selected_distance)
		distance_str = (boost::format ("%3.1f") % _p.navaid_selected_distance->in<si::NauticalMile>()).str();

	xf::TextLayout layout;
	layout.set_background (Qt::black, { _c.hmargin, 0.0 });
	layout.set_background_mode (xf::TextLayout::PerLine);
	layout.set_alignment (Qt::AlignRight);

	auto const font_a = _aids.font_0;
	auto const font_b = _aids.font_2;
	auto const font_c = _aids.font_4;

	// If reference name is not empty, format is:
	//   <reference:green> <identifier>/<course>°
	// Otherwise:
	//   <identifier:magenta>/<course>°
	if (!_p.navaid_selected_reference.isEmpty())
	{
		layout.add_fragment (_p.navaid_selected_reference, font_c, Qt::green);
		layout.add_fragment (" ", font_a, Qt::white);
		layout.add_fragment (_p.navaid_selected_identifier, font_c, Qt::white);
	}
	else
		layout.add_fragment (_p.navaid_selected_identifier, font_c, xf::InstrumentAids::kAutopilotColor);

	layout.add_fragment (course_str, font_b, Qt::white);
	layout.add_new_line();
	layout.add_fragment ("ETA ", font_b, Qt::white);
	layout.add_fragment (eta_min, font_c, Qt::white);
	layout.add_fragment ("M", font_b, Qt::white);
	layout.add_fragment (eta_sec, font_c, Qt::white);
	layout.add_fragment ("S", font_b, Qt::white);
	layout.add_new_line();
	layout.add_fragment (distance_str, font_c, Qt::white);
	layout.add_fragment ("NM", font_b, Qt::white);
	layout.paint (QPointF (_aids.width() - _c.hmargin, 0.0), Qt::AlignTop | Qt::AlignRight, _painter);
}


void
PaintingWork::paint_tcas_and_navaid_info()
{
	auto const font_a = _aids.font_2;
	auto const font_b = _aids.font_3;

	_painter.resetTransform();
	_painter.setClipping (false);

	auto configure_layout = [&](xf::TextLayout& layout, QColor const& color, QString const& reference, QString const& identifier, std::optional<si::Length> const& distance) -> void
	{
		if (!reference.isEmpty())
			layout.add_fragment (reference, font_b, color);
		layout.add_skips (font_b, 1);
		layout.add_fragment (identifier.isEmpty() ? "---" : identifier, font_b, color);
		layout.add_new_line();
		layout.add_fragment ("DME ", font_a, color);
		layout.add_fragment (distance ? (boost::format ("%.1f") % distance->in<si::NauticalMile>()).str() : std::string ("---"), font_b, color);
	};

	xf::TextLayout left_layout;
	left_layout.set_alignment (Qt::AlignLeft);
	left_layout.set_background (Qt::black, { _c.hmargin, 0.0 });

	if (_p.loc_visible)
		left_layout.add_fragment ("LOC", font_a, xf::InstrumentAids::kCyan);
	left_layout.add_skips (font_a, 1);

	if (_p.arpt_visible)
		left_layout.add_fragment ("ARPT", font_a, xf::InstrumentAids::kCyan);
	left_layout.add_skips (font_a, 1);

	if (_p.fix_visible)
		left_layout.add_fragment ("WPT", font_a, xf::InstrumentAids::kCyan);
	left_layout.add_skips (font_a, 1);

	if (_p.vor_visible || _p.dme_visible || _p.ndb_visible)
		left_layout.add_fragment ("STA", font_a, xf::InstrumentAids::kCyan);
	left_layout.add_skips (font_a, 2);

	if (_p.tcas_on && !*_p.tcas_on)
	{
		left_layout.add_fragment ("TCAS", font_b, xf::InstrumentAids::kCautionColor);
		left_layout.add_new_line();
		left_layout.add_fragment ("OFF", font_b, xf::InstrumentAids::kCautionColor);
		left_layout.add_new_line();
	}
	else
		left_layout.add_skips (font_b, 2);

	if (_navaid_left_visible)
		configure_layout (left_layout, (_p.navaid_left_type == hsi::NavType::A) ? Qt::green : xf::InstrumentAids::kCyan, _p.navaid_left_reference, _p.navaid_left_identifier, _p.navaid_left_distance);
	else
		left_layout.add_skips (font_b, 2);

	xf::TextLayout right_layout;
	right_layout.set_alignment (Qt::AlignRight);
	right_layout.set_background (Qt::black, { _c.hmargin, 0.0 });

	if (_navaid_right_visible)
		configure_layout (right_layout, (_p.navaid_right_type == hsi::NavType::A) ? Qt::green : xf::InstrumentAids::kCyan, _p.navaid_right_reference, _p.navaid_right_identifier, _p.navaid_right_distance);

	auto const size = _paint_request.metric().canvas_size();

	left_layout.paint (QPointF (_c.hmargin, size.height() - _c.vmargin), Qt::AlignBottom | Qt::AlignLeft, _painter);
	right_layout.paint (QPointF (size.width() -_c.hmargin, size.height() - _c.vmargin), Qt::AlignBottom | Qt::AlignRight, _painter);
}


void
PaintingWork::paint_pointers()
{
	if (!_p.heading_magnetic || !_p.heading_true)
		return;

	_painter.resetTransform();
	_painter.setClipping (false);

	struct Opts
	{
		bool						is_primary;
		QColor						color;
		std::optional<si::Angle>	angle;
		bool						visible;
	};

	for (Opts const& opts: { Opts { true, (_p.navaid_left_type == hsi::NavType::A ? Qt::green : xf::InstrumentAids::kCyan), _p.navaid_left_initial_bearing_magnetic, _navaid_left_visible },
							 Opts { false, (_p.navaid_right_type == hsi::NavType::A ? Qt::green : xf::InstrumentAids::kCyan), _p.navaid_right_initial_bearing_magnetic, _navaid_right_visible } })
	{
		if (!opts.angle || !opts.visible)
			continue;

		float width = 1.5f;

		if (_p.display_mode == hsi::DisplayMode::Auxiliary)
			width = 1.2f;

		_painter.setPen (_aids.get_pen (opts.color, width));
		_painter.setTransform (_c.aircraft_center_transform);
		_painter.setClipRect (_c.map_clip_rect);
		_painter.setTransform (_pointers_transform * _c.aircraft_center_transform);
		_painter.rotate (opts.angle->in<si::Degree>());

		if (opts.is_primary)
		{
			float const z = 0.13 * _c.q;
			float const delta = 0.5 * z;

			float const to_top = -_c.r - 3.0 * z;
			float const to_bottom = -_c.r + 12.0 * z;

			float const from_top = _c.r - 11.0 * z;
			float const from_bottom = _c.r + 3.0 * z;

			_painter.paint (_c.black_shadow, [&] {
				_painter.drawLine (QPointF (0.0, to_top + delta), QPointF (0.0, to_bottom));
				_painter.drawLine (QPointF (0.0, to_top), QPointF (+z, to_top + 1.4 * z));
				_painter.drawLine (QPointF (0.0, to_top), QPointF (-z, to_top + 1.4 * z));
				_painter.drawLine (QPointF (-2.0 * z, to_bottom - 0.5 * z), QPointF (+2.0 * z, to_bottom - 0.5 * z));

				_painter.drawLine (QPointF (0.0, from_top), QPointF (0.0, from_bottom));
				_painter.drawLine (QPointF (-2.0 * z, from_bottom - 1.2 * z), QPointF (0.0, from_bottom - 2.05 * z));
				_painter.drawLine (QPointF (+2.0 * z, from_bottom - 1.2 * z), QPointF (0.0, from_bottom - 2.05 * z));
			});
		}
		else
		{
			float const z = 0.13 * _c.q;

			float const to_top = -_c.r - 3.0 * z;
			float const to_bottom = -_c.r + 10.7 * z;
			QPolygonF const top_arrow = QPolygonF()
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

			float const from_top = _c.r - 12.0 * z;
			float const from_bottom = _c.r + 0.3 * z;
			QPolygonF const bottom_arrow = QPolygonF()
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

			_painter.paint (_c.black_shadow, [&] {
				_painter.drawPolyline (top_arrow);
				_painter.drawPolyline (bottom_arrow);
			});
		}
	}
}


void
PaintingWork::paint_range()
{
	if (_p.display_mode == hsi::DisplayMode::Expanded || _p.display_mode == hsi::DisplayMode::Rose)
	{
		QFont font_a = _aids.scaled_default_font (1.1f);
		QFont font_b = _aids.font_3.font;
		QFontMetricsF metr_a (font_a);
		QFontMetricsF metr_b (font_b);
		QString s ("RANGE");
		QString r;

		if (_p.range < 1_nmi)
			r = QString::fromStdString ((boost::format ("%.1f") % _p.range.in<si::NauticalMile>()).str());
		else
			r = QString::fromStdString ((boost::format ("%d") % _p.range.in<si::NauticalMile>()).str());

		QRectF rect (0.f, 0.f, std::max (metr_a.width (s), metr_b.width (r)) + 0.4f * _c.q, metr_a.height() + metr_b.height());

		_painter.setClipping (false);
		_painter.resetTransform();
		_painter.translate (5.5f * _c.q, 0.25f * _c.q);
		_painter.setPen (_aids.get_pen (Qt::white, 1.0f));
		_painter.setBrush (Qt::black);
		_painter.drawRect (rect);
		_painter.setFont (font_a);
		_painter.fast_draw_text (rect.center() - QPointF (0.f, 0.05f * _c.q), Qt::AlignBottom | Qt::AlignHCenter, s);
		_painter.setFont (font_b);
		_painter.fast_draw_text (rect.center() - QPointF (0.f, 0.135f * _c.q), Qt::AlignTop | Qt::AlignHCenter, r);
	}
}


void
PaintingWork::paint_navaids()
{
	if (!_p.navaids_visible || !_p.position)
		return;

	float scale = 0.55f * _c.q;

	_painter.setTransform (_c.aircraft_center_transform);
	_painter.setClipPath (_c.outer_map_clip);
	_painter.setFont (_aids.font_1.font);

	retrieve_navaids();
	paint_locs();

	// Return feature position on screen relative to _c.aircraft_center_transform.
	// Essentially does get_feature_xy() but it may additionally "limit-to-range" (which is used by eg. Home feature)
	// to be drawn on the edge even if it so far that it shouldn't be visible at all).
	auto position_feature = [&] (si::LonLat const& position, bool* limit_to_range = nullptr) -> QPointF
	{
		QPointF mapped_pos = get_feature_xy (position);

		if (limit_to_range)
		{
			float const range = 0.95f * _c.r;
			float const rpx = std::sqrt (mapped_pos.x() * mapped_pos.x() + mapped_pos.y() * mapped_pos.y());
			*limit_to_range = rpx >= range;
			if (*limit_to_range)
			{
				QTransform rot;
				rot.rotate ((1_rad * std::atan2 (mapped_pos.y(), mapped_pos.x())).in<si::Degree>());
				mapped_pos = rot.map (QPointF (range, 0.0));
			}
		}

		return mapped_pos;
	};

	auto paint_navaid = [&](xf::Navaid const& navaid)
	{
		QTransform feature_centered_transform = _c.aircraft_center_transform;
		QPointF translation = position_feature (navaid.position());
		feature_centered_transform.translate (translation.x(), translation.y());

		QTransform feature_scaled_transform = feature_centered_transform;
		feature_scaled_transform.scale (scale, scale);

		switch (navaid.type())
		{
			case xf::Navaid::NDB:
			{
				_painter.setTransform (feature_scaled_transform);
				_painter.setPen (_c.ndb_pen);
				_painter.setBrush (_c.ndb_pen.color());
				_painter.drawEllipse (QRectF (-0.1f, -0.1f, 0.2f, 0.2f));
				_painter.setTransform (feature_centered_transform);
				_painter.fast_draw_text (QPointF (0.15 * _c.q, 0.10 * _c.q), Qt::AlignLeft | Qt::AlignTop, navaid.identifier());
				break;
			}

			case xf::Navaid::VOR:
				_painter.setTransform (feature_scaled_transform);
				_painter.setPen (_c.vor_pen);
				_painter.setBrush (xf::InstrumentAids::kNavigationColor);

				if (navaid.vor_type() == xf::Navaid::VOROnly)
				{
					_painter.drawEllipse (QRectF (-0.07f, -0.07f, 0.14f, 0.14f));
					_painter.drawPolyline (_c.vor_shape);
				}
				else if (navaid.vor_type() == xf::Navaid::VOR_DME)
				{
					_painter.drawEllipse (QRectF (-0.07f, -0.07f, 0.14f, 0.14f));
					_painter.drawPolyline (_c.vor_shape);
					_painter.drawPolyline (_c.dme_for_vor_shape);
				}
				else if (navaid.vor_type() == xf::Navaid::VORTAC)
					_painter.drawPolyline (_c.vortac_shape);

				_painter.setTransform (feature_centered_transform);
				_painter.fast_draw_text (QPointF (0.35f * _c.q, 0.55f * _c.q), navaid.identifier());
				break;

			case xf::Navaid::DME:
				_painter.setTransform (feature_scaled_transform);
				_painter.setPen (_c.dme_pen);
				_painter.drawRect (QRectF (-0.5f, -0.5f, 1.f, 1.f));
				break;

			case xf::Navaid::FIX:
			{
				float const h = 0.75f;
				QPointF a (0.f, -0.66f * h);
				QPointF b (+0.5f * h, +0.33f * h);
				QPointF c (-0.5f * h, +0.33f * h);
				QPointF points[] = { a, b, c, a };

				_painter.setTransform (feature_scaled_transform);
				_painter.setPen (_c.fix_pen);
				_painter.drawPolyline (points, sizeof (points) / sizeof (*points));
				_painter.setTransform (feature_centered_transform);
				_painter.translate (0.5f, 0.5f);
				_painter.fast_draw_text (QPointF (0.25f * _c.q, 0.45f * _c.q), navaid.identifier());
				break;
			}

			case xf::Navaid::ARPT:
			{
				if (_p.range > _p.arpt_runways_range_threshold)
				{
					// Draw circles for airports:
					float const v = 1.1f;
					_painter.setTransform (feature_scaled_transform);
					_painter.setPen (_c.arpt_pen);
					_painter.setBrush (Qt::NoBrush);
					_painter.drawEllipse (QRectF (QPointF (-0.5 * v, -0.5 * v), QSizeF (1.0 * v, 1.0 * v)));
					// Label:
					_painter.setTransform (feature_centered_transform);
					_painter.fast_draw_text (QPointF (0.46 * scale, 0.46 * scale), Qt::AlignTop | Qt::AlignLeft, navaid.identifier());
				}
				else if (_p.range > _p.arpt_map_range_threshold)
				{
					// Draw airport runways:
					for (xf::Navaid::Runway const& runway: navaid.runways())
					{
						// Make the drawn runway somewhat more wide:
						float const half_width = 1.5f * to_px (runway.width());
						QTransform tr_l; tr_l.translate (-half_width, 0.0);
						QTransform tr_r; tr_r.translate (+half_width, 0.0);
						// Find runway's true bearing from pos_1 to pos_2 and runway
						// length in pixels:
						si::Angle true_bearing = xf::initial_bearing (runway.pos_1(), runway.pos_2());
						float const length_px = to_px (xf::haversine_earth (runway.pos_1(), runway.pos_2()));
						float const extended_length_px = to_px (_p.arpt_runway_extension_length);
						// Create transform so that the first end of the runway
						// is at (0, 0) and runway extends to the top.
						QPointF point_1 = get_feature_xy (runway.pos_1());
						QTransform transform = _c.aircraft_center_transform;
						transform.translate (point_1.x(), point_1.y());
						transform = _features_transform * transform;
						transform.rotate (true_bearing.in<si::Degree>());

						_painter.setTransform (transform);
						// The runway:
						_painter.setPen (_aids.get_pen (Qt::white, 1.0));
						_painter.drawLine (tr_l.map (QPointF (0.0, 0.0)), tr_l.map (QPointF (0.0, -length_px)));
						_painter.drawLine (tr_r.map (QPointF (0.0, 0.0)), tr_r.map (QPointF (0.0, -length_px)));
						// Extended runway:
						float const m_px = xf::clamped<float> (to_px (1_m), 0.02f, 0.04f);
						QPen dashed_pen = _aids.get_pen (Qt::white, 1.0, Qt::DashLine);
						dashed_pen.setDashPattern (QVector<qreal>() << 300 * m_px << 200 * m_px);
						_painter.setPen (dashed_pen);
						_painter.drawLine (QPointF (0.0, 0.0), QPointF (0.0, extended_length_px));
						_painter.drawLine (QPointF (0.0, -length_px), QPointF (0.0, -length_px - extended_length_px));
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

	if (_p.fix_visible)
		for (auto const& navaid: _current_navaids.fix_navs)
			paint_navaid (navaid);

	if (_p.ndb_visible)
		for (auto const& navaid: _current_navaids.ndb_navs)
			paint_navaid (navaid);

	if (_p.dme_visible)
		for (auto const& navaid: _current_navaids.dme_navs)
			paint_navaid (navaid);

	if (_p.vor_visible)
		for (auto const& navaid: _current_navaids.vor_navs)
			paint_navaid (navaid);

	if (_p.arpt_visible)
		for (auto const& navaid: _current_navaids.arpt_navs)
			paint_navaid (navaid);

	if (_p.home)
	{
		// Whether the feature is in configured HSI range:
		bool outside_range = false;
		QPointF translation = position_feature (*_p.home, &outside_range);
		QTransform feature_centered_transform = _c.aircraft_center_transform;
		feature_centered_transform.translate (translation.x(), translation.y());

		// Line from aircraft to the HOME feature:
		if (_p.home_track_visible)
		{
			float green_pen_width = 1.5f;
			float shadow_pen_width = 2.5f;

			if (_p.display_mode == hsi::DisplayMode::Auxiliary)
			{
				green_pen_width = 1.2f;
				shadow_pen_width = 2.2f;
			}

			float const shadow_scale = shadow_pen_width / green_pen_width;

			QPen home_line_pen = _aids.get_pen (_c.home_pen.color(), green_pen_width, Qt::DashLine, Qt::RoundCap);
			home_line_pen.setDashPattern (QVector<qreal>() << 7.5 << 12);

			QPen shadow_pen = _aids.get_pen (_c.black_shadow.color(), shadow_pen_width, Qt::DashLine, Qt::RoundCap);
			shadow_pen.setDashPattern (QVector<qreal>() << 7.5 / shadow_scale << 12 / shadow_scale);

			_painter.setTransform (_c.aircraft_center_transform);

			for (auto const& p: { shadow_pen, home_line_pen })
			{
				_painter.setPen (p);
				_painter.drawLine (QPointF (0.f, 0.f), translation);
			}
		}

		_painter.setTransform (feature_centered_transform);
		_painter.scale (scale, scale);

		if (outside_range)
		{
			_painter.setPen (_c.home_pen);
			_painter.setBrush (Qt::black);
			_painter.drawPolygon (_c.home_shape);
		}
		else
		{
			_painter.setPen (_c.home_pen);
			_painter.setBrush (_c.home_pen.color());
			_painter.drawPolygon (_c.home_shape);
		}
	}
}


void
PaintingWork::paint_radio_range_map()
{
	if (_p.radio_position)
	{
		auto const scale = _p.radio_range_pattern_scale;
		auto const source_rect = QRectF (_c.radio_range_heat_map.rect());
		auto target_rect = source_rect;
		target_rect.setRight (target_rect.right() * scale);
		target_rect.setBottom (target_rect.bottom() * scale);
		target_rect.moveCenter (get_feature_xy (*_p.radio_position));

		_painter.setTransform (_c.aircraft_center_transform);
		_painter.setClipPath (_c.outer_map_clip);
		_painter.drawImage (target_rect, _c.radio_range_heat_map, source_rect);
	}
}


void
PaintingWork::paint_flight_ranges()
{
	if (_p.flight_range_warning)
		paint_circle (*_p.flight_range_warning, QColor (0xff, 0xaa, 0x00));

	if (_p.flight_range_critical)
		paint_circle (*_p.flight_range_critical, QColor (0xff, 0x00, 0x00));
}


void
PaintingWork::paint_circle (CircularArea const& area, QColor const color)
{
	QPointF const center_pos = get_feature_xy (area.center);
	auto const radius_px = to_px (area.radius);
	QPointF const radius_vect (radius_px, radius_px);

	_painter.setTransform (_c.aircraft_center_transform);
	_painter.setClipPath (_c.outer_map_clip);
	auto pen = _aids.get_pen (color, 1.0);
	pen.setDashPattern (QVector<qreal>() << 5 << 3);
	_painter.setPen (pen);
	_painter.setBrush (Qt::NoBrush);
	_painter.paint (_c.black_shadow, [&]() {
		_painter.drawEllipse (QRectF (center_pos - radius_vect, center_pos + radius_vect));
	});
}


void
PaintingWork::paint_locs()
{
	if (!_p.loc_visible)
		return;

	QFontMetricsF font_metrics (_painter.font());
	QTransform rot_1; rot_1.rotate (-2.f);
	QTransform rot_2; rot_2.rotate (+2.f);
	QPointF zero (0.f, 0.f);

	// Group painting lines and texts as separate tasks. For this,
	// cache texts that need to be drawn later along with their positions.
	std::vector<std::pair<QPointF, QString>> texts_to_paint;
	texts_to_paint.reserve (128);

	auto paint_texts_to_paint = [&]() -> void
	{
		_painter.resetTransform();

		for (auto const& text_and_xy: texts_to_paint)
			_painter.fast_draw_text (text_and_xy.first, text_and_xy.second);

		texts_to_paint.clear();
	};

	auto paint_loc = [&] (xf::Navaid const& navaid) -> void
	{
		QPointF const navaid_pos = get_feature_xy (navaid.position());
		QTransform transform = _c.aircraft_center_transform;
		transform.translate (navaid_pos.x(), navaid_pos.y());
		transform = _features_transform * transform;
		transform.rotate (navaid.true_bearing().in<si::Degree>());

		float const line_1 = to_px (navaid.range());
		float const line_2 = 1.03f * line_1;

		QPointF const pt_0 (0.f, line_1);
		QPointF const pt_1 (rot_1.map (QPointF (0.f, line_2)));
		QPointF const pt_2 (rot_2.map (QPointF (0.f, line_2)));

		_painter.setTransform (transform);

		if (_p.range < 16_nmi)
			_painter.drawLine (zero, pt_0);

		_painter.drawLine (zero, pt_1);
		_painter.drawLine (zero, pt_2);
		_painter.drawLine (pt_0, pt_1);
		_painter.drawLine (pt_0, pt_2);

		QPointF text_offset (0.5f * font_metrics.width (navaid.identifier()), -0.35f * font_metrics.height());
		texts_to_paint.emplace_back (transform.map (pt_0 + QPointF (0.f, 0.6f * _c.q)) - text_offset, navaid.identifier());
	};

	// Paint localizers:
	_painter.setBrush (Qt::NoBrush);
	_painter.setPen (_c.lo_loc_pen);
	xf::Navaid const* hi_loc = nullptr;

	for (auto const& navaid: _current_navaids.loc_navs)
	{
		// Paint highlighted LOC at the end, so it's on top:
		if (navaid.identifier() == _p.highlighted_loc)
			hi_loc = &navaid;
		else
			paint_loc (navaid);
	}

	// Paint identifiers:
	paint_texts_to_paint();

	// Highlighted localizer with text:
	if (hi_loc)
	{
		_painter.setPen (_c.hi_loc_pen);
		paint_loc (*hi_loc);
		paint_texts_to_paint();
	}
}


void
PaintingWork::paint_tcas()
{
	if (!_p.tcas_on)
		return;

	_painter.setTransform (_c.aircraft_center_transform);
	_painter.setClipping (false);
	_painter.setPen (_aids.get_pen (Qt::white, 1.f));

	if (_p.tcas_range)
	{
		float const z = 0.075 * _c.q;
		float const v = 0.025 * _c.q;
		float const r = to_px (*_p.tcas_range);

		// Don't draw too small range points:
		if (r > 15.0f)
		{
			QRectF big_point (-z, -z, 2.0 * z, 2.0 * z);
			QRectF small_point (-v, -v, 2.0 * v, 2.0 * v);

			for (int angle = 0; angle < 360; angle += 30)
			{
				_painter.translate (0.0, r);

				if (angle % 90 == 0)
				{
					_painter.setBrush (Qt::NoBrush);
					_painter.paint (_c.black_shadow, [&] {
						_painter.drawEllipse (big_point);
					});
				}
				else
				{
					_painter.setBrush (Qt::white);
					_painter.paint (_c.black_shadow, [&] {
						_painter.drawEllipse (small_point);
					});
				}

				_painter.translate (0.0, -r);
				_painter.rotate (30);
			}
		}
	}
}


void
PaintingWork::update_radio_range_heat_map()
{
	if (_p.radio_range_warning && _p.radio_range_critical)
	{
		auto const yellow_start = *_p.radio_range_warning / *_p.radio_range_critical;
		auto const yellow_stop = 0.5 * (yellow_start + 1.0);
		auto const red_stop = 1.3;
		auto const black_stop = 1.6;
		auto const scale = _p.radio_range_pattern_scale;
		auto const max_range_px = to_px (black_stop * *_p.radio_range_critical);
		auto const canvas_size = QSize (2 * max_range_px, 2 * max_range_px) / scale;

		if (canvas_size.width() * canvas_size.height() > 100'000'000)
			_logger << "Radio-range heat map pixmap too big, not rendering.\n";

		auto canvas = QImage (canvas_size, QImage::Format_ARGB32_Premultiplied);

		{
			QVector<QGradientStop> palette {
				{ 0.00, Qt::black },
				{ yellow_start / black_stop, Qt::black },
				{ yellow_stop / black_stop, Qt::yellow },
				{ red_stop / black_stop, Qt::red },
				{ 1.00, Qt::black },
			};

			for (auto& stop: palette)
				stop.second = stop.second.darker (150);

			QRadialGradient gradient (canvas.rect().center(), max_range_px / scale);
			gradient.setStops (palette);

			QPainter canvas_painter (&canvas);
			canvas_painter.fillRect (canvas.rect(), QBrush (gradient));
			canvas_painter.fillRect (canvas.rect(), QBrush (Qt::black, Qt::Dense2Pattern));
		}

		_c.radio_range_heat_map = canvas;
	}
}


void
PaintingWork::retrieve_navaids()
{
	if (!_p.position)
		return;

	if (_current_navaids.retrieved && xf::haversine_earth (_current_navaids.retrieve_position, *_p.position) < 10_m && _p.range == _current_navaids.retrieve_range)
		return;

	_current_navaids.fix_navs.clear();
	_current_navaids.vor_navs.clear();
	_current_navaids.dme_navs.clear();
	_current_navaids.ndb_navs.clear();
	_current_navaids.loc_navs.clear();
	_current_navaids.arpt_navs.clear();

	for (xf::Navaid const& navaid: _navaid_storage.get_navs (*_p.position, std::max (_p.range + 20_nmi, 2.f * _p.range)))
	{
		switch (navaid.type())
		{
			case xf::Navaid::LOC:
				_current_navaids.loc_navs.push_back (navaid);
				break;

			case xf::Navaid::NDB:
				_current_navaids.ndb_navs.push_back (navaid);
				break;

			case xf::Navaid::VOR:
				_current_navaids.vor_navs.push_back (navaid);
				break;

			case xf::Navaid::DME:
				_current_navaids.dme_navs.push_back (navaid);
				break;

			case xf::Navaid::FIX:
				_current_navaids.fix_navs.push_back (navaid);
				break;

			case xf::Navaid::ARPT:
				_current_navaids.arpt_navs.push_back (navaid);
				break;

			default:
				// Other types not drawn.
				break;
		}
	}

	_current_navaids.retrieved = true;
	_current_navaids.retrieve_position = *_p.position;
	_current_navaids.retrieve_range = _p.range;
}


QPointF
PaintingWork::get_feature_xy (si::LonLat const& navaid_position) const
{
	if (!_p.position)
		return QPointF();

	QPointF navaid_pos = xf::kEarthMeanRadius.in<si::NauticalMile>() * navaid_position.rotated (*_p.position).project_flat();
	return _features_transform.map (QPointF (to_px (1_nmi * navaid_pos.x()), to_px (1_nmi * navaid_pos.y())));
}


si::Length
PaintingWork::actual_trend_range() const
{
	if (_p.ground_speed && _p.range <= _p.trend_vector_max_range)
	{
		si::Time time = 0_s;

		if (_p.range >= _p.trend_vector_min_ranges[2])
			time = _p.trend_vector_durations[2];
		else if (_p.range >= _p.trend_vector_min_ranges[1])
			time = _p.trend_vector_durations[1];
		else if (_p.range >= _p.trend_vector_min_ranges[0])
			time = _p.trend_vector_durations[0];

		return *_p.ground_speed * time;
	}
	else
		return 0_m;
}


si::Length
PaintingWork::trend_gap() const
{
	switch (_p.display_mode)
	{
		case hsi::DisplayMode::Expanded:
			return 0.015f * _p.range;
		case hsi::DisplayMode::Rose:
			return 0.030f * _p.range;
		case hsi::DisplayMode::Auxiliary:
			return 0.0375f * _p.range;
	}
	return 0_nmi;
}


si::Time
PaintingWork::trend_time_gap() const
{
	if (_p.ground_speed)
		return trend_gap() / *_p.ground_speed;
	else
		return 0_s;
}


float
PaintingWork::to_px (si::Length const length) const
{
	return length / _p.range * _c.r;
}

} // namespace hsi_detail


HSI::HSI (xf::Graphics const& graphics, xf::NavaidStorage const& navaid_storage, xf::Logger const& logger, std::string_view const& instance):
	HSI_IO (instance),
	_logger (logger.with_scope (std::string (kLoggerScope) + "#" + instance)),
	_navaid_storage (navaid_storage),
	_instrument_support (graphics)
{ }


void
HSI::process (xf::Cycle const& cycle)
{
	hsi_detail::Parameters params;
	params.update_time = cycle.update_time();
	params.display_mode = _io.display_mode.value_or (hsi::DisplayMode::Expanded);
	params.heading_mode = _io.heading_mode.value_or (hsi::HeadingMode::Magnetic);
	params.range = _io.range.value_or (5_nmi);
	params.heading_magnetic = _io.orientation_heading_magnetic.get_optional();
	params.heading_true = _io.orientation_heading_true.get_optional();
	params.ap_visible = _io.cmd_visible.value_or (false);
	params.ap_line_visible = _io.cmd_line_visible.value_or (false);
	params.ap_heading_magnetic = _io.cmd_heading_magnetic.get_optional();
	params.ap_track_magnetic = _io.cmd_track_magnetic.get_optional();
	params.ap_use_trk = _io.cmd_use_trk.get_optional();
	params.track_visible = _io.track_visible.value_or (false) && _io.track_lateral_magnetic;
	params.track_magnetic = _io.track_lateral_magnetic.get_optional();
	params.course_visible = _io.course_visible.value_or (false);
	params.course_setting_magnetic = _io.course_setting_magnetic.get_optional();
	params.course_deviation = _io.course_deviation.get_optional();
	params.course_to_flag = _io.course_to_flag.get_optional();
	params.navaid_selected_reference = QString::fromStdString (_io.navaid_selected_reference.value_or (""));
	params.navaid_selected_identifier = QString::fromStdString (_io.navaid_selected_identifier.value_or (""));
	params.navaid_selected_distance = _io.navaid_selected_distance.get_optional();
	params.navaid_selected_eta = _io.navaid_selected_eta.get_optional();
	params.navaid_selected_course_magnetic = _io.navaid_selected_course_magnetic.get_optional();
	params.navaid_left_reference = QString::fromStdString (_io.navaid_left_reference.value_or (""));
	params.navaid_left_type = _io.navaid_left_type.value_or (hsi::NavType::A);
	params.navaid_left_identifier = QString::fromStdString (_io.navaid_left_identifier.value_or (""));
	params.navaid_left_distance = _io.navaid_left_distance.get_optional();
	params.navaid_left_initial_bearing_magnetic = _io.navaid_left_initial_bearing_magnetic.get_optional();
	params.navaid_right_type = _io.navaid_right_type.value_or (hsi::NavType::A);
	params.navaid_right_reference = QString::fromStdString (_io.navaid_right_reference.value_or (""));
	params.navaid_right_identifier = QString::fromStdString (_io.navaid_right_identifier.value_or (""));
	params.navaid_right_distance = _io.navaid_right_distance.get_optional();
	params.navaid_right_initial_bearing_magnetic = _io.navaid_right_initial_bearing_magnetic.get_optional();
	params.navigation_required_performance = _io.navigation_required_performance.get_optional();
	params.navigation_actual_performance = _io.navigation_actual_performance.get_optional();
	params.center_on_track = _io.track_center_on_track.value_or (true);
	params.home_track_visible = _io.home_track_visible.value_or (false);
	params.true_home_direction = _io.home_true_direction.get_optional();
	params.dist_to_home_ground = _io.home_distance_ground.get_optional();
	params.dist_to_home_vlos = _io.home_distance_vlos.get_optional();
	params.dist_to_home_vert = _io.home_distance_vertical.get_optional();

	if (_io.home_position_longitude && _io.home_position_latitude)
		params.home = si::LonLat (*_io.home_position_longitude, *_io.home_position_latitude);
	else
		params.home.reset();

	params.ground_speed = _io.speed_gs.get_optional();
	params.true_air_speed = _io.speed_tas.get_optional();
	params.track_lateral_rotation = _io.track_lateral_rotation.get_optional();

	if (params.track_lateral_rotation)
		params.track_lateral_rotation = xf::clamped<si::AngularVelocity> (*params.track_lateral_rotation, si::convert (-1_Hz), si::convert (+1_Hz));

	params.altitude_reach_distance = _io.target_altitude_reach_distance.get_optional();
	params.wind_from_magnetic_heading = _io.wind_from_magnetic.get_optional();
	params.wind_tas_speed = _io.wind_speed_tas.get_optional();

	if (_io.position_longitude && _io.position_latitude)
		params.position = si::LonLat (*_io.position_longitude, *_io.position_latitude);
	else
		params.position.reset();

	params.navaids_visible = _io.orientation_heading_true.valid();
	params.fix_visible = _io.features_fix.value_or (false);
	params.vor_visible = _io.features_vor.value_or (false);
	params.dme_visible = _io.features_dme.value_or (false);
	params.ndb_visible = _io.features_ndb.value_or (false);
	params.loc_visible = _io.features_loc.value_or (false);
	params.arpt_visible = _io.features_arpt.value_or (false);
	params.highlighted_loc = QString::fromStdString (_io.localizer_id.value_or (""));
	params.positioning_hint.set (_io.position_source ? std::make_optional (QString::fromStdString (*_io.position_source)) : std::nullopt, _io.position_source.modification_timestamp());
	params.tcas_on = _io.tcas_on.get_optional();
	params.tcas_range = _io.tcas_range.get_optional();
	params.arpt_runways_range_threshold = *_io.arpt_runways_range_threshold;
	params.arpt_map_range_threshold = *_io.arpt_map_range_threshold;
	params.arpt_runway_extension_length = *_io.arpt_runway_extension_length;
	params.trend_vector_durations = *_io.trend_vector_durations;
	params.trend_vector_min_ranges = *_io.trend_vector_min_ranges;
	params.trend_vector_max_range = *_io.trend_vector_max_range;
	params.radio_range_pattern_scale = *_io.radio_range_pattern_scale;
	params.round_clip = false;

	if (_io.flight_range_warning_longitude && _io.flight_range_warning_latitude && _io.flight_range_warning_radius)
	{
		params.flight_range_warning = {
			si::LonLat (*_io.flight_range_warning_longitude, *_io.flight_range_warning_latitude),
			*_io.flight_range_warning_radius,
		};
	}
	else
		params.flight_range_warning.reset();

	if (_io.flight_range_critical_longitude && _io.flight_range_critical_latitude && _io.flight_range_critical_radius)
	{
		params.flight_range_critical = {
			si::LonLat (*_io.flight_range_critical_longitude, *_io.flight_range_critical_latitude),
			*_io.flight_range_critical_radius,
		};
	}
	else
		params.flight_range_critical.reset();

	if (_io.radio_position_longitude && _io.radio_position_latitude)
		params.radio_position = si::LonLat (*_io.radio_position_longitude, *_io.radio_position_latitude);
	else
		params.radio_position.reset();

	if (params.radio_position)
	{
		params.radio_range_warning = _io.radio_range_warning.get_optional();
		params.radio_range_critical = _io.radio_range_critical.get_optional();
	}

	*_parameters.lock() = params;
	mark_dirty();
}


std::packaged_task<void()>
HSI::paint (xf::PaintRequest paint_request) const
{
	auto parameters = *_parameters.lock();
	auto current_navaids = *_current_navaids.lock();
	auto mutable_lock = _mutable.lock();
	auto resize_cache_lock = _resize_cache.lock();

	parameters.sanitize();

	return std::packaged_task<void()> ([this, pr = std::move (paint_request), pp = parameters, rc_lock = std::move (resize_cache_lock), cn = current_navaids, mu_lock = std::move (mutable_lock)]() mutable {
		hsi_detail::PaintingWork (pr, _instrument_support, _navaid_storage, pp, *rc_lock, cn, *mu_lock, _logger).paint();
	});
}


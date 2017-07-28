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
#include <cmath>

// Lib:
#include <boost/format.hpp>

// Qt:
#include <QtCore/QTimer>
#include <QtGui/QPainter>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v1/window.h>
#include <xefis/utility/numeric.h>
#include <xefis/utility/painter.h>
#include <xefis/utility/text_layout.h>

// Local:
#include "adi_widget.h"


void
ADIWidget::Parameters::sanitize()
{
	sl_line_every = std::max (sl_line_every, 1);
	sl_number_every = std::max (sl_number_every, 1);
	sl_extent = std::max<si::Velocity> (sl_extent, 1_kt);
	sl_minimum = std::max (sl_minimum, 0);
	sl_maximum = std::min (sl_maximum, 9999);
	al_line_every = std::max (al_line_every, 1);
	al_number_every = std::max (al_number_every, 1);
	al_emphasis_every = std::max (al_emphasis_every, 1);
	al_bold_every = std::max (al_bold_every, 1);
	al_extent = std::max<Length> (al_extent, 1_ft);
}


ADIWidget::PaintWorkUnit::PaintWorkUnit (ADIWidget* adi_widget):
	InstrumentWidget::PaintWorkUnit (adi_widget),
	InstrumentAids (0.8f)
{
	_sky_color.setHsv (213, 230, 255);
	_sky_shadow = _sky_color.darker (400);
	_sky_shadow.setAlpha (127);
	_ground_color.setHsv (34, 255, 125);
	_ground_shadow = _ground_color.darker (400);
	_ground_shadow.setAlpha (127);
	_ladder_color = QColor (64, 51, 108, 0x80);
	_ladder_border_color = _ladder_color.darker (120);
}


void
ADIWidget::PaintWorkUnit::pop_params()
{
	_params = _params_next;
	_locals = _locals_next;
}


void
ADIWidget::PaintWorkUnit::resized()
{
	InstrumentAids::update_sizes (size(), window_size());

	_w = size().width();
	_h = size().height();
	_max_w_h = std::max (_w, _h);
	_q = 0.1f * wh();

	_center_transform.reset();
	_center_transform.translate (0.5f * _w, 0.5f * _h);

	adi_post_resize();
	sl_post_resize();
	al_post_resize();
}


void
ADIWidget::PaintWorkUnit::paint (QImage& image)
{
	auto paint_token = get_token (&image);

	_current_datetime = QDateTime::currentDateTime();

	if (_params.input_alert_visible)
		paint_input_alert (painter());
	else
	{
		adi_paint (painter());

		paint_nav (painter());
		paint_center_cross (painter(), false, true);
		paint_flight_director (painter());
		paint_control_stick (painter());
		paint_center_cross (painter(), true, false);

		if (_params.altitude_agl_failure)
			paint_radar_altimeter_failure (painter());
		else
			paint_altitude_agl (painter());

		paint_minimums_setting (painter());
		paint_hints (painter());
		paint_critical_aoa (painter());

		sl_paint (painter());
		al_paint (painter());
	}
}


void
ADIWidget::PaintWorkUnit::adi_post_resize()
{
	float const w_max = 2.f * _max_w_h;
	float const h_max = 10.f * _max_w_h;
	_adi_sky_rect = QRectF (-w_max, -h_max, 2.f * w_max, h_max + 1.f);
	_adi_gnd_rect = QRectF (-w_max, 0.f, 2.f * w_max, h_max);

	// Flight path marker:
	{
		float x = 0.013f * wh();
		float r = 1.05 * x;

		_flight_path_marker_shape = QPainterPath();
		_flight_path_marker_shape.addEllipse (QRectF (-x, -x, 2.f * x, 2.f * x));
		_flight_path_marker_shape.moveTo (QPointF (+r, 0.f));
		_flight_path_marker_shape.lineTo (QPointF (+4.f * x, 0.f));
		_flight_path_marker_shape.moveTo (QPointF (-r, 0.f));
		_flight_path_marker_shape.lineTo (QPointF (-4.f * x, 0.f));
		_flight_path_marker_shape.moveTo (QPointF (0.f, -r));
		_flight_path_marker_shape.lineTo (QPointF (0.f, -2.f * x));
	}

	// Old style clip:
	{
		float h = 0.2835f * wh();
		float w = 0.255f * wh();
		float r = 0.2f * h;

		_old_horizon_clip = QPainterPath();
		_old_horizon_clip.addRoundedRect (-w, -h, 2.f * w, 2.f * h, r, r);
	}

	// Pitch scale clipping path:
	{
		float const w = wh() * 2.f / 9.f;

		QPainterPath clip_path;
		clip_path.setFillRule (Qt::WindingFill);
		clip_path.addEllipse (QRectF (-1.f * w, -1.f * w, 2.f * w, 2.f * w));
		clip_path.addRect (QRectF (-1.f * w, 0.f, 2.f * w, 1.375f * w));

		_pitch_scale_clipping_path = clip_path;
	}
}


void
ADIWidget::PaintWorkUnit::adi_pre_paint()
{
	Angle p = xf::floored_mod (_params.orientation_pitch + 180_deg, 360_deg) - 180_deg;
	Angle r = xf::floored_mod (_params.orientation_roll + 180_deg, 360_deg) - 180_deg;
	Angle hdg = xf::floored_mod (_params.orientation_heading, 360_deg);

	// Mirroring, eg. -180° pitch is the same
	// as 0° pitch with roll inverted:
	if (p < -90_deg)
	{
		p = -180_deg - p;
		r = +180_deg - r;
	}
	else if (p > 90_deg)
	{
		p = +180_deg - p;
		r = +180_deg - r;
	}

	_params.orientation_pitch = p;
	_params.orientation_roll = r;
	_params.orientation_heading = hdg;

	_pitch_transform.reset();
	_pitch_transform.translate (0.f, -pitch_to_px (p));

	_roll_transform.reset();
	_roll_transform.rotate (-r.quantity<Degree>());

	_heading_transform.reset();
	_heading_transform.translate (-heading_to_px (hdg), 0.f);

	// Total transform of horizon (heading is not really necessary here):
	_horizon_transform = _pitch_transform * _roll_transform * _center_transform;
	// Without the following, Qt did something weird sometimes, like aligning drawn points to display pixels (?).
	_horizon_transform.shear (0.0001f, 0.f);

	// Limit FPM position:
	_params.flight_path_alpha = xf::clamped<Angle> (_params.flight_path_alpha, -25.0_deg, +25.0_deg);
	_params.flight_path_beta = xf::clamped<Angle> (_params.flight_path_beta, -25.0_deg, +25.0_deg);
	_flight_path_marker_position = QPointF (-heading_to_px (_params.flight_path_beta), -pitch_to_px (_params.flight_path_alpha));
}


void
ADIWidget::PaintWorkUnit::adi_paint (xf::Painter& painter)
{
	adi_pre_paint();

	if (_params.orientation_failure)
	{
		adi_clear (painter);
		adi_paint_attitude_failure (painter);
	}
	else
	{
		adi_paint_horizon (painter);
		adi_paint_pitch_scale (painter);
		adi_paint_heading (painter);
		adi_paint_tcas_ra (painter);
		adi_paint_roll_scale (painter);
		adi_paint_pitch_disagree (painter);
		adi_paint_roll_disagree (painter);
	}

	if (_params.flight_path_marker_failure)
		adi_paint_flight_path_marker_failure (painter);
	else
		adi_paint_flight_path_marker (painter);

	if (_params.flight_director_failure)
		adi_paint_flight_director_falure (painter);
}


void
ADIWidget::PaintWorkUnit::adi_clear (xf::Painter& painter)
{
	painter.setClipping (false);
	painter.resetTransform();
	painter.setPen (Qt::NoPen);
	painter.setBrush (Qt::black);
	painter.drawRect (QRect (QPoint (0, 0), size()));
}


void
ADIWidget::PaintWorkUnit::adi_paint_horizon (xf::Painter& painter)
{
	if (_params.orientation_pitch_visible && _params.orientation_roll_visible)
	{
		painter.setClipping (false);
		if (_params.old_style)
		{
			adi_clear (painter);
			painter.setTransform (_center_transform);
			painter.setClipPath (_old_horizon_clip);
		}
		painter.setTransform (_horizon_transform);
		painter.fillRect (_adi_sky_rect, _sky_color);
		painter.fillRect (_adi_gnd_rect, _ground_color);
	}
	else
		adi_clear (painter);
}


void
ADIWidget::PaintWorkUnit::adi_paint_pitch_scale (xf::Painter& painter)
{
	if (!_params.orientation_pitch_visible)
		return;

	float const w = wh() * 0.22222f; // 0.(2) == 2/9
	float const z = 0.5f * w;
	float const fpxs = _font_10.pixelSize();

	// Clip rectangle before and after rotation:
	painter.setTransform (_center_transform);
	painter.setClipPath (_pitch_scale_clipping_path);
	painter.setTransform (_roll_transform * _center_transform);
	painter.setClipRect (QRectF (-w, -1.f * w, 2.f * w, 2.2f * w), Qt::IntersectClip);
	painter.setTransform (_horizon_transform);
	QFont font = _font_13;
	font.setPixelSize (font_size (12.f));
	painter.setFont (font);

	// Pitch scale is clipped to small rectangle, so narrow it even more:
	float clipped_pitch_factor = 0.45f;
	xf::Range<Angle> deg_range (_params.orientation_pitch - clipped_pitch_factor * 0.485f * _params.fov,
								   _params.orientation_pitch + clipped_pitch_factor * 0.365f * _params.fov);

	painter.setPen (get_pen (Qt::white, 1.f));
	// 10° lines, exclude +/-90°:
	for (int deg = -90; deg <= 90; deg += 10)
	{
		QColor shadow_color = deg > 0 ? _sky_shadow : _ground_shadow;
		if (!deg_range.includes (1_deg * deg) || deg == 0)
			continue;
		float d = pitch_to_px (1_deg * deg);
		painter.add_shadow (shadow_color, [&] {
			painter.drawLine (QPointF (-z, d), QPointF (z, d));
		});
		// Degs number:
		int abs_deg = std::abs (deg);
		QString deg_t = QString::number (abs_deg > 90 ? 180 - abs_deg : abs_deg);
		// Text:
		QRectF lbox (-z - 4.25f * fpxs, d - 0.5f * fpxs, 4.f * fpxs, fpxs);
		QRectF rbox (+z + 0.25f * fpxs, d - 0.5f * fpxs, 4.f * fpxs, fpxs);
		painter.fast_draw_text (lbox, Qt::AlignVCenter | Qt::AlignRight, deg_t);
		painter.fast_draw_text (rbox, Qt::AlignVCenter | Qt::AlignLeft, deg_t);
	}
	// 5° lines:
	for (int deg = -90; deg <= 90; deg += 5)
	{
		QColor shadow_color = deg > 0 ? _sky_shadow : _ground_shadow;
		if (!deg_range.includes (1_deg * deg) || deg % 10 == 0)
			continue;
		float d = pitch_to_px (1_deg * deg);
		painter.add_shadow (shadow_color, [&] {
			painter.drawLine (QPointF (-z / 2.f, d), QPointF (z / 2.f, d));
		});
	}
	// 2.5° lines:
	for (int deg = -900; deg <= 900; deg += 25)
	{
		QColor shadow_color = deg > 0 ? _sky_shadow : _ground_shadow;
		if (!deg_range.includes (1_deg * deg / 10) || deg % 50 == 0)
			continue;
		float d = pitch_to_px (1_deg * deg / 10.f);
		painter.add_shadow (shadow_color, [&] {
			painter.drawLine (QPointF (-z / 4.f, d), QPointF (z / 4.f, d));
		});
	}
	// -90°, 90° lines:
	if (deg_range.includes (-90_deg) || deg_range.includes (+90_deg))
	{
		for (float deg: { -90.f, 90.f })
		{
			QColor shadow_color = deg > 0 ? _sky_shadow : _ground_shadow;
			float d = pitch_to_px (1_deg * deg);
			painter.setPen (get_pen (Qt::white, 1.75f));
			painter.add_shadow (shadow_color, [&] {
				painter.drawLine (QPointF (-z, d), QPointF (z, d));
			});
		}
	}

	// FPA bug:
	if (_params.cmd_fpa)
	{
		for (auto pen: { _autopilot_pen_1, _autopilot_pen_2 })
		{
			Angle fpa = *_params.cmd_fpa;
			painter.setPen (pen);
			for (auto y_angle: { fpa - 0.5_deg, fpa + 0.5_deg })
			{
				auto y_pos = pitch_to_px (y_angle);
				painter.drawLine (QPointF (-z, y_pos), QPointF (-0.25 * z, y_pos));
				painter.drawLine (QPointF (+z, y_pos), QPointF (+0.25 * z, y_pos));
			}
		}
	}
}


void
ADIWidget::PaintWorkUnit::adi_paint_roll_scale (xf::Painter& painter)
{
	float const w = wh() * 3.f / 9.f;

	QPen pen = get_pen (Qt::white, 1.f);
	painter.setPen (pen);
	painter.setBrush (QBrush (Qt::white));

	QPen warning_pen = pen;
	warning_pen.setColor (_warning_color_2);

	painter.setTransform (_center_transform);
	painter.setClipRect (QRectF (-w, -w, 2.f * w, 2.25f * w));
	for (float deg: { -60.f, -45.f, -30.f, -20.f, -10.f, 0.f, +10.f, +20.f, +30.f, +45.f, +60.f })
	{
		QColor shadow_color = deg > 0 ? _sky_shadow : _ground_shadow;

		painter.setTransform (_center_transform);
		painter.rotate (1.f * deg);
		painter.translate (0.f, -0.795f * w);

		if (deg == 0.f)
		{
			// Triangle:
			QPointF p0 (0.f, 0.f);
			QPointF px (0.025f * w, 0.f);
			QPointF py (0.f, 0.05f * w);
			painter.add_shadow ([&] {
				painter.drawPolygon (QPolygonF() << p0 << p0 - px - py << p0 + px - py);
			});
		}
		else
		{
			float length = -0.05f * w;
			if (std::abs (std::fmod (deg, 60.f)) < 1.f)
				length *= 1.6f;
			else if (std::abs (std::fmod (deg, 30.f)) < 1.f)
				length *= 2.2f;
			painter.add_shadow (shadow_color, [&] {
				painter.drawLine (QPointF (0.f, 0.f), QPointF (0.f, length));
			});
		}
	}

	if (!_params.orientation_roll_visible)
		return;

	float const bold_width = pen_width (3.f);
	QPointF a (0, 0.01f * w); // Miter
	QPointF b (-0.062f * w, 0.1f * w);
	QPointF c (+0.062f * w, 0.1f * w);
	QPointF x0 (0.002f * w, 0.f);
	QPointF y0 (0.f, 0.0f * w);
	QPointF y1 (0.f, 1.f * bold_width);

	QPolygonF slip_skid_polygon = QPolygonF()
		<< b - x0 + y0
		<< b - x0 + y1
		<< c + x0 + y1
		<< c + x0 + y0
		<< b - x0 + y0;
	QPolygonF bank_angle_polygon = QPolygonF() << b << a << c << b;

	for (bool is_shadow: { true, false })
	{
		painter.setTransform (_roll_transform * _center_transform);
		painter.translate (0.f, -0.79f * w);

		if (_params.roll_warning)
		{
			painter.setPen (warning_pen);
			painter.setBrush (warning_pen.color());
			if (is_shadow)
				painter.configure_for_shadow();
			painter.drawPolygon (bank_angle_polygon);
			if (is_shadow)
				painter.configure_normal();
		}
		else
		{
			painter.setPen (pen);
			if (is_shadow)
				painter.configure_for_shadow();
			painter.drawPolyline (bank_angle_polygon);
			if (is_shadow)
				painter.configure_normal();
		}

		if (_params.slip_skid_visible)
		{
			painter.translate (-xf::clamped (_params.slip_skid.quantity<si::Degree>(), -4.0, +4.0) * 0.08 * w, 0.0);

			if (_params.roll_warning || _params.slip_skid_warning)
				painter.setPen (warning_pen);
			else
				painter.setPen (pen);

			if (_params.slip_skid_warning)
			{
				painter.setBrush (warning_pen.color());
				if (is_shadow)
					painter.configure_for_shadow();
				painter.drawPolygon (slip_skid_polygon);
				if (is_shadow)
					painter.configure_normal();
			}
			else
			{
				if (is_shadow)
					painter.configure_for_shadow();
				painter.drawPolyline (slip_skid_polygon);
				if (is_shadow)
					painter.configure_normal();
			}
		}
	}
}


void
ADIWidget::PaintWorkUnit::adi_paint_heading (xf::Painter& painter)
{
	float const w = wh() * 2.25f / 9.f;
	float const fpxs = _font_10.pixelSize();

	if (!_params.orientation_pitch_visible || !_params.orientation_roll_visible)
		return;

	// Clip rectangle before and after rotation:
	painter.setTransform (_center_transform);
	painter.setClipPath (_pitch_scale_clipping_path);
	painter.setTransform (_roll_transform * _center_transform);
	painter.setClipRect (QRectF (-1.1f * w, -0.8f * w, 2.2f * w, 1.9f * w), Qt::IntersectClip);

	QPen p = get_pen (Qt::white, 1.f);
	p.setCapStyle (Qt::FlatCap);
	painter.setPen (p);
	painter.setFont (_font_10);

	if (_params.orientation_heading_visible)
	{
		float clipped_pitch_factor = 0.5f;
		xf::Range<Angle> deg_range (_params.orientation_heading - clipped_pitch_factor * 0.485f * _params.fov,
									   _params.orientation_heading + clipped_pitch_factor * 0.350f * _params.fov);

		painter.setTransform (_heading_transform * _horizon_transform);
		if (_params.orientation_heading_numbers_visible)
		{
			for (int deg = -180; deg < 540; deg += 10)
			{
				if (!deg_range.includes (1_deg * deg))
					continue;

				float d10 = heading_to_px (1_deg * deg);
				float d05 = heading_to_px (1_deg * deg + 5_deg);
				// 10° lines:
				painter.draw_outlined_line (QPointF (d10, -w / 18.f), QPointF (d10, 0.f));
				// 5° lines:
				painter.draw_outlined_line (QPointF (d05, -w / 36.f), QPointF (d05, 0.f));

				QString text = QString::fromStdString ((boost::format ("%02d") % (xf::floored_mod (1.f * deg, 360.f) / 10)).str());
				if (text == "00")
					text = "N";
				else if (text == "09")
					text = "E";
				else if (text == "18")
					text = "S";
				else if (text == "27")
					text = "W";
				painter.fast_draw_text (QRectF (d10 - 2.f * fpxs, 0.f, 4.f * fpxs, fpxs),
										Qt::AlignVCenter | Qt::AlignHCenter, text);
			}
		}
	}

	// Main horizon line:
	painter.setTransform (_horizon_transform);
	painter.setPen (get_pen (Qt::white, 1.25f));
	painter.draw_outlined_line (QPointF (-1.25 * w, 0.f), QPointF (1.25f * w, 0.f));
}


void
ADIWidget::PaintWorkUnit::adi_paint_tcas_ra (xf::Painter& painter)
{
	if (_params.tcas_ra_pitch_minimum || _params.tcas_ra_pitch_maximum)
	{
		painter.setPen (get_pen (Qt::red, 3.f));

		if (_params.old_style)
		{
			painter.setTransform (_center_transform);
			painter.setClipPath (_old_horizon_clip);
		}
		else
			painter.setClipping (false);

		auto paint_red_lines = [&] (Angle pitch1, Angle pitch2)
		{
			painter.setTransform (_horizon_transform);
			painter.translate (0.f, pitch_to_px (pitch1));
			float const h1 = heading_to_px (6_deg);
			float const h2 = heading_to_px (30_deg);
			float const p2 = pitch_to_px (pitch2);
			painter.add_shadow ([&] {
				painter.drawLine (-h1, 0.f, +h1, 0.f);
				painter.drawLine (-h1, 0.f, -h2, p2);
				painter.drawLine (+h1, 0.f, +h2, p2);
			});
		};

		if (_params.tcas_ra_pitch_minimum)
			paint_red_lines (*_params.tcas_ra_pitch_minimum, *_params.tcas_ra_pitch_minimum - 90_deg);

		if (_params.tcas_ra_pitch_maximum)
			paint_red_lines (*_params.tcas_ra_pitch_maximum, *_params.tcas_ra_pitch_maximum + 90_deg);
	}
}


void
ADIWidget::PaintWorkUnit::adi_paint_pitch_disagree (xf::Painter& painter)
{
	if (!_params.pitch_disagree)
		return;

	painter.setClipping (false);
	painter.setTransform (_center_transform);
	painter.setPen (get_pen (_warning_color_1, 1.f));
	painter.setFont (_font_16);
	painter.add_shadow ([&] {
		painter.fast_draw_text (QPointF (-0.225f * wh(), 0.285f * wh()), Qt::AlignVCenter | Qt::AlignLeft, "PITCH");
	});
}


void
ADIWidget::PaintWorkUnit::adi_paint_roll_disagree (xf::Painter& painter)
{
	if (!_params.roll_disagree)
		return;

	painter.setClipping (false);
	painter.setTransform (_center_transform);
	painter.setPen (get_pen (_warning_color_1, 1.f));
	painter.setFont (_font_16);
	painter.add_shadow ([&] {
		painter.fast_draw_text (QPointF (+0.225f * wh(), 0.285f * wh()), Qt::AlignVCenter | Qt::AlignRight, "ROLL");
	});
}


void
ADIWidget::PaintWorkUnit::adi_paint_flight_path_marker (xf::Painter& painter)
{
	if (!_params.flight_path_visible)
		return;

	painter.setTransform (_center_transform);
	painter.setClipRect (QRectF (-0.325f * wh(), -0.4f * wh(), 0.65f * wh(), 0.8f * wh()));
	painter.translate (_flight_path_marker_position);
	painter.setPen (get_pen (Qt::white, 1.25f));
	painter.setBrush (Qt::NoBrush);
	painter.add_shadow (1.8f, [&] {
		painter.drawPath (_flight_path_marker_shape);
	});
}


void
ADIWidget::PaintWorkUnit::sl_post_resize()
{
	float const wh = this->wh();

	_params.speed = xf::clamped<si::Velocity> (_params.speed, 0_kt, 9999.99_kt);
	_params.speed_mach = xf::clamped (_params.speed_mach, 0.0, 9.99);
	_params.speed_minimum = xf::clamped<si::Velocity> (_params.speed_minimum, 0.0_kt, 9999.99_kt);
	if (_params.speed_minimum_maneuver)
		_params.speed_minimum_maneuver = xf::clamped<si::Velocity> (*_params.speed_minimum_maneuver, 0.0_kt, 9999.99_kt);
	if (_params.speed_maximum_maneuver)
		_params.speed_maximum_maneuver = xf::clamped<si::Velocity> (*_params.speed_maximum_maneuver, 0.0_kt, 9999.99_kt);
	_params.speed_maximum = xf::clamped<si::Velocity> (_params.speed_maximum, 0.0_kt, 9999.99_kt);

	_sl_ladder_rect = QRectF (-0.0675f * wh, -0.375 * wh, 0.135 * wh, 0.75f * wh);
	_sl_ladder_pen = QPen (_ladder_border_color, pen_width (0.75f), Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin);
	_sl_black_box_pen = get_pen (Qt::white, 1.2f, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
	_sl_scale_pen = get_pen (Qt::white, 1.f);
	_sl_speed_bug_pen = get_pen (Qt::green, 1.5f);

	QFont actual_speed_font = _font_20;
	float const digit_width = _font_20_digit_width;
	float const digit_height = _font_20_digit_height;
	_sl_margin = 0.25f * digit_width;
	_sl_digits = (_params.speed >= 1000_kt - 0.5_kt) ? 4 : 3;

	float const box_height_factor = 2.35;
	_sl_black_box_rect = QRectF (-_sl_digits * digit_width - 2.f * _sl_margin, -0.5f * box_height_factor * digit_height,
								 +_sl_digits * digit_width + 2.f * _sl_margin, box_height_factor * digit_height);

	_sl_transform = _center_transform;
	_sl_transform.translate (-0.4f * wh, 0.f);

	// Speed bug shape:
	{
		float const x = _sl_ladder_rect.width() / 4.0f;

		_sl_bug_shape = QPolygonF()
			<< QPointF (0.f, 0.f)
			<< QPointF (+0.5f * x, -0.5f * x)
			<< QPointF (2.f * x, -0.5f * x)
			<< QPointF (2.f * x, +0.5f * x)
			<< QPointF (+0.5f * x, +0.5f * x);
	}
}


void
ADIWidget::PaintWorkUnit::sl_pre_paint()
{
	_params.speed = xf::clamped<si::Velocity> (_params.speed, 1_kt * _params.sl_minimum, 1_kt * _params.sl_maximum);
	_sl_min_shown = _params.speed - 0.5f * _params.sl_extent;
	_sl_max_shown = _params.speed + 0.5f * _params.sl_extent;
	_sl_min_shown = std::max<si::Velocity> (_sl_min_shown, 1_kt * _params.sl_minimum);
	_sl_max_shown = std::min<si::Velocity> (_sl_max_shown, 1_kt * _params.sl_maximum);
	if (_sl_min_shown < 0_kt)
		_sl_min_shown = 0_kt;
	_sl_rounded_speed = static_cast<int> (_params.speed.quantity<Knot>() + 0.5);
}


void
ADIWidget::PaintWorkUnit::sl_paint (xf::Painter& painter)
{
	sl_pre_paint();

	float const x = _sl_ladder_rect.width() / 4.0f;

	painter.setClipping (false);
	painter.setTransform (_sl_transform);

	if (_params.speed_failure)
		sl_paint_failure (painter);
	else
	{
		painter.setPen (_sl_ladder_pen);
		painter.setBrush (_ladder_color);
		painter.drawRect (_sl_ladder_rect);

		sl_paint_ladder_scale (painter, x);
		sl_paint_speed_limits (painter, x);
		sl_paint_bugs (painter, x);
		sl_paint_speed_tendency (painter, x);
		sl_paint_black_box (painter, x);
		sl_paint_ias_disagree (painter, x);
	}

	sl_paint_mach_or_gs (painter, x);
	sl_paint_novspd_flag (painter);
	sl_paint_ap_setting (painter);
}


void
ADIWidget::PaintWorkUnit::sl_paint_black_box (xf::Painter& painter, float x)
{
	if (!_params.speed_visible)
		return;

	QFont actual_speed_font = _font_20;
	float const digit_width = _font_20_digit_width;

	painter.setClipping (false);
	painter.setTransform (_sl_transform);
	painter.translate (+0.75f * x, 0.f);

	QPen border_pen = _sl_black_box_pen;
	bool speed_is_in_warning_area = (_params.speed_minimum < _params.speed && _params.speed_minimum_maneuver && _params.speed < *_params.speed_minimum_maneuver) ||
									(_params.speed_maximum > _params.speed && _params.speed_maximum_maneuver && _params.speed > *_params.speed_maximum_maneuver);
	if (_locals.speed_blinking_active || speed_is_in_warning_area)
	{
		if (_locals.speed_blink || speed_is_in_warning_area)
			border_pen.setColor (_warning_color_2);
		else
			border_pen.setColor (Qt::black);
	}

	painter.setPen (border_pen);
	painter.setBrush (QBrush (QColor (0, 0, 0)));

	QPolygonF black_box_polygon = QPolygonF()
		<< QPointF (+0.5f * x, 0.f)
		<< QPointF (0.f, -0.5f * x)
		<< _sl_black_box_rect.topRight()
		<< _sl_black_box_rect.topLeft()
		<< _sl_black_box_rect.bottomLeft()
		<< _sl_black_box_rect.bottomRight()
		<< QPointF (0.f, +0.5f * x);

	QColor ps = painter.shadow_color();
	painter.set_shadow_color (Qt::black);
	painter.add_shadow ([&] {
		painter.drawPolygon (black_box_polygon);
	});
	painter.set_shadow_color (ps);

	QRectF box_1000 = _sl_black_box_rect.adjusted (_sl_margin, _sl_margin, -_sl_margin, -_sl_margin);
	QRectF box_0100 =
		_sl_digits == 3
			? box_1000
			: box_1000.adjusted (digit_width, 0.f, 0.f, 0.f);
	QRectF box_0010 = box_0100.adjusted (digit_width, 0.f, 0.f, 0.f);
	QRectF box_0001 = box_0010.adjusted (digit_width, 0.f, 0.f, 0.f);

	painter.setPen (QPen (Qt::white, 1.f, Qt::SolidLine, Qt::RoundCap));
	painter.setFont (actual_speed_font);
	if (_sl_digits == 4)
		paint_rotating_digit (painter, box_1000, _params.speed.quantity<Knot>(), 1000, 1.25f, 0.0005f, 0.5f, false, true);
	paint_rotating_digit (painter, box_0100, _params.speed.quantity<Knot>(), 100, 1.25f, 0.005f, 0.5f, false, true, true);
	paint_rotating_digit (painter, box_0010, _params.speed.quantity<Knot>(), 10, 1.25f, 0.05f, 0.5f, false, false);
	float pos_0001 = _sl_rounded_speed - _params.speed.quantity<Knot>();
	paint_rotating_value (painter, box_0001, pos_0001, 0.7f,
						  QString::number (static_cast<int> (std::abs (std::fmod (1.f * _sl_rounded_speed + 1.f, 10.f)))),
						  QString::number (static_cast<int> (std::abs (std::fmod (1.f * _sl_rounded_speed, 10.f)))),
							 _params.speed > (1_kt * _params.sl_minimum + 0.5_kt)
								? QString::number (static_cast<int> (xf::floored_mod (1.f * _sl_rounded_speed - 1.f, 10.f)))
								: " ");
}


void
ADIWidget::PaintWorkUnit::sl_paint_ias_disagree (xf::Painter& painter, float x)
{
	if (!_params.ias_disagree)
		return;

	painter.setClipping (false);
	painter.setTransform (_sl_transform);
	painter.setFont (_font_8);
	painter.setPen (get_pen (_warning_color_2, 1.f));
	QPointF position (-1.75f * x, 9.5f * x);
	painter.add_shadow ([&] {
		painter.fast_draw_text (position, Qt::AlignVCenter | Qt::AlignLeft, "IAS");
		painter.fast_draw_text (position + QPointF (0.f, 0.9f * x), Qt::AlignVCenter | Qt::AlignLeft, "DISAGREE");
	});
}


void
ADIWidget::PaintWorkUnit::sl_paint_ladder_scale (xf::Painter& painter, float x)
{
	if (!_params.speed_visible)
		return;

	QFont ladder_font = _font_13;
	float const ladder_digit_width = _font_13_digit_width;
	float const ladder_digit_height = _font_13_digit_height;

	painter.setFont (ladder_font);

	// Special clipping that leaves some margin around black indicator:
	QPainterPath clip_path_m;
	clip_path_m.addRect (_sl_black_box_rect.translated (x, 0.f).adjusted (0.f, -0.2f * x, 0.f, +0.2f * x));
	QPainterPath clip_path;
	clip_path.addRect (_sl_ladder_rect);
	clip_path -= clip_path_m;

	painter.setTransform (_sl_transform);
	painter.setClipPath (clip_path, Qt::IntersectClip);
	painter.translate (2.f * x, 0.f);

	painter.setPen (_sl_scale_pen);
	// -+line_every is to have drawn also numbers that barely fit the scale.
	for (int kt = (static_cast<int> (_sl_min_shown.quantity<Knot>()) / _params.sl_line_every) * _params.sl_line_every - _params.sl_line_every;
		 kt <= _sl_max_shown.quantity<Knot>() + _params.sl_line_every;
		 kt += _params.sl_line_every)
	{
		if (kt < _params.sl_minimum || kt > _params.sl_maximum)
			continue;
		float posy = kt_to_px (1_kt * kt);
		painter.draw_outlined_line (QPointF (-0.8f * x, posy), QPointF (0.f, posy));

		if ((kt - _params.sl_minimum) % _params.sl_number_every == 0)
			painter.fast_draw_text (QRectF (-4.f * ladder_digit_width - 1.25f * x, -0.5f * ladder_digit_height + posy,
											+4.f * ladder_digit_width, ladder_digit_height),
									Qt::AlignVCenter | Qt::AlignRight, QString::number (kt));
	}
}


void
ADIWidget::PaintWorkUnit::sl_paint_speed_limits (xf::Painter& painter, float x)
{
	if (!_params.speed_visible)
		return;

	QPointF ydif (0.f, pen_width (0.25f));
	QPen pen_b (QColor (0, 0, 0), pen_width (8.f), Qt::SolidLine, Qt::FlatCap);
	QPen pen_r (QColor (255, 0, 0), pen_width (8.f), Qt::DashLine, Qt::FlatCap);
	pen_r.setDashPattern (QVector<qreal> { 0.5f, 0.75f });
	QPen pen_y (_warning_color_2, pen_width (1.2f), Qt::SolidLine, Qt::FlatCap);

	float tr_right = 0.45f * x;
	float p1w = 0.45f * pen_width (1.2f);

	painter.setTransform (_sl_transform);
	painter.translate (tr_right, 0.f);
	painter.setClipRect (_sl_ladder_rect.adjusted (0.f, -ydif.y(), 0.f, ydif.y()));

	float min_posy = kt_to_px (_params.speed_minimum);
	float min_man_posy = kt_to_px (_params.speed_minimum_maneuver ? *_params.speed_minimum_maneuver : 0_mps);
	float max_man_posy = kt_to_px (_params.speed_maximum_maneuver ? *_params.speed_maximum_maneuver : 0_mps);
	float max_posy = kt_to_px (_params.speed_maximum);
	QPointF min_point = _sl_ladder_rect.bottomRight() + ydif;
	QPointF max_point = _sl_ladder_rect.topRight() - ydif;

	if (_params.speed_minimum_maneuver && *_params.speed_minimum_maneuver > _sl_min_shown)
	{
		QPolygonF poly = QPolygonF()
			<< QPointF (_sl_ladder_rect.right() - tr_right, min_man_posy)
			<< QPointF (_sl_ladder_rect.right() - p1w, min_man_posy)
			<< min_point - QPointF (p1w, 0.f);
		painter.setPen (pen_y);
		painter.add_shadow ([&] {
			painter.drawPolyline (poly);
		});
	}

	if (_params.speed_maximum_maneuver && *_params.speed_maximum_maneuver < _sl_max_shown)
	{
		QPolygonF poly = QPolygonF()
			<< QPointF (_sl_ladder_rect.right() - tr_right, max_man_posy)
			<< QPointF (_sl_ladder_rect.right() - p1w, max_man_posy)
			<< max_point - QPointF (p1w, 0.f);
		painter.setPen (pen_y);
		painter.add_shadow ([&] {
			painter.drawPolyline (poly);
		});
	}

	if (_params.speed_maximum_visible && _params.speed_maximum < _sl_max_shown)
	{
		painter.setPen (pen_b);
		painter.drawLine (QPointF (_sl_ladder_rect.right(), max_posy), max_point);
		painter.setPen (pen_r);
		painter.drawLine (QPointF (_sl_ladder_rect.right(), max_posy), max_point);
	}

	if (_params.speed_minimum_visible && _params.speed_minimum > _sl_min_shown)
	{
		painter.setPen (pen_b);
		painter.drawLine (QPointF (_sl_ladder_rect.right(), min_posy), min_point);
		painter.setPen (pen_r);
		painter.drawLine (QPointF (_sl_ladder_rect.right(), min_posy), min_point);
	}
}


void
ADIWidget::PaintWorkUnit::sl_paint_speed_tendency (xf::Painter& painter, float x)
{
	if (!_params.speed_lookahead_visible || !_params.speed_visible)
		return;

	QPen pen (get_pen (_navigation_color, 1.25f));
	pen.setCapStyle (Qt::RoundCap);
	pen.setJoinStyle (Qt::RoundJoin);

	using std::abs;

	painter.setTransform (_sl_transform);
	painter.setPen (pen);
	painter.translate (1.2f * x, 0.f);
	if (_params.speed_lookahead < _params.speed)
		painter.scale (1.f, -1.f);
	float length = std::min<float> (_sl_ladder_rect.height() / 2.f,
									1.f * abs (kt_to_px (xf::clamped<si::Velocity> (_params.speed_lookahead, 1_kt * _params.sl_minimum, 1_kt * _params.sl_maximum)))) - 0.5f * x;

	if (length > 0.2f * x)
	{
		painter.setClipRect (QRectF (_sl_ladder_rect.topLeft(), QPointF (_sl_ladder_rect.right(), 0.f)));
		painter.add_shadow ([&] {
			painter.drawPolygon (QPolygonF()
				<< QPointF (0.f, 0.f)
				<< QPointF (0.f, -length)
				<< QPointF (-0.2f * x, 0.f - length)
				<< QPointF (0.f, -0.5f * x - length)
				<< QPointF (+0.2f * x, 0.f - length)
				<< QPointF (0.f, -length));
		});
	}
}


void
ADIWidget::PaintWorkUnit::sl_paint_bugs (xf::Painter& painter, float x)
{
	if (!_params.speed_visible)
		return;

	QFont speed_bug_font = _font_10;
	float const speed_bug_digit_height = _font_10_digit_height;

	painter.setClipping (false);
	painter.setTransform (_sl_transform);
	painter.setFont (speed_bug_font);

	for (auto& bug: _params.speed_bugs)
	{
		if (bug.second > _sl_min_shown && bug.second < _sl_max_shown)
		{
			float posy = kt_to_px (bug.second);
			painter.setPen (_sl_speed_bug_pen);
			painter.setClipRect (_sl_ladder_rect.translated (x, 0.f));
			painter.add_shadow ([&] {
				painter.drawLine (QPointF (1.5f * x, posy), QPointF (2.25f * x, posy));
			});
			painter.setClipping (false);
			painter.fast_draw_text (QRectF (2.5f * x, posy - 0.5f * speed_bug_digit_height,
											2.f * x, speed_bug_digit_height),
									Qt::AlignVCenter | Qt::AlignLeft, bug.first);
		}
	}

	// Speed bug:
	if (_params.cmd_speed)
	{
		float posy = xf::clamped<float> (kt_to_px (xf::clamped<si::Velocity> (*_params.cmd_speed, 1_kt * _params.sl_minimum, 1_kt * _params.sl_maximum)),
										 -_sl_ladder_rect.height() / 2.0, _sl_ladder_rect.height() / 2.0);
		painter.setClipRect (_sl_ladder_rect.translated (2.5f * x, 0.f));
		painter.translate (1.25f * x, posy);
		painter.setBrush (Qt::NoBrush);
		painter.setPen (_autopilot_pen_1);
		painter.drawPolygon (_sl_bug_shape);
		painter.setPen (_autopilot_pen_2);
		painter.drawPolygon (_sl_bug_shape);
	}
}


void
ADIWidget::PaintWorkUnit::sl_paint_mach_or_gs (xf::Painter& painter, float x)
{
	if (!_params.speed_mach_visible && !_params.speed_ground)
		return;

	painter.setClipping (false);
	painter.setTransform (_sl_transform);
	painter.translate (0.f, 0.75f * x);
	QPointF paint_position (0.0, _sl_ladder_rect.bottom() + 0.5 * _font_20_digit_height);

	if (_params.speed_mach_visible)
	{
		QFont font = _font_20;

		QString mach_str = QString ("%1").arg (_params.speed_mach, 0, 'f', 3);
		if (mach_str.left (2) == "0.")
			mach_str = mach_str.mid (1);

		painter.setPen (get_pen (Qt::white, 1.f));
		painter.setFont (font);
		painter.fast_draw_text (paint_position, Qt::AlignCenter, mach_str);
	}
	else if (_params.speed_ground)
	{
		xf::TextLayout layout;
		layout.set_alignment (Qt::AlignHCenter);
		layout.add_fragment ("GS", _font_16, Qt::white);
		layout.add_fragment (" ", _font_10, Qt::white);
		layout.add_fragment (QString::number (static_cast<int> (_params.speed_ground->quantity<Knot>())), _font_20, Qt::white);
		layout.paint (paint_position, Qt::AlignCenter, painter);
	}
}


void
ADIWidget::PaintWorkUnit::sl_paint_ap_setting (xf::Painter& painter)
{
	if (!_params.cmd_speed && !_params.cmd_mach)
		return;

	QFont actual_speed_font = _font_20;
	float const digit_width = _font_20_digit_width;
	float const digit_height = _font_20_digit_height;
	float const margin = 0.2f * digit_width;
	int digits = 0;
	QString value;

	// Mach info has priority:
	if (_params.cmd_mach)
	{
		value = QString::fromStdString ((boost::format ("%5.3f") % *_params.cmd_mach).str());
		if (value.size() > 0 && value[0] == '0')
			value = value.mid (1);
		digits = value.size();
	}
	else if (_params.cmd_speed)
	{
		value = QString::number (std::abs (static_cast<int> (_params.cmd_speed->quantity<Knot>())));
		digits = 4;
	}

	QRectF digits_box (0.f, 0.f, digits * digit_width + 2.f * margin, 1.3f * digit_height);
	QRectF box_rect (_sl_ladder_rect.right() - digits_box.width(), _sl_ladder_rect.top() - 1.4f * digits_box.height(),
					 digits_box.width(), digits_box.height());

	painter.setClipping (false);
	painter.setTransform (_sl_transform);
	painter.setPen (get_pen (QColor (0, 0, 0), 0.5f));
	painter.setBrush (QBrush (QColor (0, 0, 0)));
	painter.drawRect (box_rect);

	painter.setPen (get_pen (_autopilot_color, 1.f));
	painter.setFont (actual_speed_font);

	QRectF box = box_rect.adjusted (margin, margin, -margin, -margin);
	painter.fast_draw_text (box, Qt::AlignVCenter | Qt::AlignRight, value);
}


void
ADIWidget::PaintWorkUnit::sl_paint_novspd_flag (xf::Painter& painter)
{
	if (_params.novspd_flag)
	{
		float margin = 0.025f * _q;
		QString sa = "NO";
		QString sb = "VSPD";
		QFont font = _font;
		font.setPixelSize (font_size (18.f));
		QFontMetricsF metrics (font);
		float font_height = 0.9f * metrics.height();

		QRectF rect (0.f, 0.f, metrics.width (sa), font_height * (sb.size() + 1));
		rect.moveLeft (0.9f * _q);
		rect.moveBottom (-0.4f * _q);

		painter.setClipping (false);
		painter.setTransform (_sl_transform);
		painter.setPen (Qt::NoPen);
		painter.setBrush (Qt::black);
		painter.drawRect (rect.adjusted (-margin, 0.f, +margin, 0.f));
		painter.setPen (get_pen (_warning_color_2, 1.f));
		painter.setFont (font);

		QPointF c (rect.center().x(), rect.top());
		QPointF h (0.f, font_height);

		painter.fast_draw_text (c + 0.5f * h, Qt::AlignHCenter | Qt::AlignVCenter, sa);
		for (int i = 0; i < sb.size(); ++i)
			painter.fast_draw_text (c + 1.5f * h + i * h, Qt::AlignHCenter | Qt::AlignVCenter, sb.mid (i, 1));
	}
}


void
ADIWidget::PaintWorkUnit::al_post_resize()
{
	float const wh = this->wh();

	_al_ladder_rect = QRectF (-0.0675f * wh, -0.375 * wh, 0.135 * wh, 0.75f * wh);
	_al_ladder_pen = QPen (_ladder_border_color, pen_width (0.75f), Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin);
	_al_black_box_pen = get_pen (Qt::white, 1.2f, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
	_al_scale_pen_1 = get_pen (Qt::white, 1.f);
	_al_scale_pen_2 = get_pen (Qt::white, 3.f, Qt::SolidLine, Qt::SquareCap);
	_al_altitude_bug_pen = get_pen (QColor (0, 255, 0), 1.5f);
	_al_ldg_alt_pen = get_pen (QColor (255, 220, 0), 1.5f);
	_al_ldg_alt_pen.setCapStyle (Qt::RoundCap);

	float const b_digit_width = _font_20_digit_width;
	float const b_digit_height = _font_20_digit_height;
	float const s_digit_width = _font_16_digit_width;
	int const b_digits = 2;
	int const s_digits = 3;
	_al_margin = 0.25f * b_digit_width;

	float const box_height_factor = 2.35;
	_al_b_digits_box = QRectF (0.f, 0.f, b_digits * b_digit_width, box_height_factor * b_digit_height - 2.f * _al_margin);
	_al_s_digits_box = QRectF (0.f, 0.f, s_digits * s_digit_width, box_height_factor * b_digit_height - 2.f * _al_margin);
	_al_black_box_rect = QRectF (0.f, -0.5f * _al_b_digits_box.height() - _al_margin,
								 _al_b_digits_box.width() + _al_s_digits_box.width() + 2.f * _al_margin, _al_b_digits_box.height() + 2.f * _al_margin);
	_al_metric_box_rect = QRectF (_al_black_box_rect.topLeft() - QPointF (0.f, 1.25f * _font_16_digit_height), _al_black_box_rect.topRight());
	_al_b_digits_box.translate (_al_margin, -0.5f * _al_b_digits_box.height());
	_al_s_digits_box.translate (_al_margin + _al_b_digits_box.width(), -0.5f * _al_s_digits_box.height());
}


void
ADIWidget::PaintWorkUnit::al_pre_paint()
{
	_params.altitude = xf::clamped<Length> (_params.altitude, -99999_ft, +99999_ft);
	_params.vertical_speed = xf::clamped<si::Velocity> (_params.vertical_speed, -9999_fpm, +9999_fpm);

	float sgn = _params.altitude < 0_ft ? -1.f : 1.f;
	_al_min_shown = _params.altitude - 0.5f * _params.al_extent;
	_al_max_shown = _params.altitude + 0.5f * _params.al_extent;
	_al_rounded_altitude = static_cast<int> (_params.altitude.quantity<Foot>() + sgn * 10.f) / 20 * 20;

	_al_transform = _center_transform;
	_al_transform.translate (+0.4f * wh(), 0.f);
}


void
ADIWidget::PaintWorkUnit::al_paint (xf::Painter& painter)
{
	al_pre_paint();

	float const x = _al_ladder_rect.width() / 4.0f;

	if (_params.vertical_speed_failure)
		al_paint_vertical_speed_failure (painter, x);
	else
	{
		painter.setClipping (false);
		painter.setTransform (_al_transform);

		al_paint_vertical_speed (painter, x);
	}

	if (_params.altitude_failure)
		al_paint_failure (painter);
	else
	{
		painter.setClipping (false);
		painter.setTransform (_al_transform);

		painter.setPen (_al_ladder_pen);
		painter.setBrush (_ladder_color);
		painter.drawRect (_al_ladder_rect);

		al_paint_ladder_scale (painter, x);
		al_paint_bugs (painter, x);
		al_paint_altitude_tendency (painter, x);
		al_paint_black_box (painter, x);
		al_paint_altitude_disagree (painter, x);
	}

	painter.setClipping (false);
	painter.setTransform (_al_transform);

	al_paint_pressure (painter, x);
	al_paint_ap_setting (painter);
	al_paint_ldgalt_flag (painter, x);
}


void
ADIWidget::PaintWorkUnit::al_paint_black_box (xf::Painter& painter, float x)
{
	QFont b_font = _font_20;
	float const b_digit_width = _font_20_digit_width;
	float const b_digit_height = _font_20_digit_height;

	QFont s_font = _font_16;
	float const s_digit_width = _font_16_digit_width;
	float const s_digit_height = _font_16_digit_height;

	QFont m_font = _font_13;

	QFontMetricsF s_metrics (s_font);
	QFontMetricsF m_metrics (m_font);

	if (!_params.altitude_visible)
		return;

	painter.setClipping (false);
	painter.setTransform (_al_transform);
	painter.translate (-0.75f * x, 0.f);

	QPolygonF black_box_polygon = QPolygonF()
		<< QPointF (-0.5f * x, 0.f)
		<< QPointF (0.f, -0.5f * x)
		<< _al_black_box_rect.topLeft()
		<< _al_black_box_rect.topRight()
		<< _al_black_box_rect.bottomRight()
		<< _al_black_box_rect.bottomLeft()
		<< QPointF (0.f, +0.5f * x);

	if (_params.show_metric)
	{
		painter.setPen (_al_black_box_pen);
		painter.setBrush (Qt::black);

		// Metric box:
		QColor ps = painter.shadow_color();
		painter.set_shadow_color (Qt::black);
		painter.add_shadow ([&] {
			painter.drawRect (_al_metric_box_rect);
		});
		painter.set_shadow_color (ps);

		// Metric value:
		float xcorr = 0.25f * m_metrics.width (" ");
		QPointF m_pos (_al_metric_box_rect.right() - 1.5f * m_metrics.width ("M"), _al_metric_box_rect.center().y());
		painter.setPen (get_pen (QColor (0x00, 0xee, 0xff), 1.f));
		painter.setFont (m_font);
		painter.fast_draw_text (m_pos, Qt::AlignLeft | Qt::AlignVCenter, "M");
		painter.setPen (get_pen (Qt::white, 1.f));
		painter.fast_draw_text (m_pos + QPointF (-xcorr, 0.f), Qt::AlignRight | Qt::AlignVCenter,
								QString ("%1").arg (std::round (_params.altitude.quantity<Meter>()), 0, 'f', 0));
	}

	painter.setPen (_al_black_box_pen);
	painter.setBrush (Qt::black);

	// Feet box:
	QColor ps = painter.shadow_color();
	painter.set_shadow_color (Qt::black);
	painter.add_shadow ([&] {
		painter.drawPolygon (black_box_polygon);
	});
	painter.set_shadow_color (ps);

	// Feet value:
	QRectF box_10000 = QRectF (_al_b_digits_box.topLeft(), QSizeF (b_digit_width, _al_b_digits_box.height()));
	QRectF box_01000 = box_10000.translated (b_digit_width, 0.f);
	QRectF box_00100 = QRectF (_al_s_digits_box.topLeft(), QSizeF (s_digit_width, _al_b_digits_box.height()));
	QRectF box_00011 = box_00100.translated (s_digit_width, 0.f).adjusted (0.f, 0.f, s_digit_width, 0.f);

	// 11100 part:
	painter.setFont (b_font);
	paint_rotating_digit (painter, box_10000, _params.altitude.quantity<Foot>(), 10000, 1.25f * s_digit_height / b_digit_height, 0.0005f, 5.f, true, true);
	paint_rotating_digit (painter, box_01000, _params.altitude.quantity<Foot>(), 1000, 1.25f * s_digit_height / b_digit_height, 0.005f, 5.f, false, false);
	painter.setFont (s_font);
	paint_rotating_digit (painter, box_00100, _params.altitude.quantity<Foot>(), 100, 1.25f, 0.05f, 5.f, false, false);

	// 00011 part:
	float pos_00011 = (_al_rounded_altitude - _params.altitude.quantity<Foot>()) / 20.f;
	paint_rotating_value (painter, box_00011, pos_00011, 0.75f,
						  QString::number (static_cast<int> (std::abs (std::fmod (_al_rounded_altitude / 10.f + 2.f, 10.f)))) + "0",
						  QString::number (static_cast<int> (std::abs (std::fmod (_al_rounded_altitude / 10.f + 0.f, 10.f)))) + "0",
						  QString::number (static_cast<int> (std::abs (std::fmod (_al_rounded_altitude / 10.f - 2.f, 10.f)))) + "0");
}


void
ADIWidget::PaintWorkUnit::al_paint_altitude_disagree (xf::Painter& painter, float x)
{
	if (!_params.altitude_disagree)
		return;

	painter.setClipping (false);
	painter.setTransform (_al_transform);
	painter.setFont (_font_8);
	painter.setPen (get_pen (_warning_color_2, 1.f));
	QPointF position (-1.75f * x, 9.5f * x);
	painter.add_shadow ([&] {
		painter.fast_draw_text (position, Qt::AlignVCenter | Qt::AlignLeft, "ALT");
		painter.fast_draw_text (position + QPointF (0.f, 0.9f * x), Qt::AlignVCenter | Qt::AlignLeft, "DISAGREE");
	});
}


void
ADIWidget::PaintWorkUnit::al_paint_ladder_scale (xf::Painter& painter, float x)
{
	if (!_params.altitude_visible)
		return;

	QFont b_ladder_font = _font_13;
	float const b_ladder_digit_width = _font_13_digit_width;
	float const b_ladder_digit_height = _font_13_digit_height;

	QFont s_ladder_font = _font_10;
	float const s_ladder_digit_width = _font_10_digit_width;
	float const s_ladder_digit_height = _font_10_digit_height;

	// Special clipping that leaves some margin around black indicator:
	QPainterPath clip_path_m;
	clip_path_m.addRect (_al_black_box_rect.translated (-x, 0.f).adjusted (0.f, -0.2f * x, 0.f, +0.2f * x));
	QPainterPath clip_path;
	clip_path.addRect (_al_ladder_rect);
	clip_path -= clip_path_m;

	painter.setTransform (_al_transform);
	painter.setClipPath (clip_path, Qt::IntersectClip);
	painter.translate (-2.f * x, 0.f);

	// -+line_every is to have drawn also numbers that barely fit the scale.
	for (int ft = (static_cast<int> (_al_min_shown.quantity<Foot>()) / _params.al_line_every) * _params.al_line_every - _params.al_line_every;
		 ft <= _al_max_shown.quantity<Foot>() + _params.al_line_every;
		 ft += _params.al_line_every)
	{
		if (ft > 100000.f)
			continue;

		float posy = ft_to_px (1_ft * ft);

		painter.setPen (ft % _params.al_bold_every == 0 ? _al_scale_pen_2 : _al_scale_pen_1);
		painter.draw_outlined_line (QPointF (0.f, posy), QPointF (0.8f * x, posy));

		if (ft % _params.al_number_every == 0)
		{
			QRectF big_text_box (1.1f * x, -0.5f * b_ladder_digit_height + posy,
								 2.f * b_ladder_digit_width, b_ladder_digit_height);
			if (std::abs (ft) / 1000 > 0)
			{
				QString big_text = QString::number (ft / 1000);
				painter.setFont (b_ladder_font);
				painter.fast_draw_text (big_text_box, Qt::AlignVCenter | Qt::AlignRight, big_text);
			}

			QString small_text = QString ("%1").arg (QString::number (std::abs (ft % 1000)), 3, '0');
			if (ft == 0)
				small_text = "0";
			painter.setFont (s_ladder_font);
			QRectF small_text_box (1.1f * x + 2.1f * b_ladder_digit_width, -0.5f * s_ladder_digit_height + posy,
								   3.f * s_ladder_digit_width, s_ladder_digit_height);
			painter.fast_draw_text (small_text_box, Qt::AlignVCenter | Qt::AlignRight, small_text);
			// Minus sign?
			if (ft < 0)
			{
				if (ft > -1000)
					painter.fast_draw_text (small_text_box.adjusted (-s_ladder_digit_width, 0.f, 0.f, 0.f),
											Qt::AlignVCenter | Qt::AlignLeft, MINUS_SIGN);
			}

			// Additional lines above/below every 1000 ft:
			if (ft % _params.al_emphasis_every == 0)
			{
				painter.setPen (get_pen (Qt::white, 1.0));
				float r, y;
				r = big_text_box.left() + 4.0 * x;
				y = posy - 0.75f * big_text_box.height();
				painter.draw_outlined_line (QPointF (big_text_box.left(), y),
											QPointF (r, y));
				y = posy + 0.75f * big_text_box.height();
				painter.draw_outlined_line (QPointF (big_text_box.left(), y),
											QPointF (r, y));
			}
		}
	}
}


void
ADIWidget::PaintWorkUnit::al_paint_altitude_tendency (xf::Painter& painter, float x)
{
	if (!_params.altitude_lookahead_visible || !_params.altitude_visible)
		return;

	QPen pen (get_pen (_navigation_color, 1.25f));
	pen.setCapStyle (Qt::RoundCap);
	pen.setJoinStyle (Qt::RoundJoin);

	painter.setTransform (_al_transform);
	painter.translate (-1.2f * x, 0.f);
	painter.setPen (pen);
	if (_params.altitude_lookahead < _params.altitude)
		painter.scale (1.f, -1.f);
	float length = std::min<float> (_al_ladder_rect.height() / 2.f, 1.f * std::abs (ft_to_px (_params.altitude_lookahead))) - 0.5f * x;

	if (length > 0.2f * x)
	{
		painter.setClipRect (QRectF (_al_ladder_rect.topLeft(), QPointF (_al_ladder_rect.right(), 0.f)));
		painter.add_shadow ([&] {
			painter.drawPolygon (QPolygonF()
				<< QPointF (0.f, 0.f)
				<< QPointF (0.f, -length)
				<< QPointF (-0.2f * x, 0.f - length)
				<< QPointF (0.f, -0.5f * x - length)
				<< QPointF (+0.2f * x, 0.f - length)
				<< QPointF (0.f, -length));
		});
	}
}


void
ADIWidget::PaintWorkUnit::al_paint_bugs (xf::Painter& painter, float x)
{
	if (_params.altitude_visible)
	{
		QFont altitude_bug_font = _font_10;
		float const altitude_bug_digit_height = _font_10_digit_height;

		painter.setClipping (false);
		painter.setTransform (_al_transform);
		painter.setFont (altitude_bug_font);

		for (auto& bug: _params.altitude_bugs)
		{
			if (bug.second > _al_min_shown && bug.second < _al_max_shown)
			{
				float posy = ft_to_px (bug.second);
				QRectF text_rect (-4.5f * x, posy - 0.5f * altitude_bug_digit_height,
								  +2.f * x, altitude_bug_digit_height);
				painter.setClipRect (_al_ladder_rect.adjusted (-x, 0.f, 0.f, 0.f));

				painter.setPen (_al_altitude_bug_pen);
				painter.add_shadow ([&] {
					painter.drawLine (QPointF (-1.5f * x, posy), QPointF (-2.25f * x, posy));
				});

				painter.setClipping (false);
				painter.fast_draw_text (text_rect, Qt::AlignVCenter | Qt::AlignRight, bug.first);
			}
		}

		// Altitude warning:
		if (_params.altitude_landing_visible)
		{
			QPointF p1 (-2.05f * x, ft_to_px (_params.altitude_landing_amsl + _params.altitude_landing_warning_lo));
			QPointF p2 (-2.05f * x, ft_to_px (_params.altitude_landing_amsl + _params.altitude_landing_warning_hi));
			QPointF p0 (-2.05f * x, ft_to_px (_params.altitude_landing_amsl));

			QPen w = _al_ldg_alt_pen;
			w.setColor (Qt::white);
			w.setCapStyle (Qt::SquareCap);

			painter.setClipRect (_al_ladder_rect.adjusted (-x, 0.f, 0.f, 0.f));
			painter.setPen (w);
			painter.add_shadow ([&] {
				painter.drawPolyline (QPolygonF() << p1 << p2 << p2 + QPointF (0.25f * x, 0.f));
			});
			painter.setPen (_al_ldg_alt_pen);
			painter.add_shadow ([&] {
				painter.drawLine (p0, p1);
			});

			// Landing altitude bug (ground indicator):
			if (_params.altitude_landing_amsl > _al_min_shown && _params.altitude_landing_amsl < _al_max_shown)
			{
				painter.setClipRect (_al_ladder_rect);
				float posy = ft_to_px (_params.altitude_landing_amsl);

				painter.setPen (_al_ldg_alt_pen);
				painter.drawLine (QPointF (+2.25f * x, posy), QPointF (-2.25f * x, posy));
				for (int i = -8; i <= 4; ++i)
				{
					QPointF p (0.4f * i * x + 0.125f * x, posy + 0.1f * x);
					painter.drawLine (p, p + QPointF (x, x));
				}
			}
		}

		// AP bug:
		if (_params.cmd_altitude)
		{
			Length cmd_altitude = xf::clamped<Length> (*_params.cmd_altitude, -99999_ft, +99999_ft);
			float posy = xf::clamped (ft_to_px (cmd_altitude),
									  static_cast<float> (-_al_ladder_rect.height() / 2), static_cast<float> (_al_ladder_rect.height() / 2));
			QPolygonF bug_shape = QPolygonF()
				<< QPointF (0.f, 0.f)
				<< QPointF (-0.5f * x, -0.5f * x)
				<< QPointF (-0.5f * x, _al_black_box_rect.top())
				<< QPointF (+1.3f * x, _al_black_box_rect.top())
				<< QPointF (+1.3f * x, _al_black_box_rect.bottom())
				<< QPointF (-0.5f * x, _al_black_box_rect.bottom())
				<< QPointF (-0.5f * x, +0.5f * x);
			painter.setClipRect (_al_ladder_rect.translated (-x, 0.f));
			painter.translate (-2.f * x, posy);
			painter.setBrush (Qt::NoBrush);
			painter.setPen (_autopilot_pen_1);
			painter.drawPolygon (bug_shape);
			painter.setPen (_autopilot_pen_2);
			painter.drawPolygon (bug_shape);
		}

		// Baro bug:
		if (_params.minimums_altitude_visible)
		{
			if (_params.minimums_amsl > _al_min_shown && _params.minimums_amsl < _al_max_shown)
			{
				if (!(_locals.minimums_blinking_active && !_locals.minimums_blink))
				{
					float posy = ft_to_px (_params.minimums_amsl);
					painter.setTransform (_al_transform);
					painter.setClipRect (_al_ladder_rect.adjusted (-2.5f * x, 0.f, 0.f, 0.f));
					QPen pen = get_pen (get_minimums_color(), 1.25f);
					pen.setMiterLimit (0.35f);
					painter.setPen (pen);
					painter.setBrush (Qt::NoBrush);
					QPointF a (_al_ladder_rect.left(), posy);
					QPointF b (_al_ladder_rect.left() - 0.65f * x, posy - 0.65f * x);
					QPointF c (_al_ladder_rect.left() - 0.65f * x, posy + 0.65f * x);
					QPolygonF poly = QPolygonF() << a << b << c;
					painter.add_shadow ([&] {
						painter.drawLine (a, QPointF (_al_ladder_rect.right(), posy));
						painter.drawPolygon (poly);
					});
				}
			}
		}
	}

	// Vertical speed bug:
	if (_params.cmd_vertical_speed && _params.vertical_speed_visible)
	{
		painter.setClipping (false);
		painter.setTransform (_al_transform);
		painter.translate (4.15f * x, 0.f);
		float posy = -8.f * x * scale_vertical_speed (*_params.cmd_vertical_speed);
		for (auto pen: { _autopilot_pen_1, _autopilot_pen_2 })
		{
			painter.setPen (pen);
			for (auto y: { posy - 0.2f * x, posy + 0.2f * x })
				painter.drawLine (QPointF (-0.25 * x, y), QPointF (0.2f * x, y));
		}
	}
}


void
ADIWidget::PaintWorkUnit::al_paint_vertical_speed (xf::Painter& painter, float x)
{
	QPen bold_white_pen = get_pen (Qt::white, 1.25f);
	QPen thin_white_pen = get_pen (Qt::white, 0.50f);

	float const y = x * 4.f;
	float const line_w = 0.2f * x;

	painter.setClipping (false);
	painter.setTransform (_al_transform);
	painter.translate (4.f * x, 0.f);

	painter.setPen (_al_ladder_pen);
	painter.setBrush (_ladder_color);
	painter.drawPolygon (QPolygonF()
		<< QPointF (0.0f, -0.6 * y)
		<< QPointF (-x, -0.6 * y - x)
		<< QPointF (-x, -1.9f * y - x)
		<< QPointF (+0.3f * x, -1.9f * y - x)
		<< QPointF (1.66f * x, -y - x)
		<< QPointF (1.66f * x, +y + x)
		<< QPointF (+0.3f * x, +1.9f * y + x)
		<< QPointF (-x, +1.9f * y + x)
		<< QPointF (-x, +0.6 * y + x)
		<< QPointF (0.0f, +0.6 * y));

	// Scale:
	painter.setFont (_font_10);
	painter.setPen (bold_white_pen);
	painter.draw_outlined_line (QPointF (0.f, 0.f), QPointF (0.5f * x, 0.f));
	for (float kfpm: { -6.f, -2.f, -1.f, +1.f, +2.f, +6.f })
	{
		float posy = -2.f * y * scale_vertical_speed (kfpm * 1000_fpm);
		QRectF num_rect (-1.55f * x, posy - x, 1.3f * x, 2.f * x);
		painter.draw_outlined_line (QPointF (0.f, posy), QPointF (line_w, posy));
		painter.fast_draw_text (num_rect, Qt::AlignVCenter | Qt::AlignRight, QString::number (std::abs (static_cast<int> (kfpm))));
	}
	painter.setPen (thin_white_pen);
	for (float kfpm: { -4.f, -1.5f, -0.5f, +0.5f, +1.5f, +4.f })
	{
		float posy = -2.f * y * scale_vertical_speed (kfpm * 1000_fpm);
		painter.draw_outlined_line (QPointF (0.f, posy), QPointF (line_w, posy));
	}

	// Variometer:
	if (_params.energy_variometer_visible)
	{
		painter.setClipping (false);
		float posy = -2.f * y * scale_energy_variometer (_params.energy_variometer_rate);
		float x = pen_width (2.0);
		painter.setPen (QPen (_navigation_color, pen_width (1.0)));
		painter.setBrush (_navigation_color);
		QPolygonF rhomb = QPolygonF()
			<< QPointF (0.f, +1.5f * x)
			<< QPointF (-x, 0.f)
			<< QPointF (0.f, -1.5f * x)
			<< QPointF (+x, 0.f)
			<< QPointF (0.f, +1.5f * x);
		painter.add_shadow ([&] {
			painter.drawPolyline (rhomb.translated (1.25f * x, posy));
		});
	}

	// TCAS
	painter.setPen (Qt::NoPen);
	painter.setBrush (Qt::red);

	auto paint_red_lines = [&] (si::Velocity speed1, si::Velocity speed2)
	{
		painter.setTransform (_al_transform);
		painter.translate (4.f * x, 0.f);
		float const s1 = -2.f * y * scale_vertical_speed (speed1, 1.015f);
		float const s2 = -2.f * y * scale_vertical_speed (speed2, 1.015f);
		float const ys = 0.875f;
		painter.add_shadow ([&] {
			painter.drawPolygon (QPolygonF()
				<< QPointF (0.35f * x, s1)
				<< QPointF (0.75f * x, ys * s1)
				<< QPointF (0.75f * x, ys * s2)
				<< QPointF (0.35f * x, s2));
		});
	};

	si::Velocity min_vspd = 5500_fpm;

	if (_params.tcas_ra_vertical_speed_minimum)
		paint_red_lines (*_params.tcas_ra_vertical_speed_minimum - 20000_fpm,
						 std::max (*_params.tcas_ra_vertical_speed_minimum, -min_vspd));

	if (_params.tcas_ra_vertical_speed_maximum)
		paint_red_lines (*_params.tcas_ra_vertical_speed_maximum + 20000_fpm,
						 std::min (*_params.tcas_ra_vertical_speed_maximum, +min_vspd));

	// Pointer:
	if (_params.vertical_speed_visible)
	{
		painter.setClipRect (QRectF (0.15f * x, -2.75f * y - x, (1.66f - 0.15f) * x, 5.5f * y + 2.f * x));
		QPen indicator_pen = bold_white_pen;
		indicator_pen.setCapStyle (Qt::FlatCap);
		painter.setPen (indicator_pen);
		painter.draw_outlined_line (QPointF (3.f * x, 0.f), QPointF (line_w, -2.f * y * scale_vertical_speed (_params.vertical_speed)));
	}

	// Numeric indicators above and below:
	painter.setPen (bold_white_pen);
	int abs_vertical_speed = static_cast<int> (std::abs (_params.vertical_speed.quantity<FootPerMinute>())) / 10 * 10;
	if (abs_vertical_speed >= 100)
	{
		QString str = QString::number (abs_vertical_speed);
		if (str.size() == 2)
			str = "  " + str;
		else if (str.size() == 3)
			str = " " + str;

		float const fh = _font_13_digit_height;
		float const sgn = _params.vertical_speed > 0_fpm ? 1.f : -1.f;
		painter.setClipping (false);
		painter.setFont (_font_13);
		painter.translate (-1.05f * x, sgn * -2.35f * y);
		painter.fast_draw_text (QRectF (0.f, -0.5f * fh, 4.f * fh, fh), Qt::AlignVCenter | Qt::AlignLeft, str);
	}
}


void
ADIWidget::PaintWorkUnit::al_paint_pressure (xf::Painter& painter, float x)
{
	if (!_params.pressure_visible)
		return;

	painter.setClipping (false);
	painter.setTransform (_al_transform);
	painter.translate (0.f, 0.75f * x);

	QFont font_a = _params.use_standard_pressure ? _font_13 : _font_16;
	QFont font_b = _font_13;
	QFontMetricsF metrics_a (font_a);
	QFontMetricsF metrics_b (font_b);

	QString unit_str = _params.pressure_display_hpa? " HPA" : " IN";
	int precision = _params.pressure_display_hpa ? 0 : 2;
	QString pressure_str = QString ("%1").arg (_params.pressure_display_hpa? _params.pressure_qnh.quantity<HectoPascal>() : _params.pressure_qnh.quantity<InchOfMercury>(), 0, 'f', precision);

	QRectF nn_rect (0.f, _al_ladder_rect.bottom(), metrics_a.width (pressure_str), 1.2f * _font_16_digit_height);
	QRectF zz_rect (0.f, nn_rect.top(), metrics_b.width (unit_str), nn_rect.height());
	nn_rect.moveLeft (-0.5f * (zz_rect.width() + nn_rect.width()));
	// Correct position of zz_rect to get correct baseline position:
	zz_rect.translate (0.f, metrics_b.descent() - metrics_a.descent());
	zz_rect.moveLeft (nn_rect.right());

	painter.setPen (QPen (_navigation_color, pen_width(), Qt::SolidLine, Qt::RoundCap));
	if (_params.use_standard_pressure)
	{
		painter.setFont (_font_16);
		painter.fast_draw_text (QPointF (0.5f * (nn_rect.left() + zz_rect.right()), nn_rect.bottom()), Qt::AlignHCenter | Qt::AlignBottom, "STD");
		painter.translate (0.f, 0.9f * metrics_a.height());
		painter.setPen (QPen (Qt::white, 1.f, Qt::SolidLine, Qt::RoundCap));
	}
	painter.setFont (font_a);
	painter.fast_draw_text (nn_rect, Qt::AlignBottom | Qt::AlignRight, pressure_str);
	painter.setFont (font_b);
	painter.fast_draw_text (zz_rect, Qt::AlignBottom | Qt::AlignLeft, unit_str);
}


void
ADIWidget::PaintWorkUnit::al_paint_ap_setting (xf::Painter& painter)
{
	if (!_params.cmd_altitude)
		return;

	Length cmd_altitude = xf::clamped<Length> (*_params.cmd_altitude, -99999_ft, +99999_ft);

	QFont b_font = _font_20;
	QFontMetricsF b_metrics (b_font);
	float const b_digit_width = _font_20_digit_width;
	float const b_digit_height = _font_20_digit_height;

	QFont s_font = _font_16;
	QFontMetricsF s_metrics (s_font);
	float const s_digit_width = _font_16_digit_width;

	QFont m_font = _font_13;
	QFontMetricsF m_metrics (m_font);

	int const b_digits = 2;
	int const s_digits = 3;
	float const margin = 0.2f * b_digit_width;

	QRectF b_digits_box (0.f, 0.f, b_digits * b_digit_width + margin, 1.3f * b_digit_height);
	QRectF s_digits_box (0.f, 0.f, s_digits * s_digit_width + margin, 1.3f * b_digit_height);
	QRectF box_rect (_al_ladder_rect.left(), _al_ladder_rect.top() - 1.4f * b_digits_box.height(),
					 b_digits_box.width() + s_digits_box.width(), b_digits_box.height());
	QRectF metric_rect (box_rect.topLeft() - QPointF (0.f, 1.25f * m_metrics.height()), box_rect.topRight());
	b_digits_box.translate (box_rect.left(), box_rect.top());
	s_digits_box.translate (b_digits_box.right(), b_digits_box.top());

	painter.setClipping (false);
	painter.setTransform (_al_transform);

	if (_params.show_metric)
	{
		painter.setPen (get_pen (Qt::black, 0.5f));
		painter.setBrush (QBrush (Qt::black));

		// Metric box:
		painter.drawRect (metric_rect);

		// Metric value:
		float xcorr = 0.25f * m_metrics.width (" ");
		QPointF m_pos (metric_rect.right() - 1.4f * m_metrics.width ("M"), metric_rect.center().y());
		painter.setPen (get_pen (QColor (0x00, 0xee, 0xff), 1.f));
		painter.setFont (m_font);
		painter.fast_draw_text (m_pos, Qt::AlignLeft | Qt::AlignVCenter, "M");
		painter.setPen (get_pen (_autopilot_color, 1.f));
		painter.fast_draw_text (m_pos + QPointF (-xcorr, 0.f), Qt::AlignRight | Qt::AlignVCenter,
								QString ("%1").arg (std::round (cmd_altitude.quantity<Meter>()), 0, 'f', 0));
	}

	painter.setPen (get_pen (Qt::black, 0.5f));
	painter.setBrush (QBrush (Qt::black));
	painter.drawRect (box_rect);

	if (_params.cmd_altitude_acquired)
	{
		float const z = 0.5f * margin;
		QRectF em_box_rect = box_rect.adjusted (-z, -z, z, z);

		painter.setBrush (Qt::NoBrush);
		for (QPen pen: { get_pen (Qt::black, 1.8f), get_pen (Qt::white, 1.4f) })
		{
			painter.setPen (pen);
			painter.drawRect (em_box_rect);
		}
	}

	painter.setPen (get_pen (_autopilot_color, 1.f));
	painter.setFont (b_font);

	// 11000 part of the altitude setting:
	QRectF box_11000 = b_digits_box.adjusted (margin, margin, 0.f, -margin);
	QString minus_sign_s = cmd_altitude < -0.5_ft ? MINUS_SIGN : "";
	painter.fast_draw_text (box_11000, Qt::AlignVCenter | Qt::AlignRight,
							minus_sign_s + QString::number (std::abs (xf::symmetric_round (cmd_altitude.quantity<Foot>()) / 1000)));

	painter.setFont (s_font);

	// 00111 part of the altitude setting:
	QRectF box_00111 = s_digits_box.adjusted (0.f, margin, -margin, -margin);
	painter.fast_draw_text (box_00111, Qt::AlignVCenter | Qt::AlignLeft,
							QString ("%1").arg (static_cast<int> (std::round (std::abs (cmd_altitude.quantity<Foot>()))) % 1000, 3, 'f', 0, '0'));
}


void
ADIWidget::PaintWorkUnit::al_paint_ldgalt_flag (xf::Painter& painter, float x)
{
	if (_params.ldgalt_flag)
	{
		painter.setClipping (false);
		painter.setTransform (_al_transform);
		painter.setPen (_warning_color_2);
		painter.setFont (_font_10);
		painter.fast_draw_text (QPoint (2.2f * x, 10.4f * x), Qt::AlignVCenter | Qt::AlignLeft, "LDG");
		painter.fast_draw_text (QPoint (2.2f * x, 10.4f * x + 1.1f * _font_13_digit_height), Qt::AlignVCenter | Qt::AlignLeft, "ALT");
	}
}


float
ADIWidget::PaintWorkUnit::scale_vertical_speed (si::Velocity vertical_speed, float max_value) const
{
	float vspd = std::abs (vertical_speed.quantity<FootPerMinute>());

	if (vspd < 1000.f)
		vspd = vspd / 1000.f * 0.46f;
	else if (vspd < 2000)
		vspd = 0.46f + 0.32f * (vspd - 1000.f) / 1000.f;
	else if (vspd < 6000)
		vspd = 0.78f + 0.22f * (vspd - 2000.f) / 4000.f;

	vspd = std::min (vspd, max_value);

	if (vertical_speed < 0_fpm)
		vspd *= -1.f;

	return vspd;
}


float
ADIWidget::PaintWorkUnit::scale_energy_variometer (Power power, float max_value) const
{
	si::Velocity equivalent_speed = power / _params.energy_variometer_1000_fpm_power * 1000_fpm;
	return scale_vertical_speed (equivalent_speed, max_value);
}


void
ADIWidget::PaintWorkUnit::paint_center_cross (xf::Painter& painter, bool center_box, bool rest)
{
	float const w = wh() * 3.f / 9.f;

	QPointF x (0.025f * w, 0.f);
	QPointF y (0.f, 0.025f * w);
	QPolygonF a = QPolygonF()
		<< -x - y
		<<  x - y
		<<  x + y
		<< -x + y;
	QPolygonF b = QPolygonF()
		<< -27.f * x - y
		<< -11.f * x - y
		<< -11.f * x + 4.f * y
		<< -13.f * x + 4.f * y
		<< -13.f * x + y
		<< -27.f * x + y;

	painter.setClipping (false);
	painter.setTransform (_center_transform);

	if (rest)
	{
		painter.setBrush (QBrush (QColor (0, 0, 0)));
		painter.setPen (Qt::NoPen);
		painter.drawPolygon (a);
		painter.setPen (get_pen (Qt::white, 1.5f));
		painter.add_shadow ([&] {
			painter.drawPolygon (b);
			painter.scale (-1.f, 1.f);
			painter.drawPolygon (b);
		});
	}

	if (center_box)
	{
		painter.setPen (get_pen (Qt::white, 1.5f));
		painter.setBrush (Qt::NoBrush);
		painter.add_shadow ([&] {
			painter.drawPolygon (a);
		});
	}
}


void
ADIWidget::PaintWorkUnit::paint_flight_director (xf::Painter& painter)
{
	using xf::sgn;
	using std::abs;

	float const w = wh() * 1.4f / 9.f;
	Angle range = _params.fov / 4.f;

	Angle pitch = si::cos (_params.orientation_roll) * (_params.flight_director_pitch - _params.orientation_pitch);
	pitch = xf::clamped (pitch, -range, +range);

	Angle roll = _params.flight_director_roll - _params.orientation_roll;
	if (abs (roll) > 180_deg)
		roll = roll - sgn (roll.quantity<Degree>()) * 360_deg;
	roll = xf::clamped (roll, -range, +range);

	float ypos = pitch_to_px (pitch);
	float xpos = heading_to_px (roll) / 2.f;

	painter.setClipping (false);
	painter.setTransform (_center_transform);

	for (auto pen: { get_pen (_autopilot_pen_1.color(), 2.3f),
					 get_pen (_autopilot_pen_2.color(), 1.65f) })
	{
		painter.setPen (pen);
		if (_params.flight_director_pitch_visible && _params.orientation_pitch_visible)
			painter.drawLine (QPointF (-w, ypos), QPointF (+w, ypos));
		if (_params.flight_director_roll_visible && _params.orientation_roll_visible)
			painter.drawLine (QPointF (xpos, -w), QPointF (xpos, +w));
	}
}


void
ADIWidget::PaintWorkUnit::paint_control_stick (xf::Painter& painter)
{
	if (!_params.control_stick_visible)
		return;

	float const w = wh() * 0.2f / 9.f;
	Angle range = 17.5_deg;

	Angle pitch = xf::clamped (_params.control_stick_pitch, -range, +range);
	Angle roll = xf::clamped (_params.control_stick_roll, -range, +range);

	float ypos = pitch_to_px (pitch);
	float xpos = heading_to_px (roll);

	painter.setClipping (false);
	painter.setTransform (_center_transform);

	// Four corners:
	double z = 0.25 * _q;
	QPolygonF corner = QPolygonF()
		<< QPointF (heading_to_px (-range), pitch_to_px (-range) - z)
		<< QPointF (heading_to_px (-range), pitch_to_px (-range))
		<< QPointF (heading_to_px (-range) + z, pitch_to_px (-range));
	for (auto pen: { get_pen (_navigation_color.darker (300), 2.25f),
					 get_pen (_navigation_color, 1.25f) })
	{
		painter.setPen (pen);
		painter.drawPolyline (corner);
		painter.scale (-1.0, +1.0);
		painter.drawPolyline (corner);
		painter.scale (+1.0, -1.0);
		painter.drawPolyline (corner);
		painter.scale (-1.0, +1.0);
		painter.drawPolyline (corner);
		painter.scale (+1.0, -1.0);
	}

	// Pointer:
	for (auto pen: { get_pen (_navigation_color.darker (300), 2.5f),
					 get_pen (_navigation_color, 1.5f) })
	{
		painter.setPen (pen);
		painter.drawLine (QPointF (xpos, ypos - w), QPointF (xpos, ypos + w));
		painter.drawLine (QPointF (xpos - w, ypos), QPointF (xpos + w, ypos));
	}
}


void
ADIWidget::PaintWorkUnit::paint_altitude_agl (xf::Painter& painter)
{
	if (!_params.altitude_agl_visible)
		return;

	Length aagl = xf::clamped<Length> (_params.altitude_agl, -9999_ft, +99999_ft);
	QFont radar_altimeter_font = _font_20;
	float const digit_width = _font_20_digit_width;
	float const digit_height = _font_20_digit_height;
	float const v = 0.03f * _q;

	int digits = 4;
	if (_params.altitude_agl > 9999_ft)
		digits = 5;
	float const margin = 0.2f * digit_width;

	QRectF box_rect (0.f, 0.f, digits * digit_width + 2.f * margin, 1.3f * digit_height);
	box_rect.translate (-box_rect.width() / 2.f, 0.35f * wh());
	box_rect.adjust (-v, -v, +v, +v);

	painter.setClipping (false);
	painter.setTransform (_center_transform);
	if (is_newly_set (_locals.altitude_agl_ts))
		painter.setPen (get_pen (Qt::white, 1.25f));
	else
		painter.setPen (Qt::NoPen);
	painter.setBrush (QBrush (Qt::black));
	painter.drawRect (box_rect);

	painter.setPen (get_pen (Qt::white, 1.f));
	painter.setFont (radar_altimeter_font);

	QRectF box = box_rect.adjusted (margin, margin, -margin, -margin);
	painter.fast_draw_text (box, Qt::AlignVCenter | Qt::AlignHCenter, QString ("%1").arg (std::round (aagl.quantity<Foot>())));
}


void
ADIWidget::PaintWorkUnit::paint_minimums_setting (xf::Painter& painter)
{
	if (!_params.minimums_altitude_visible)
		return;

	float x = 0.18f * wh();

	painter.setClipping (false);
	painter.setTransform (_center_transform);

	QFont font_a = _font_10;
	QFont font_b = _font_16;
	QFontMetricsF metrics_a (font_a);
	QFontMetricsF metrics_b (font_b);

	QString mins_str = _params.minimums_type;
	QString alt_str = QString ("%1").arg (_params.minimums_setting.quantity<Foot>(), 0, 'f', 0);

	QRectF mins_rect (1.35f * x, 1.8f * x, metrics_a.width (mins_str), metrics_a.height());
	mins_rect.moveRight (mins_rect.left());
	QRectF alt_rect (0.f, 0.f, metrics_b.width (alt_str), metrics_b.height());
	alt_rect.moveTopRight (mins_rect.bottomRight());

	QPen minimums_pen = get_pen (get_minimums_color(), 1.f);

	if (!(_locals.minimums_blinking_active && !_locals.minimums_blink))
	{
		painter.setPen (minimums_pen);
		painter.setFont (font_a);
		painter.fast_draw_text (mins_rect, Qt::AlignVCenter | Qt::AlignRight, mins_str);
		painter.setFont (font_b);
		painter.fast_draw_text (alt_rect, Qt::AlignVCenter | Qt::AlignRight, alt_str);
	}

	if (is_newly_set (_locals.minimums_altitude_ts))
	{
		float v = 0.06f * _q;
		QRectF frame = alt_rect.united (mins_rect).adjusted (-2.f * v, -0.75f * v, +2.f * v, 0.f);
		painter.setPen (minimums_pen);
		painter.setBrush (Qt::NoBrush);
		painter.add_shadow ([&] {
			painter.drawRect (frame);
		});
	}
}


void
ADIWidget::PaintWorkUnit::paint_nav (xf::Painter& painter)
{
	using std::abs;

	painter.setClipping (false);
	painter.setTransform (_center_transform);

	if (_params.navaid_reference_visible)
	{
		QString loc_str = _params.navaid_identifier;
		if (_params.navaid_course_magnetic)
		{
			int course_int = xf::symmetric_round (_params.navaid_course_magnetic->quantity<Degree>());
			if (course_int == 0)
				course_int = 360;
			loc_str += QString::fromStdString ((boost::format ("/%03d°") % course_int).str());
		}

		painter.setPen (Qt::white);
		painter.setFont (_font_10);
		painter.fast_draw_text (QPointF (-0.24f * wh(), -0.3925f * wh()), Qt::AlignTop | Qt::AlignLeft, loc_str);

		if (_params.navaid_hint != "")
		{
			painter.setPen (Qt::white);
			painter.setFont (_font_16);
			painter.fast_draw_text (QPointF (-0.24f * wh(), -0.32f * wh()), Qt::AlignTop | Qt::AlignLeft, _params.navaid_hint);
		}

		QString dme_val = "DME ---";
		if (_params.navaid_distance)
			dme_val = QString::fromStdString ((boost::format ("DME %.1f") % _params.navaid_distance->quantity<NauticalMile>()).str());

		painter.setPen (Qt::white);
		painter.setFont (_font_10);
		painter.fast_draw_text (QPointF (-0.24f * wh(), -0.36f * wh()), Qt::AlignTop | Qt::AlignLeft, dme_val);

		QPen ladder_pen (_ladder_border_color, pen_width (0.75f), Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin);

		auto paint_ladder = [&](std::optional<Angle> original_approach_deviation, std::optional<Angle> original_path_deviation) -> void
		{
			Angle approach_deviation;
			if (original_approach_deviation)
				approach_deviation = xf::clamped<Angle> (*original_approach_deviation, -2.25_deg, +2.25_deg);
			Angle path_deviation;
			if (original_path_deviation)
				path_deviation = xf::clamped<Angle> (*original_path_deviation, -2.25_deg, +2.25_deg);

			QRectF rect (0.f, 0.f, 0.385f * wh(), 0.055f * wh());
			centrify (rect);

			QRectF elli (0.f, 0.f, 0.015f * wh(), 0.015f * wh());
			centrify (elli);

			if (!_params.old_style)
			{
				painter.setPen (ladder_pen);
				painter.setBrush (_ladder_color);
				painter.drawRect (rect);
			}

			QPolygonF pink_pointer;
			QPolygonF white_pointer;
			bool pink_filled = false;
			bool pink_visible = false;
			bool white_visible = false;

			if (!_params.deviation_mixed_mode)
			{
				// Only ILS:
				float w = 0.012f * wh();
				pink_pointer = QPolygonF()
					<< QPointF (0.f, -w)
					<< QPointF (+1.6f * w, 0.f)
					<< QPointF (0.f, +w)
					<< QPointF (-1.6f * w, 0.f)
					<< QPointF (0.f, -w);
				pink_pointer.translate (approach_deviation.quantity<Degree>() * 0.075f * wh(), 0.f);
				pink_visible = !!original_approach_deviation;
				pink_filled = original_approach_deviation && abs (*original_approach_deviation) <= abs (approach_deviation);
				white_visible = false;
			}
			else
			{
				// ILS and flight path:
				float w = 0.012f * wh();
				pink_pointer = QPolygonF()
					<< QPointF (0.f, -0.2f * w)
					<< QPointF (+1.0f * w, 2.0f * w)
					<< QPointF (-1.0f * w, 2.0f * w);
				pink_pointer.translate (path_deviation.quantity<Degree>() * 0.075f * wh(), 0.f);
				pink_visible = !!original_path_deviation;
				pink_filled = original_path_deviation && abs (*original_path_deviation) <= abs (path_deviation);
				white_pointer = QPolygonF()
					<< QPointF (0.f, -0.8f * w)
					<< QPointF (+1.6f * w, 0.f)
					<< QPointF (0.f, +0.8f * w)
					<< QPointF (-1.6f * w, 0.f)
					<< QPointF (0.f, -0.8f * w);
				white_pointer.translate (approach_deviation.quantity<Degree>() * 0.075f * wh(), -0.65f * w);
				white_visible = !!original_approach_deviation;
			}

			if (pink_visible)
			{
				for (QColor color: { _autopilot_pen_1.color(), _autopilot_pen_2.color() })
				{
					painter.setPen (get_pen (color, 1.f));
					if (pink_filled)
						painter.setBrush (color);
					else
						painter.setBrush (Qt::NoBrush);
					painter.drawPolygon (pink_pointer);
				}
			}

			if (white_visible)
			{
				painter.setPen (get_pen (Qt::white, 1.f));
				painter.setBrush (Qt::NoBrush);
				painter.drawPolyline (white_pointer);
			}

			if (!_params.deviation_mixed_mode)
			{
				// Paint ILS deviation scale:
				painter.setPen (get_pen (Qt::white, 1.5f));
				painter.setBrush (Qt::NoBrush);
				for (float x: { -1.f, -0.5f, +0.5f, +1.f })
					painter.drawEllipse (elli.translated (0.15f * wh() * x, 0.f));
				painter.draw_outlined_line (QPointF (0.f, -rect.height() / 3.f), QPointF (0.f, +rect.height() / 3.f));
			}
			else
			{
				// Paint path deviation scale:
				painter.setPen (get_pen (Qt::white, 1.2f));
				painter.setBrush (Qt::NoBrush);
				for (float x: { -1.f, +1.f })
				{
					float sx = 0.15f * wh() * x;
					painter.draw_outlined_line (QPointF (sx, -rect.height() / 2.75f), QPointF (sx, +rect.height() / 8.f));
				}
				painter.draw_outlined_line (QPointF (0.f, -rect.height() / 2.1f), QPointF (0.f, +rect.height() / 6.f));
			}
		};

		painter.setTransform (_center_transform);
		painter.translate (0.f, 0.452f * wh());
		if (_params.deviation_lateral_failure)
			paint_horizontal_failure_flag (painter, QPointF (0.f, 0.f), font_size (18.f), "LOC");
		else
			paint_ladder (_params.deviation_lateral_approach, _params.deviation_lateral_flight_path);

		painter.setTransform (_center_transform);
		painter.translate (0.28f * wh(), 0.f);
		if (_params.deviation_vertical_failure)
			paint_vertical_failure_flag (painter, QPointF (0.f, 0.f), font_size (18.f), "G/S");
		else
		{
			painter.rotate (-90);
			paint_ladder (_params.deviation_vertical_approach, _params.deviation_vertical_flight_path);
		}
	}

	if (_params.runway_visible && !_params.deviation_lateral_failure && _params.deviation_lateral_approach)
	{
		float w = 0.15f * wh();
		float h = 0.05f * wh();
		float p = 1.3f;
		float offset = 0.5f * xf::clamped (_params.deviation_lateral_approach->quantity<Degree>(), -1.5, +1.5);
		float ypos = -pitch_to_px (xf::clamped<Angle> (_params.runway_position + 3.5_deg, 3.5_deg, 25_deg));

		painter.setTransform (_center_transform);
		painter.translate (0.f, ypos);

		QPointF tps[] = { QPointF (-w, 0.f), QPointF (0.f, 0.f), QPointF (+w, 0.f) };
		QPointF bps[] = { QPointF (-w * p, h), QPointF (0.f, h), QPointF (+w * p, h) };

		for (QPointF& point: tps)
			point += QPointF (2.5f * w * offset, 0);
		for (QPointF& point: bps)
			point += QPointF (2.5f * p * w * offset, 0);

		painter.setClipRect (QRectF (-1.675f * w, -0.2f * h, 3.35f * w, 1.4f * h));

		QPolygonF runway = QPolygonF() << tps[0] << tps[2] << bps[2] << bps[0];

		painter.setBrush (Qt::NoBrush);
		for (auto pen: { QPen (_navigation_color.darker (400), pen_width (2.f)),
						 QPen (_navigation_color, pen_width (1.33f)) })
		{
			pen.setCapStyle (Qt::RoundCap);
			painter.setPen (pen);
			painter.drawPolygon (runway);

			pen.setCapStyle (Qt::FlatCap);
			painter.setPen (pen);
			painter.drawLine (tps[1], bps[1]);
		}
	}
}


void
ADIWidget::PaintWorkUnit::paint_hints (xf::Painter& painter)
{
	float const q = 0.1f * wh();

	if (_params.control_hint_visible)
	{
		painter.setClipping (false);
		painter.setTransform (_center_transform);
		painter.setFont (_font_20);
		painter.setBrush (Qt::NoBrush);
		painter.setPen (get_pen (_navigation_color, 1.0));
		QPointF text_hook = QPointF (0.f, -3.1f * q);
		painter.fast_draw_text (text_hook, Qt::AlignVCenter | Qt::AlignHCenter, _params.control_hint);

		if (is_newly_set (_locals.control_hint_ts))
		{
			float a = 0.055f * _q;
			QRectF frame (text_hook, QSizeF (2.25f * _q, _font_20_digit_height));
			centrify (frame);
			frame.adjust (0.f, -a, 0.f, +a);
			painter.add_shadow ([&] {
				painter.drawRect (frame);
			});
		}
	}

	if (_params.fma_visible)
	{
		QRectF rect (0.f, 0.f, 6.3f * q, 0.65f * q);
		centrify (rect);

		float x16 = rect.left() + 1.f / 6.f * rect.width();
		float x26 = rect.left() + 2.f / 6.f * rect.width();
		float x36 = rect.left() + 3.f / 6.f * rect.width();
		float x46 = rect.left() + 4.f / 6.f * rect.width();
		float x56 = rect.left() + 5.f / 6.f * rect.width();
		float y13 = rect.top() + 8.5f / 30.f * rect.height();
		float y23 = rect.top() + 23.5f / 30.f * rect.height();

		QPointF b1 = QPointF (x16, y13);
		QPointF b2 = QPointF (x36, y13);
		QPointF b3 = QPointF (x56, y13);

		QPointF s1 = QPointF (x16, y23);
		QPointF s2 = QPointF (x36, y23);
		QPointF s3 = QPointF (x56, y23);

		auto paint_big_rect = [&](QPointF point) -> void
		{
			float v = 0.03f * _q;
			QRectF frame (point, QSizeF (1.9f * _q, _font_13_digit_height));
			centrify (frame);
			frame.adjust (0.f, -v, 0.f, +v);
			painter.drawRect (frame);
		};

		auto paint_armed_rect = [&](QPointF point) -> void
		{
			float v = 0.025f * _q;
			QRectF frame (point, QSizeF (1.9f * _q, _font_10_digit_height));
			centrify (frame);
			frame.adjust (0.f, -v, 0.f, +v);
			painter.drawRect (frame);
		};

		painter.setClipping (false);
		painter.setTransform (_center_transform);
		painter.translate (0.f, -4.575f * q);
		painter.setPen (QPen (_ladder_border_color, pen_width (0.75f), Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin));
		painter.setBrush (_ladder_color);
		painter.drawRect (rect);
		painter.setPen (get_pen (Qt::white, 1.2f));
		painter.drawLine (QPointF (x26, rect.top()), QPointF (x26, rect.bottom()));
		painter.drawLine (QPointF (x46, rect.top()), QPointF (x46, rect.bottom()));
		painter.setBrush (Qt::NoBrush);

		painter.setPen (get_pen (_navigation_color, 1.0f));
		if (is_newly_set (_locals.fma_speed_ts))
			paint_big_rect (b1);
		if (is_newly_set (_locals.fma_lateral_ts))
			paint_big_rect (b2);
		if (is_newly_set (_locals.fma_vertical_ts))
			paint_big_rect (b3);

		painter.setPen (get_pen (Qt::white, 1.0f));
		if (is_newly_set (_locals.fma_speed_armed_ts))
			paint_armed_rect (s1);
		if (is_newly_set (_locals.fma_lateral_armed_ts))
			paint_armed_rect (s2);
		if (is_newly_set (_locals.fma_vertical_armed_ts))
			paint_armed_rect (s3);

		QFont font_big = _font_16;
		font_big.setPixelSize (font_size (14.f));
		QFont font_small = _font_13;

		painter.setPen (get_pen (_navigation_color, 1.0f));
		painter.setFont (font_big);
		painter.fast_draw_text (b1, Qt::AlignVCenter | Qt::AlignHCenter, _params.fma_speed_hint);
		painter.fast_draw_text (b2, Qt::AlignVCenter | Qt::AlignHCenter, _params.fma_lateral_hint);
		painter.fast_draw_text (b3, Qt::AlignVCenter | Qt::AlignHCenter, _params.fma_vertical_hint);

		painter.setPen (get_pen (Qt::white, 1.0f));
		painter.setFont (font_small);
		painter.fast_draw_text (s1, Qt::AlignVCenter | Qt::AlignHCenter, _params.fma_speed_armed_hint);
		painter.fast_draw_text (s2, Qt::AlignVCenter | Qt::AlignHCenter, _params.fma_lateral_armed_hint);
		painter.fast_draw_text (s3, Qt::AlignVCenter | Qt::AlignHCenter, _params.fma_vertical_armed_hint);
	}
}


void
ADIWidget::PaintWorkUnit::paint_critical_aoa (xf::Painter& painter)
{
	if (!_params.critical_aoa_visible || !_params.orientation_pitch_visible)
		return;

	painter.setClipping (false);
	painter.setTransform (_center_transform);
	painter.translate (0.f, pitch_to_px (xf::clamped<Angle> (_params.critical_aoa - _params.aoa_alpha, -20_deg, +16_deg)));

	float const w = wh() * 3.f / 9.f;

	QPointF x (0.025f * w, 0.f);
	QPointF y (0.f, 0.025f * w);

	auto paint = [&](QColor color, float pen_width_scale) -> void
	{
		painter.setPen (get_pen (color, pen_width_scale * 2.f));
		painter.drawPolyline (QPolygonF()
			<< -11.f * x + y
			<< -11.f * x - y
			<< -17.f * x - y
		);
		QPen pen = get_pen (color, pen_width_scale * 1.5f);
		pen.setCapStyle (Qt::FlatCap);
		painter.setPen (pen);
		painter.drawLine (-12.5f * x - y, -14.f * x - 3.65f * y);
		painter.drawLine (-14.f * x - y, -15.5f * x - 3.65f * y);
		painter.drawLine (-15.5f * x - y, -17.f * x - 3.65f * y);
	};

	paint (painter.shadow_color(), 1.25f);
	paint (_warning_color_2, 0.9f);
	painter.scale (-1.f, 1.f);
	paint (painter.shadow_color(), 1.25f);
	paint (_warning_color_2, 0.9f);
}


void
ADIWidget::PaintWorkUnit::paint_input_alert (xf::Painter& painter)
{
	QFont font = _font;
	font.setPixelSize (font_size (30.f));

	QString alert = "NO INPUT";

	QFontMetricsF font_metrics (font);
	int width = font_metrics.width (alert);

	QPen pen = get_pen (Qt::white, 2.f);

	painter.setClipping (false);

	painter.setTransform (_center_transform);
	painter.setPen (Qt::NoPen);
	painter.setBrush (Qt::black);
	painter.drawRect (QRect (QPoint (0, 0), size()));

	painter.setTransform (_center_transform);
	painter.setPen (pen);
	painter.setBrush (QBrush (QColor (0xdd, 0, 0)));
	painter.setFont (font);

	QRectF rect (-0.6f * width, -0.5f * font_metrics.height(), 1.2f * width, 1.2f * font_metrics.height());

	painter.drawRect (rect);
	painter.fast_draw_text (rect, Qt::AlignVCenter | Qt::AlignHCenter, alert);
}


void
ADIWidget::PaintWorkUnit::paint_dashed_zone (xf::Painter& painter, QColor const& color, QRectF const& target)
{
	QFontMetricsF metrics (painter.font());
	float w = 0.7f * metrics.width ("0");
	float h = 0.55f * metrics.height();
	QPointF center = target.center();
	QRectF box (center - QPointF (w / 2.f, h / 1.9f), QSizeF (w, h));
	QPen pen = get_pen (color, 1.2f);
	QPointF difx (box.width() / 2.5f, 0.f);
	QPointF dify (0.f, box.height() / 2.5f);
	pen.setCapStyle (Qt::RoundCap);
	painter.save();
	painter.setPen (pen);
	painter.drawLine (box.topLeft(), box.bottomRight());
	painter.drawLine (box.topLeft() + difx, box.bottomRight() - dify);
	painter.drawLine (box.topLeft() + dify, box.bottomRight() - difx);
	painter.drawLine (box.topLeft() + 2.f * difx, box.bottomRight() - 2.f * dify);
	painter.drawLine (box.topLeft() + 2.f * dify, box.bottomRight() - 2.f * difx);
	painter.restore();
}


void
ADIWidget::PaintWorkUnit::adi_paint_attitude_failure (xf::Painter& painter)
{
	painter.setClipping (false);
	painter.setTransform (_center_transform);
	paint_horizontal_failure_flag (painter, QPointF (0.f, -0.055f * wh()), font_size (20.f), "ATT");
}


void
ADIWidget::PaintWorkUnit::adi_paint_flight_path_marker_failure (xf::Painter& painter)
{
	painter.setClipping (false);
	painter.setTransform (_center_transform);
	// On Boeing EFIS FPM is called FPV - Flight Path Vector:
	paint_horizontal_failure_flag (painter, QPointF (-0.175f * wh(), -0.075f * wh()), font_size (18.f), "FPV");
}


void
ADIWidget::PaintWorkUnit::adi_paint_flight_director_falure (xf::Painter& painter)
{
	painter.setClipping (false);
	painter.setTransform (_center_transform);
	paint_horizontal_failure_flag (painter, QPointF (+0.2f * wh(), -0.075f * wh()), font_size (18.f), "FD");
}


void
ADIWidget::PaintWorkUnit::sl_paint_failure (xf::Painter& painter)
{
	painter.setClipping (false);
	painter.setTransform (_sl_transform);
	paint_vertical_failure_flag (painter, QPointF (0.f, 0.f), font_size (20.f), "SPD");
}


void
ADIWidget::PaintWorkUnit::al_paint_vertical_speed_failure (xf::Painter& painter, float x)
{
	painter.setClipping (false);
	painter.setTransform (_al_transform);
	painter.translate (4.f * x, 0.f);
	paint_vertical_failure_flag (painter, QPointF (0.f, 0.f), font_size (20.f), "VERT");
}


void
ADIWidget::PaintWorkUnit::al_paint_failure (xf::Painter& painter)
{
	painter.setClipping (false);
	painter.setTransform (_al_transform);
	paint_vertical_failure_flag (painter, QPointF (0.f, 0.f), font_size (20.f), "ALT");
}


void
ADIWidget::PaintWorkUnit::paint_radar_altimeter_failure (xf::Painter& painter)
{
	painter.setClipping (false);
	painter.setTransform (_center_transform);
	paint_horizontal_failure_flag (painter, QPointF (0.f, 0.35f * wh()), font_size (20.f), " RA ");
}


void
ADIWidget::PaintWorkUnit::paint_rotating_value (xf::Painter& painter,
								  QRectF const& rect, float position, float height_scale,
								  QString const& next, QString const& curr, QString const& prev)
{
	QColor red (255, 0, 0);
	QColor green (0, 255, 0);

	QFont font = painter.font();
	QFontMetricsF font_metrics (font);
	float height = height_scale * font_metrics.height();

	QRectF box_next = rect.translated (0.f, -height);
	QRectF box_prev = rect.translated (0.f, +height);

	painter.save();
	painter.setClipRect (rect);
	painter.translate (0.f, -height * position);

	for (std::pair<QRectF, QString> x: { std::make_pair (box_next, next),
										 std::make_pair (rect, curr),
										 std::make_pair (box_prev, prev) })
	{
		if (x.second == "G" || x.second == "R")
			paint_dashed_zone (painter, x.second == "G" ? green : red, x.first);
		else if (x.second == "-")
			; // Paint nothing.
		else
			painter.fast_draw_text (x.first, Qt::AlignVCenter | Qt::AlignLeft, x.second);
	}

	painter.restore();
}


void
ADIWidget::PaintWorkUnit::paint_rotating_digit (xf::Painter& painter,
								  QRectF const& box, float value, int round_target, float const height_scale, float const delta, float const phase,
								  bool two_zeros, bool zero_mark, bool black_zero)
{
	auto round_to = [] (float value, int to) -> float
	{
		float sgn = value >= 0.f ? +1.f : -1.f;
		return static_cast<int> (value + sgn * to / 2.f) / to * to;
	};

	float rounded = round_to (value + phase, round_target);
	float dtr = (value + phase - rounded) / round_target;
	float pos = 0.f;
	float epsilon = 0.000001f;
	float xa = std::fmod ((value + phase) / round_target + 1.f - epsilon, 10.f);
	float xb = std::fmod ((value + phase) / round_target + 0.f - epsilon, 10.f);
	float xc = std::fmod ((value + phase) / round_target - 1.f - epsilon, 10.f);

	int a = static_cast<int> (std::abs (xa));
	int b = static_cast<int> (std::abs (xb));
	int c = static_cast<int> (std::abs (xc));

	QString sa = zero_mark && a == 0 ? (black_zero ? "-" : (xa >= 0.f ? "G" : "R")) : QString::number (a);
	QString sb = zero_mark && b == 0 ? (black_zero ? "-" : (xb >= 0.f ? "G" : "R")) : QString::number (b);
	QString sc = zero_mark && c == 0 ? (black_zero ? "-" : (xc >= 0.f ? "G" : "R")) : QString::number (c);

	if (std::abs (dtr) < delta && (two_zeros || std::abs (value) >= round_target / 2))
		pos = xf::floored_mod (-dtr * (0.5f / delta), 1.f) - 0.5f;

	paint_rotating_value (painter, box, pos, height_scale, sa, sb, sc);
}


void
ADIWidget::PaintWorkUnit::paint_horizontal_failure_flag (xf::Painter& painter, QPointF const& center, float pixel_font_size, QString const& message)
{
	QFont font = _font_10;
	font.setPixelSize (pixel_font_size);
	QFontMetricsF metrics (font);

	QRectF box (0.f, 0.f, metrics.width (message) + 0.65f * metrics.width ("0"), metrics.height());
	centrify (box);
	box.translate (center);

	painter.setPen (get_pen (_warning_color_2, 1.f));
	painter.setFont (font);
	painter.setBrush (Qt::black);

	painter.add_shadow ([&] {
		painter.drawRect (box);
	});
	painter.fast_draw_text (center, Qt::AlignHCenter | Qt::AlignVCenter, message);
}


void
ADIWidget::PaintWorkUnit::paint_vertical_failure_flag (xf::Painter& painter, QPointF const& center, float pixel_font_size, QString const& message)
{
	QFont font = _font_10;
	font.setPixelSize (pixel_font_size);

	float const digit_width = 1.6f * get_digit_width (font);
	float const digit_height = 1.f * QFontMetricsF (font).height();

	QRectF box (0.f, 0.f, 1.f * digit_width, message.size() * digit_height);
	centrify (box);
	box.translate (center);

	painter.setPen (get_pen (_warning_color_2, 1.f));
	painter.setFont (font);
	painter.setBrush (Qt::black);

	painter.add_shadow ([&] {
		painter.drawRect (box);
	});
	QPointF top_letter = center + QPointF (0.f, -0.5f * digit_height * (message.size() - 1));
	for (int i = 0; i < message.size(); ++i)
		painter.fast_draw_text (top_letter + QPointF (0.f, i * digit_height), Qt::AlignHCenter | Qt::AlignVCenter, message[i]);
}


ADIWidget::ADIWidget (QWidget* parent, xf::WorkPerformer* work_performer):
	InstrumentWidget (parent, work_performer),
	_local_paint_work_unit (this)
{
	setAttribute (Qt::WA_NoBackground);

	_speed_blinking_warning = new QTimer (this);
	_speed_blinking_warning->setInterval (200);
	QObject::connect (_speed_blinking_warning, SIGNAL (timeout()), this, SLOT (blink_speed()));

	_minimums_blinking_warning = new QTimer (this);
	_minimums_blinking_warning->setInterval (200);
	QObject::connect (_minimums_blinking_warning, SIGNAL (timeout()), this, SLOT (blink_minimums()));

	_locals.minimums_altitude_ts = QDateTime::currentDateTime();

	set_painter (&_local_paint_work_unit);
}


ADIWidget::~ADIWidget()
{
	wait_for_painter();
}


void
ADIWidget::set_params (Parameters const& new_params)
{
	_params = new_params;
	_params.sanitize();
	request_repaint();
}


void
ADIWidget::resizeEvent (QResizeEvent* event)
{
	InstrumentWidget::resizeEvent (event);

	auto xw = dynamic_cast<v1::Window*> (window());
	if (xw)
		_local_paint_work_unit.set_scaling (xw->pen_scale(), xw->font_scale());
}


void
ADIWidget::request_repaint()
{
	update_blinker (_speed_blinking_warning,
					_params.speed_visible &&
					((_params.speed_minimum_visible && _params.speed < _params.speed_minimum) ||
					 (_params.speed_maximum_visible && _params.speed > _params.speed_maximum)),
					&_locals.speed_blink);

	update_blinker (_minimums_blinking_warning,
					_params.altitude_visible && _params.minimums_altitude_visible &&
					_params.altitude < _params.minimums_amsl &&
					_local_paint_work_unit.is_newly_set (_locals.minimums_altitude_ts, 5_s),
					&_locals.minimums_blink);

	InstrumentWidget::request_repaint();
}


void
ADIWidget::push_params()
{
	QDateTime now = QDateTime::currentDateTime();

	Parameters old = _local_paint_work_unit._params_next;

	if (_params.minimums_amsl < old.altitude && _params.altitude < _params.minimums_amsl)
		_locals.minimums_altitude_ts = now;

	if (_params.altitude_agl_visible && !old.altitude_agl_visible)
		_locals.altitude_agl_ts = now;

	if (_params.minimums_altitude_visible != old.minimums_altitude_visible)
		_locals.minimums_altitude_ts = now;

	if (_params.control_hint != old.control_hint)
		_locals.control_hint_ts = now;

	if (_params.control_hint_visible != old.control_hint_visible)
		_locals.control_hint_ts = now;

	if (_params.fma_speed_hint != old.fma_speed_hint)
		_locals.fma_speed_ts = now;

	if (_params.fma_speed_armed_hint != old.fma_speed_armed_hint)
		_locals.fma_speed_armed_ts = now;

	if (_params.fma_lateral_hint != old.fma_lateral_hint)
		_locals.fma_lateral_ts = now;

	if (_params.fma_lateral_armed_hint != old.fma_lateral_armed_hint)
		_locals.fma_lateral_armed_ts = now;

	if (_params.fma_vertical_hint != old.fma_vertical_hint)
		_locals.fma_vertical_ts = now;

	if (_params.fma_vertical_armed_hint != old.fma_vertical_armed_hint)
		_locals.fma_vertical_armed_ts = now;

	_locals.speed_blinking_active = _speed_blinking_warning->isActive();
	_locals.minimums_blinking_active = _minimums_blinking_warning->isActive();
	_local_paint_work_unit._params_next = _params;
	_local_paint_work_unit._locals_next = _locals;
}


void
ADIWidget::update_blinker (QTimer* warning_timer, bool condition, bool* blink_state)
{
	if (condition)
	{
		if (!warning_timer->isActive())
		{
			warning_timer->start();
			*blink_state = true;
		}
	}
	else if (warning_timer->isActive())
		warning_timer->stop();
}


void
ADIWidget::blink_speed()
{
	_locals.speed_blink = !_locals.speed_blink;
}


void
ADIWidget::blink_minimums()
{
	_locals.minimums_blink = !_locals.minimums_blink;
}


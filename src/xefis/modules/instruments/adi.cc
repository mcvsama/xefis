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
#include "adi.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/instrument/text_layout.h>

// Neutrino:
#include <neutrino/numeric.h>
#include <neutrino/si/utils.h>

// Standard:
#include <cstddef>
#include <algorithm>


namespace adi_detail {

void
Parameters::sanitize()
{
	vl_line_every = std::max (vl_line_every, 1);
	vl_number_every = std::max (vl_number_every, 1);
	vl_extent = std::max<si::Velocity> (vl_extent, 1_kt);
	vl_minimum = std::max (vl_minimum, 0);
	vl_maximum = std::min (vl_maximum, 9999);
	al_line_every = std::max (al_line_every, 1);
	al_number_every = std::max (al_number_every, 1);
	al_emphasis_every = std::max (al_emphasis_every, 1);
	al_bold_every = std::max (al_bold_every, 1);
	al_extent = std::max<si::Length> (al_extent, 1_ft);

	// Set orientation angles to range -180…180°:
	{
		std::optional<si::Angle> p;
		std::optional<si::Angle> r;

		if (this->orientation_pitch)
			p = xf::floored_mod (*this->orientation_pitch + 180_deg, 360_deg) - 180_deg;

		if (this->orientation_roll)
			r = xf::floored_mod (*this->orientation_roll + 180_deg, 360_deg) - 180_deg;

		if (p && r)
		{
			// Mirroring, eg. -180° pitch is the same
			// as 0° pitch with roll inverted:
			if (*p < -90_deg)
			{
				p = -180_deg - *p;
				r = +180_deg - *r;
			}
			else if (*p > 90_deg)
			{
				p = +180_deg - *p;
				r = +180_deg - *r;
			}

			this->orientation_pitch = p;
			this->orientation_roll = r;
		}

		if (this->orientation_heading)
			this->orientation_heading = xf::floored_mod (*this->orientation_heading, 360_deg);
	}

	// Limit FPM position:
	{
		if (this->flight_path_alpha)
			this->flight_path_alpha = xf::clamped<si::Angle> (*this->flight_path_alpha, -25.0_deg, +25.0_deg);

		if (this->flight_path_beta)
			this->flight_path_beta = xf::clamped<si::Angle> (*this->flight_path_beta, -25.0_deg, +25.0_deg);
	}

	// Speed limits:
	{
		if (this->speed)
		{
			this->speed = std::clamp<si::Velocity> (*this->speed, 1_kt * this->vl_minimum, 1_kt * this->vl_maximum);
			this->speed = std::clamp<si::Velocity> (*this->speed, 0_kt, 9999.99_kt);
		}

		if (this->speed_mach)
			this->speed_mach = xf::clamped (*this->speed_mach, 0.0, 9.99);

		if (this->speed_minimum)
			this->speed_minimum = xf::clamped<si::Velocity> (*this->speed_minimum, 0.0_kt, 9999.99_kt);

		if (this->speed_minimum_maneuver)
			this->speed_minimum_maneuver = xf::clamped<si::Velocity> (*this->speed_minimum_maneuver, 0.0_kt, 9999.99_kt);

		if (this->speed_maximum_maneuver)
			this->speed_maximum_maneuver = xf::clamped<si::Velocity> (*this->speed_maximum_maneuver, 0.0_kt, 9999.99_kt);

		if (this->speed_maximum)
			this->speed_maximum = xf::clamped<si::Velocity> (*this->speed_maximum, 0.0_kt, 9999.99_kt);
	}

	// Altitude limits:
	{
		if (this->altitude_amsl)
			this->altitude_amsl = xf::clamped<si::Length> (*this->altitude_amsl, -99999_ft, +99999_ft);

		if (this->vertical_speed)
			this->vertical_speed = xf::clamped<si::Velocity> (*this->vertical_speed, -9999_fpm, +9999_fpm);
	}
}


Blinker::Blinker (si::Time period):
	_period (period)
{ }


inline bool
Blinker::active() const noexcept
{
	return _active;
}


inline bool
Blinker::visibility_state() const noexcept
{
	return _visibility_state;
}


void
Blinker::update (bool condition)
{
	if (condition)
	{
		if (!_active)
		{
			_active = true;
			_visibility_state = true;
		}
	}
	else
	{
		if (_active)
		{
			_active = false;
			_start_timestamp.reset();
		}
	}
}


void
Blinker::update_current_time (si::Time now)
{
	if (_active && !_start_timestamp)
		_start_timestamp = now;

	if (_start_timestamp)
	{
		uint64_t i = (now - *_start_timestamp) / _period;
		_visibility_state = (i % 2) == 1;
	}
}


AdiPaintRequest::AdiPaintRequest (xf::PaintRequest const& paint_request, xf::InstrumentSupport const& instrument_support, Parameters const& params, Precomputed const& precomputed, Blinker const& speed_warning_blinker, Blinker const& decision_height_warning_blinker):
	paint_request (paint_request),
	params (params),
	precomputed (precomputed),
	painter (instrument_support.get_painter (paint_request)),
	aids_ptr (instrument_support.get_aids (paint_request)),
	aids (*aids_ptr),
	speed_warning_blinker (speed_warning_blinker),
	decision_height_warning_blinker (decision_height_warning_blinker),
	q (0.1f * aids.lesser_dimension())
{
	this->default_shadow = aids.default_shadow();
	this->black_shadow = aids.default_shadow();
	this->black_shadow.set_color (Qt::black);
}


inline float
AdiPaintRequest::pitch_to_px (si::Angle const degrees) const
{
	auto const correction = 0.775;
	return -degrees / (params.fov * correction) * aids.lesser_dimension();
}


inline float
AdiPaintRequest::heading_to_px (si::Angle const degrees) const
{
	return pitch_to_px (-degrees);
}


void
AdiPaintRequest::paint_rotating_value (QRectF const& rect, float position, float height_scale,
									   QString const& next, QString const& curr, QString const& prev)
{
	QColor red (255, 0, 0);
	QColor green (0, 255, 0);

	QFont const font = painter.font();
	QFontMetricsF const font_metrics (font);
	float height = height_scale * font_metrics.height();

	// A little bit father to ensure next/prev are hidden beyond clipping area:
	auto const a_little_bit_farther = 1.0f;
	QRectF box_next = rect.translated (0.f, -a_little_bit_farther * height);
	QRectF box_prev = rect.translated (0.f, +a_little_bit_farther * height);

	painter.save_context ([&] {
		painter.setClipRect (rect);
		painter.translate (0.f, -height * position);

		for (std::pair<QRectF, QString> x: { std::make_pair (box_next, next),
											 std::make_pair (rect, curr),
											 std::make_pair (box_prev, prev) })
		{
			if (x.second == "G" || x.second == "R")
				paint_dashed_zone (x.second == "G" ? green : red, x.first);
			else if (x.second == " ")
				; // Paint nothing.
			else
				painter.fast_draw_text (x.first, Qt::AlignVCenter | Qt::AlignLeft, x.second);
		}
	});
}


void
AdiPaintRequest::paint_rotating_digit (QRectF const& box, float const value, int const round_target, float const height_scale, float const delta, float const phase,
									   bool const two_zeros, bool const zero_mark, bool const black_zero)
{
	auto round_to = [](float v, int to) -> float
	{
		float sgn = v >= 0.f ? +1.f : -1.f;
		return static_cast<int> (v + sgn * to / 2.f) / to * to;
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

	QString sa = zero_mark && a == 0 ? (black_zero ? " " : (xa >= 0.f ? "G" : "-")) : QString::number (a);
	QString sb = zero_mark && b == 0 ? (black_zero ? " " : (xb >= 0.f ? "G" : "-")) : QString::number (b);
	QString sc = zero_mark && c == 0 ? (black_zero ? " " : (xc >= 0.f ? "G" : "-")) : QString::number (c);

	if (std::abs (dtr) < delta && (two_zeros || std::abs (value) >= round_target / 2))
		pos = xf::floored_mod (-dtr * (0.5f / delta), 1.f) - 0.5f;

	paint_rotating_value (box, pos, height_scale, sa, sb, sc);
}


void
AdiPaintRequest::paint_dashed_zone (QColor const& color, QRectF const& target)
{
	QFontMetricsF const metrics (painter.font());
	float const w = 0.7f * metrics.width ("0");
	float const h = 0.55f * metrics.height();
	QPointF const center = target.center();
	QRectF box (center - QPointF (w / 2.f, h / 1.9f), QSizeF (w, h));
	QPen pen = aids.get_pen (color, 1.2f);
	QPointF const difx (box.width() / 2.5f, 0.f);
	QPointF const dify (0.f, box.height() / 2.5f);
	pen.setCapStyle (Qt::RoundCap);
	painter.save_context ([&] {
		painter.setPen (pen);
		painter.drawLine (box.topLeft(), box.bottomRight());
		painter.drawLine (box.topLeft() + difx, box.bottomRight() - dify);
		painter.drawLine (box.topLeft() + dify, box.bottomRight() - difx);
		painter.drawLine (box.topLeft() + 2.f * difx, box.bottomRight() - 2.f * dify);
		painter.drawLine (box.topLeft() + 2.f * dify, box.bottomRight() - 2.f * difx);
	});
}


void
AdiPaintRequest::paint_horizontal_failure_flag (QString const& message, QPointF const& center, QFont const& font, QColor const color, bool const focused)
{
	QPen const normal_pen = aids.get_pen (color, 1.0f);
	QFontMetricsF const metrics (font);
	QRectF box (0.f, 0.f, metrics.width (message) + 0.65f * metrics.width ("0"), metrics.height());

	aids.centrify (box);
	box.translate (center);

	painter.setFont (font);
	painter.setBrush (Qt::black);
	painter.setPen (focused ? normal_pen : Qt::NoPen);

	painter.paint (default_shadow, [&] {
		painter.drawRect (box);
	});

	painter.setPen (normal_pen);
	painter.fast_draw_text (center, Qt::AlignHCenter | Qt::AlignVCenter, message, default_shadow);
}


void
AdiPaintRequest::paint_vertical_failure_flag (QString const& message, QPointF const& center, QFont const& font, QColor const color, bool const focused)
{
	QPen const normal_pen = aids.get_pen (color, 1.0f);
	float const digit_width = 1.6f * xf::InstrumentAids::FontInfo::get_digit_width (font);
	float const digit_height = 1.f * QFontMetricsF (font).height();

	QRectF box (0.f, 0.f, 1.f * digit_width, message.size() * digit_height);
	aids.centrify (box);
	box.translate (center);

	painter.setFont (font);
	painter.setBrush (Qt::black);
	painter.setPen (focused ? normal_pen : Qt::NoPen);

	painter.paint (default_shadow, [&] {
		painter.drawRect (box);
	});
	QPointF const top_letter = center + QPointF (0.f, -0.5f * digit_height * (message.size() - 1));

	painter.setPen (normal_pen);

	for (int i = 0; i < message.size(); ++i)
		painter.fast_draw_text (top_letter + QPointF (0.f, i * digit_height), Qt::AlignHCenter | Qt::AlignVCenter, message[i], default_shadow);
}


inline QColor
AdiPaintRequest::get_decision_height_color() const
{
	return params.altitude_amsl && params.decision_height_amsl && *params.altitude_amsl < *params.decision_height_amsl
		? aids.kCautionColor
		: aids.kNavigationColor;
}


void
ArtificialHorizon::paint (AdiPaintRequest& pr) const
{
	precompute (pr);

	if (pr.params.orientation_failure)
	{
		clear (pr);
		paint_orientation_failure (pr);
	}
	else
	{
		paint_horizon (pr);
		paint_pitch_scale (pr);
		paint_heading (pr);
		paint_tcas_ra (pr);
		paint_roll_scale (pr);
		paint_pitch_disagree (pr);
		paint_roll_disagree (pr);
	}

	if (pr.params.flight_path_marker_failure)
		paint_flight_path_marker_failure (pr);
	else
		paint_flight_path_marker (pr);

	if (pr.params.flight_director_failure)
		paint_flight_director_failure (pr);
}


void
ArtificialHorizon::precompute (AdiPaintRequest& pr) const
{
	(*_mutable_this.lock())->precompute (pr);
}


void
ArtificialHorizon::precompute (AdiPaintRequest& pr)
{
	_pitch_transform.reset();
	_pitch_transform.translate (0.f, -pr.pitch_to_px (pr.params.orientation_pitch.value_or (0_deg)));

	_roll_transform.reset();
	_roll_transform.rotate (-pr.params.orientation_roll.value_or (0_deg).in<si::Degree>());

	_heading_transform.reset();
	_heading_transform.translate (-pr.heading_to_px (pr.params.orientation_heading.value_or (0_deg)), 0.f);

	// Total transform of horizon (heading is not really necessary here):
	_fast_horizon_transform = _pitch_transform * _roll_transform * pr.precomputed.center_transform;
	// Without the shear, Qt did something weird sometimes, like aligning drawn points to display pixels (?).
	_horizon_transform = _fast_horizon_transform;
	_horizon_transform.shear (0.0001f, 0.f);

	// Limit FPM position:
	if (pr.params.flight_path_alpha && pr.params.flight_path_beta)
		_flight_path_marker_position = QPointF (-pr.heading_to_px (*pr.params.flight_path_beta),
												-pr.pitch_to_px (*pr.params.flight_path_alpha));
	else
		_flight_path_marker_position = QPointF { 0.0, 0.0 };

	if (pr.paint_request.size_changed())
	{
		float const greater_dimension = pr.aids.greater_dimension();
		float const lesser_dimension = pr.aids.lesser_dimension();
		float const w_max = 2.f * greater_dimension;
		float const h_max = 2.f * greater_dimension;
		_sky_rect = QRectF (-w_max, -h_max, 2.f * w_max, h_max + 1.f);
		_gnd_rect = QRectF (-w_max, 0.f, 2.f * w_max, h_max);

		// Flight path marker:
		{
			float const x = 0.013f * lesser_dimension;
			float const r = 1.05 * x;

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
			float const h = 0.2835f * lesser_dimension;
			float const w = 0.255f * lesser_dimension;
			float const r = 0.2f * h;

			_old_horizon_clip = QPainterPath();
			_old_horizon_clip.addRoundedRect (-w, -h, 2.f * w, 2.f * h, r, r);
		}

		// Pitch scale clipping path:
		{
			float const w = lesser_dimension * 2.f / 9.f;

			QPainterPath clip_path;
			clip_path.setFillRule (Qt::WindingFill);
			clip_path.addEllipse (QRectF (-1.f * w, -1.f * w, 2.f * w, 2.f * w));
			clip_path.addRect (QRectF (-1.f * w, 0.f, 2.f * w, 1.375f * w));

			_pitch_scale_clipping_path = clip_path;
		}
	}
}


void
ArtificialHorizon::clear (AdiPaintRequest& pr) const
{
	pr.painter.setClipping (false);
	pr.painter.resetTransform();
	pr.painter.setPen (Qt::NoPen);
	pr.painter.setBrush (Qt::black);
	pr.painter.drawRect (QRect (QPoint (0, 0), pr.paint_request.metric().canvas_size()));
}


void
ArtificialHorizon::paint_horizon (AdiPaintRequest& pr) const
{
	if (pr.params.orientation_pitch && pr.params.orientation_roll)
	{
		pr.painter.setClipping (false);

		if (pr.params.old_style)
		{
			clear (pr);
			pr.painter.setTransform (pr.precomputed.center_transform);
			pr.painter.setClipPath (_old_horizon_clip);
		}

		pr.painter.setPen (Qt::NoPen);
		// Painting without transform is much faster:
		pr.painter.resetTransform();
		pr.painter.fillRect (QRectF (QPointF (0.0f, 0.0f), pr.paint_request.metric().canvas_size()), kSkyColor);

		pr.painter.setTransform (_fast_horizon_transform);
		pr.painter.fillRect (_gnd_rect, kGroundColor);
	}
	else
		clear (pr);
}


void
ArtificialHorizon::paint_pitch_scale (AdiPaintRequest& pr) const
{
	if (pr.params.orientation_pitch)
	{
		float const lesser_dimension = pr.aids.lesser_dimension();
		float const w = lesser_dimension * (2.0f / 9.0f);
		float const z = 0.5f * w;
		float const fpxs = pr.aids.font_1.font.pixelSize();

		// Clip rectangle before and after rotation:
		pr.painter.setTransform (pr.precomputed.center_transform);
		pr.painter.setClipPath (_pitch_scale_clipping_path);

		if (pr.params.old_style)
			pr.painter.setClipPath (_old_horizon_clip, Qt::IntersectClip);

		pr.painter.setTransform (_roll_transform * pr.precomputed.center_transform);
		pr.painter.setClipRect (QRectF (-w, -1.f * w, 2.f * w, 2.2f * w), Qt::IntersectClip);
		pr.painter.setTransform (_horizon_transform);
		pr.painter.setFont (pr.aids.scaled_default_font (1.2f));

		// Pitch scale is clipped to small rectangle, so narrow it even more:
		float const clipped_pitch_factor = 0.45f;
		xf::Range<si::Angle> deg_range (*pr.params.orientation_pitch - clipped_pitch_factor * 0.485f * pr.params.fov,
										*pr.params.orientation_pitch + clipped_pitch_factor * 0.365f * pr.params.fov);

		pr.painter.setPen (pr.aids.get_pen (Qt::white, 1.f));

		// 10° lines, exclude +/-90°:
		for (int deg = -90; deg <= 90; deg += 10)
		{
			if (deg_range.includes (1_deg * deg) && deg != 0)
			{
				float const d = pr.pitch_to_px (1_deg * deg);
				pr.painter.paint (get_shadow (pr, deg), [&] {
					pr.painter.drawLine (QPointF (-z, d), QPointF (z, d));
				});
				// Degs number:
				int const abs_deg = std::abs (deg);
				QString const deg_t = QString::number (abs_deg > 90 ? 180 - abs_deg : abs_deg);
				//// Text:
				QRectF const lbox (-z - 4.25f * fpxs, d - 0.5f * fpxs, 4.f * fpxs, fpxs);
				QRectF const rbox (+z + 0.25f * fpxs, d - 0.5f * fpxs, 4.f * fpxs, fpxs);
				pr.painter.fast_draw_text (lbox, Qt::AlignVCenter | Qt::AlignRight, deg_t, pr.default_shadow);
				pr.painter.fast_draw_text (rbox, Qt::AlignVCenter | Qt::AlignLeft, deg_t, pr.default_shadow);
			}
		}

		// 5° lines:
		for (int deg = -90; deg <= 90; deg += 5)
		{
			if (deg_range.includes (1_deg * deg) && deg % 10 != 0)
			{
				float const d = pr.pitch_to_px (1_deg * deg);
				pr.painter.paint (get_shadow (pr, deg), [&] {
					pr.painter.drawLine (QPointF (-z / 2.f, d), QPointF (z / 2.f, d));
				});
			}
		}

		// 2.5° lines:
		for (int deg = -900; deg <= 900; deg += 25)
		{
			if (deg_range.includes (1_deg * deg / 10) && deg % 50 != 0)
			{
				float const d = pr.pitch_to_px (1_deg * deg / 10.f);
				pr.painter.paint (get_shadow (pr, deg), [&] {
					pr.painter.drawLine (QPointF (-z / 4.f, d), QPointF (z / 4.f, d));
				});
			}
		}

		// -90°, 90° lines:
		if (deg_range.includes (-90_deg) || deg_range.includes (+90_deg))
		{
			for (float deg: { -90.f, 90.f })
			{
				float const d = pr.pitch_to_px (1_deg * deg);
				pr.painter.setPen (pr.aids.get_pen (Qt::white, 1.75f));
				pr.painter.paint (get_shadow (pr, deg), [&] {
					pr.painter.drawLine (QPointF (-z, d), QPointF (z, d));
				});
			}
		}

		// FPA bug:
		if (pr.params.cmd_fpa)
		{
			for (auto const& pen: { pr.aids.autopilot_pen_1, pr.aids.autopilot_pen_2 })
			{
				si::Angle fpa = *pr.params.cmd_fpa;
				pr.painter.setPen (pen);

				for (auto y_angle: { fpa - 0.5_deg, fpa + 0.5_deg })
				{
					auto const y_pos = pr.pitch_to_px (y_angle);
					pr.painter.drawLine (QPointF (-z, y_pos), QPointF (-0.25 * z, y_pos));
					pr.painter.drawLine (QPointF (+z, y_pos), QPointF (+0.25 * z, y_pos));
				}
			}
		}
	}
}


void
ArtificialHorizon::paint_roll_scale (AdiPaintRequest& pr) const
{
	float const w = pr.aids.lesser_dimension() * 3.f / 9.f;

	QPen pen = pr.aids.get_pen (Qt::white, 1.f);
	pr.painter.setPen (pen);
	pr.painter.setBrush (QBrush (Qt::white));

	QPen warning_pen = pen;
	warning_pen.setColor (pr.aids.kCautionColor);

	pr.painter.setTransform (pr.precomputed.center_transform);
	pr.painter.setClipRect (QRectF (-w, -w, 2.f * w, 2.25f * w));

	for (float deg: { -60.f, -45.f, -30.f, -20.f, -10.f, 0.f, +10.f, +20.f, +30.f, +45.f, +60.f })
	{
		pr.painter.setTransform (pr.precomputed.center_transform);
		pr.painter.rotate (1.f * deg);
		pr.painter.translate (0.f, -0.795f * w);

		if (deg == 0.f)
		{
			// Triangle:
			QPointF const p0 (0.f, 0.f);
			QPointF const px (0.025f * w, 0.f);
			QPointF const py (0.f, 0.05f * w);
			QPolygonF const poly ({ p0, p0 - px - py, p0 + px - py });

			pr.painter.paint (pr.default_shadow, [&] {
				pr.painter.drawPolygon (poly);
			});
		}
		else
		{
			float length = -0.05f * w;

			if (std::abs (std::fmod (deg, 60.f)) < 1.f)
				length *= 1.6f;
			else if (std::abs (std::fmod (deg, 30.f)) < 1.f)
				length *= 2.2f;

			pr.painter.paint (get_shadow (pr, deg), [&] {
				pr.painter.drawLine (QPointF (0.f, 0.f), QPointF (0.f, length));
			});
		}
	}

	if (pr.params.orientation_roll)
	{
		float const bold_width = pr.aids.pen_width (3.f);
		QPointF const a (0, 0.01f * w); // Miter
		QPointF const b (-0.062f * w, 0.1f * w);
		QPointF const c (+0.062f * w, 0.1f * w);
		QPointF const x0 (0.002f * w, 0.f);
		QPointF const y0 (0.f, 0.0f * w);
		QPointF const y1 (0.f, 1.f * bold_width);

		QPolygonF const slip_skid_polygon ({
			b - x0 + y0,
			b - x0 + y1,
			c + x0 + y1,
			c + x0 + y0,
			b - x0 + y0
		});
		QPolygonF const bank_angle_polygon ({ b, a, c, b });

		pr.painter.setTransform (_roll_transform * pr.precomputed.center_transform);
		pr.painter.translate (0.f, -0.79f * w);

		if (pr.params.roll_warning)
		{
			pr.painter.setPen (warning_pen);
			pr.painter.setBrush (warning_pen.color());
			pr.painter.paint (pr.default_shadow, [&] {
				pr.painter.drawPolygon (bank_angle_polygon);
			});
		}
		else
		{
			pr.painter.setPen (pen);
			pr.painter.paint (pr.default_shadow, [&] {
				pr.painter.drawPolyline (bank_angle_polygon);
			});
		}

		if (pr.params.slip_skid)
		{
			pr.painter.translate (-xf::clamped (pr.params.slip_skid->in<si::Degree>(), -4.0, +4.0) * 0.03 * w, 0.0);

			if (pr.params.roll_warning || pr.params.slip_skid_warning)
				pr.painter.setPen (warning_pen);
			else
				pr.painter.setPen (pen);

			if (pr.params.slip_skid_warning)
			{
				pr.painter.setBrush (warning_pen.color());
				pr.painter.paint (pr.default_shadow, [&] {
					pr.painter.drawPolygon (slip_skid_polygon);
				});
			}
			else
			{
				pr.painter.paint (pr.default_shadow, [&] {
					pr.painter.drawPolyline (slip_skid_polygon);
				});
			}
		}
	}
}


void
ArtificialHorizon::paint_heading (AdiPaintRequest& pr) const
{
	float const w = pr.aids.lesser_dimension() * 2.25f / 9.f;
	float const fpxs = pr.aids.font_1.font.pixelSize();

	if (pr.params.orientation_pitch && pr.params.orientation_roll)
	{
		// Clip rectangle before and after rotation:
		pr.painter.setTransform (pr.precomputed.center_transform);
		pr.painter.setClipPath (_pitch_scale_clipping_path);

		if (pr.params.old_style)
			pr.painter.setClipPath (_old_horizon_clip, Qt::IntersectClip);

		pr.painter.setTransform (_roll_transform * pr.precomputed.center_transform);
		pr.painter.setClipRect (QRectF (-1.1f * w, -0.8f * w, 2.2f * w, 1.9f * w), Qt::IntersectClip);

		QPen p = pr.aids.get_pen (Qt::white, 1.f);
		p.setCapStyle (Qt::FlatCap);
		pr.painter.setPen (p);
		pr.painter.setFont (pr.aids.font_1.font);

		if (pr.params.orientation_heading)
		{
			float clipped_pitch_factor = 0.5f;
			xf::Range<si::Angle> deg_range (*pr.params.orientation_heading - clipped_pitch_factor * 0.485f * pr.params.fov,
											*pr.params.orientation_heading + clipped_pitch_factor * 0.350f * pr.params.fov);

			pr.painter.setTransform (_heading_transform * _horizon_transform);
			if (pr.params.orientation_heading_numbers_visible)
			{
				for (int deg = -180; deg < 540; deg += 10)
				{
					if (!deg_range.includes (1_deg * deg))
						continue;

					float const d10 = pr.heading_to_px (1_deg * deg);
					float const d05 = pr.heading_to_px (1_deg * deg + 5_deg);
					// 10° lines:
					pr.painter.paint (pr.default_shadow, [&] {
						pr.painter.drawLine (QPointF (d10, -w / 18.f), QPointF (d10, 0.f));
					});
					// 5° lines:
					pr.painter.paint (pr.default_shadow, [&] {
						pr.painter.drawLine (QPointF (d05, -w / 36.f), QPointF (d05, 0.f));
					});

					QString text = QString::fromStdString ((boost::format ("%02d") % (xf::floored_mod (1.f * deg, 360.f) / 10)).str());
					if (text == "00")
						text = "N";
					else if (text == "09")
						text = "E";
					else if (text == "18")
						text = "S";
					else if (text == "27")
						text = "W";
					pr.painter.fast_draw_text (QRectF (d10 - 2.f * fpxs, 0.f, 4.f * fpxs, fpxs),
											   Qt::AlignVCenter | Qt::AlignHCenter, text, pr.default_shadow);
				}
			}
		}

		// Main horizon line:
		pr.painter.setTransform (_horizon_transform);
		pr.painter.setPen (pr.aids.get_pen (Qt::white, 1.25f));
		pr.painter.paint (pr.default_shadow, [&] {
			pr.painter.drawLine (QPointF (-1.25 * w, 0.f), QPointF (1.25f * w, 0.f));
		});
	}
}


void
ArtificialHorizon::paint_tcas_ra (AdiPaintRequest& pr) const
{
	if (pr.params.tcas_ra_pitch_minimum || pr.params.tcas_ra_pitch_maximum)
	{
		pr.painter.setPen (pr.aids.get_pen (Qt::red, 3.f));

		if (pr.params.old_style)
		{
			pr.painter.setTransform (pr.precomputed.center_transform);
			pr.painter.setClipPath (_old_horizon_clip);
		}
		else
			pr.painter.setClipping (false);

		auto paint_red_lines = [&] (si::Angle const pitch1, si::Angle const pitch2)
		{
			pr.painter.setTransform (_horizon_transform);
			pr.painter.translate (0.f, pr.pitch_to_px (pitch1));

			float const h1 = pr.heading_to_px (6_deg);
			float const h2 = pr.heading_to_px (30_deg);
			float const p2 = pr.pitch_to_px (pitch2);

			pr.painter.paint (pr.default_shadow, [&] {
				pr.painter.drawLine (-h1, 0.f, +h1, 0.f);
				pr.painter.drawLine (-h1, 0.f, -h2, p2);
				pr.painter.drawLine (+h1, 0.f, +h2, p2);
			});
		};

		if (pr.params.tcas_ra_pitch_minimum)
			paint_red_lines (*pr.params.tcas_ra_pitch_minimum, *pr.params.tcas_ra_pitch_minimum - 90_deg);

		if (pr.params.tcas_ra_pitch_maximum)
			paint_red_lines (*pr.params.tcas_ra_pitch_maximum, *pr.params.tcas_ra_pitch_maximum + 90_deg);
	}
}


void
ArtificialHorizon::paint_pitch_disagree (AdiPaintRequest& pr) const
{
	if (pr.params.pitch_disagree)
	{
		pr.painter.setClipping (false);
		pr.painter.setTransform (pr.precomputed.center_transform);
		pr.paint_horizontal_failure_flag ("PITCH", QPointF (-1.6f * pr.q, 2.9f * pr.q), pr.aids.scaled_default_font (1.6f), pr.aids.kWarningColor, pr.params.pitch_disagree_focus);
	}
}


void
ArtificialHorizon::paint_roll_disagree (AdiPaintRequest& pr) const
{
	if (pr.params.roll_disagree)
	{
		pr.painter.setClipping (false);
		pr.painter.setTransform (pr.precomputed.center_transform);
		pr.paint_horizontal_failure_flag ("ROLL", QPointF (+1.6f * pr.q, 2.9f * pr.q), pr.aids.scaled_default_font (1.6f), pr.aids.kWarningColor, pr.params.roll_disagree_focus);
	}
}


void
ArtificialHorizon::paint_flight_path_marker (AdiPaintRequest& pr) const
{
	if (pr.params.flight_path_alpha && pr.params.flight_path_beta)
	{
		auto const ld = pr.aids.lesser_dimension();

		pr.painter.setTransform (pr.precomputed.center_transform);
		pr.painter.setClipRect (QRectF (-0.325f * ld, -0.4f * ld, 0.65f * ld, 0.8f * ld));
		pr.painter.translate (_flight_path_marker_position);
		pr.painter.setPen (pr.aids.get_pen (Qt::white, 1.25f));
		pr.painter.setBrush (Qt::NoBrush);

		xf::Shadow shadow = pr.default_shadow;

		pr.painter.paint (shadow, [&] {
			pr.painter.drawPath (_flight_path_marker_shape);
		});
	}
}


void
ArtificialHorizon::paint_orientation_failure (AdiPaintRequest& pr) const
{
	auto const ld = pr.aids.lesser_dimension();

	pr.painter.setClipping (false);
	pr.painter.setTransform (pr.precomputed.center_transform);
	pr.paint_horizontal_failure_flag ("ATT", QPointF (0.f, -0.055f * ld), pr.aids.scaled_default_font (2.0f), pr.aids.kCautionColor, pr.params.orientation_failure_focus);
}


void
ArtificialHorizon::paint_flight_path_marker_failure (AdiPaintRequest& pr) const
{
	auto const ld = pr.aids.lesser_dimension();

	pr.painter.setClipping (false);
	pr.painter.setTransform (pr.precomputed.center_transform);
	// On Boeing EFIS FPM is called FPV - Flight Path Vector:
	pr.paint_horizontal_failure_flag ("FPV", QPointF (-0.175f * ld, -0.075f * ld), pr.aids.scaled_default_font (1.8f), pr.aids.kCautionColor, pr.params.flight_path_marker_failure_focus);
}


void
ArtificialHorizon::paint_flight_director_failure (AdiPaintRequest& pr) const
{
	auto const ld = pr.aids.lesser_dimension();

	pr.painter.setClipping (false);
	pr.painter.setTransform (pr.precomputed.center_transform);
	pr.paint_horizontal_failure_flag ("FD", QPointF (+0.2f * ld, -0.075f * ld), pr.aids.scaled_default_font (1.8f), pr.aids.kCautionColor, pr.params.flight_director_failure_focus);
}


inline xf::Shadow
ArtificialHorizon::get_shadow (AdiPaintRequest& pr, int degrees) const
{
	xf::Shadow shadow = pr.default_shadow;
	shadow.set_color (degrees > 0 ? kSkyShadow : kGroundShadow);
	return shadow;
}


void
VelocityLadder::paint (AdiPaintRequest& pr) const
{
	precompute (pr);

	float const x = _ladder_rect.width() / 4.0f;

	pr.painter.setClipping (false);
	pr.painter.setTransform (_transform);

	paint_novspd_flag (pr);

	if (pr.params.speed_failure)
		paint_failure (pr);
	else
	{
		pr.painter.setPen (_ladder_pen);
		pr.painter.setBrush (pr.kLadderColor);
		pr.painter.drawRect (_ladder_rect);

		paint_ladder_scale (pr, x);
		paint_speed_limits (pr, x);
		paint_bugs (pr, x);
		paint_speed_tendency (pr, x);
		paint_black_box (pr, x);
		paint_ias_disagree (pr, x);
	}

	paint_mach_or_gs (pr, x);
	paint_ap_setting (pr);
}


void
VelocityLadder::precompute (AdiPaintRequest& pr) const
{
	(*_mutable_this.lock())->precompute (pr);
}


void
VelocityLadder::precompute (AdiPaintRequest& pr)
{
	auto const speed = pr.params.speed.value_or (0_mps);
	_min_shown = speed - 0.5f * pr.params.vl_extent;
	_max_shown = speed + 0.5f * pr.params.vl_extent;
	_min_shown = std::max<si::Velocity> (_min_shown, 1_kt * pr.params.vl_minimum);
	_max_shown = std::min<si::Velocity> (_max_shown, 1_kt * pr.params.vl_maximum);

	if (_min_shown < 0_kt)
		_min_shown = 0_kt;

	_rounded_speed = static_cast<int> (speed.in<si::Knot>() + 0.5);

	if (pr.paint_request.size_changed())
	{
		auto const ld = pr.aids.lesser_dimension();

		_ladder_rect = QRectF (-0.0675f * ld, -0.375 * ld, 0.135 * ld, 0.75f * ld);
		_ladder_pen = QPen (pr.kLadderBorderColor, pr.aids.pen_width (0.75f), Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin);
		_black_box_pen = pr.aids.get_pen (Qt::white, 1.2f, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
		_scale_pen = pr.aids.get_pen (Qt::white, 1.f);
		_speed_bug_pen = pr.aids.get_pen (Qt::green, 1.5f);

		float const digit_width = pr.aids.font_5.digit_width;
		float const digit_height = pr.aids.font_5.digit_height;
		_margin = 0.25f * digit_width;
		_digits = (speed >= 1000_kt - 0.5_kt) ? 4 : 3;

		float const box_height_factor = 2.35;
		_black_box_rect = QRectF (-_digits * digit_width - 2.f * _margin, -0.5f * box_height_factor * digit_height,
								  +_digits * digit_width + 2.f * _margin, box_height_factor * digit_height);

		_transform = pr.precomputed.center_transform;
		_transform.translate (-0.4f * ld, 0.f);

		float const x = _ladder_rect.width() / 4.0f;

		// Speed bug shape:
		{
			_bug_shape = QPolygonF ({
				QPointF (0.f, 0.f),
				QPointF (+0.5f * x, -0.5f * x),
				QPointF (2.f * x, -0.5f * x),
				QPointF (2.f * x, +0.5f * x),
				QPointF (+0.5f * x, +0.5f * x)
			});
		}

		// Special clipping that leaves some clearance around black indicator:
		QMarginsF const clearance_margins (0.f, +0.3f * x, 0.f, +0.3f * x);
		QPainterPath black_box_clearance_path;
		black_box_clearance_path.addRect (_black_box_rect.translated (x, 0.f) + clearance_margins);
		_ladder_clip_path = QPainterPath();
		_ladder_clip_path.addRect (_ladder_rect);
		_ladder_clip_path -= black_box_clearance_path;
	}
}


void
VelocityLadder::paint_black_box (AdiPaintRequest& pr, float const x) const
{
	if (pr.params.speed)
	{
		QFont const& actual_speed_font = pr.aids.font_5.font;
		float const digit_width = pr.aids.font_5.digit_width;

		pr.painter.setClipping (false);
		pr.painter.setTransform (_transform);
		pr.painter.translate (+0.75f * x, 0.f);

		QPen border_pen = _black_box_pen;
		bool const speed_is_in_warning_area =
			(pr.params.speed_minimum && *pr.params.speed > *pr.params.speed_minimum && pr.params.speed_minimum_maneuver && *pr.params.speed < *pr.params.speed_minimum_maneuver) ||
			(pr.params.speed_maximum && *pr.params.speed < *pr.params.speed_maximum && pr.params.speed_maximum_maneuver && *pr.params.speed > *pr.params.speed_maximum_maneuver);

		if (pr.speed_warning_blinker.active() || speed_is_in_warning_area)
		{
			if (pr.speed_warning_blinker.visibility_state() || speed_is_in_warning_area)
				border_pen.setColor (pr.aids.kCautionColor);
			else
				border_pen.setColor (Qt::black);
		}

		pr.painter.setPen (border_pen);
		pr.painter.setBrush (QBrush (QColor (0, 0, 0)));

		QPolygonF const black_box_polygon ({
			QPointF (+0.5f * x, 0.f),
			QPointF (0.f, -0.5f * x),
			_black_box_rect.topRight(),
			_black_box_rect.topLeft(),
			_black_box_rect.bottomLeft(),
			_black_box_rect.bottomRight(),
			QPointF (0.f, +0.5f * x)
		});

		pr.painter.paint (pr.black_shadow, [&] {
			pr.painter.drawPolygon (black_box_polygon);
		});

		QRectF box_1000 = _black_box_rect.adjusted (_margin, _margin, -_margin, -_margin);
		QRectF box_0100 =
			_digits == 3
				? box_1000
				: box_1000.adjusted (digit_width, 0.f, 0.f, 0.f);
		QRectF box_0010 = box_0100.adjusted (digit_width, 0.f, 0.f, 0.f);
		QRectF box_0001 = box_0010.adjusted (digit_width, 0.f, 0.f, 0.f);

		pr.painter.setPen (QPen (Qt::white, 1.f, Qt::SolidLine, Qt::RoundCap));
		pr.painter.setFont (actual_speed_font);

		if (_digits == 4)
			pr.paint_rotating_digit (box_1000, pr.params.speed->in<si::Knot>(), 1000, 1.25f, 0.0005f, 0.5f, false, true);

		pr.paint_rotating_digit (box_0100, pr.params.speed->in<si::Knot>(), 100, 1.25f, 0.005f, 0.5f, false, true, true);
		pr.paint_rotating_digit (box_0010, pr.params.speed->in<si::Knot>(), 10, 1.25f, 0.05f, 0.5f, false, false);
		float pos_0001 = _rounded_speed - pr.params.speed->in<si::Knot>();
		pr.paint_rotating_value (box_0001, pos_0001, 0.7f,
								 QString::number (static_cast<int> (std::abs (std::fmod (1.f * _rounded_speed + 1.f, 10.f)))),
								 QString::number (static_cast<int> (std::abs (std::fmod (1.f * _rounded_speed, 10.f)))),
								 *pr.params.speed > (1_kt * pr.params.vl_minimum + 0.5_kt)
									? QString::number (static_cast<int> (xf::floored_mod (1.f * _rounded_speed - 1.f, 10.f)))
									: " ");
	}
}


void
VelocityLadder::paint_ias_disagree (AdiPaintRequest& pr, float const x) const
{
	if (pr.params.ias_disagree)
	{
		pr.painter.setClipping (false);
		pr.painter.setTransform (_transform);
		pr.painter.setFont (pr.aids.font_0.font);
		pr.painter.setPen (pr.aids.get_pen (pr.aids.kCautionColor, 1.f));

		QPointF const position (-1.75f * x, 9.5f * x);

		pr.painter.paint (pr.default_shadow, [&] {
			pr.painter.fast_draw_text (position, Qt::AlignVCenter | Qt::AlignLeft, "IAS", pr.black_shadow);
			pr.painter.fast_draw_text (position + QPointF (0.f, 0.9f * x), Qt::AlignVCenter | Qt::AlignLeft, "DISAGREE", pr.black_shadow);
		});
	}
}


void
VelocityLadder::paint_ladder_scale (AdiPaintRequest& pr, float const x) const
{
	if (pr.params.speed)
	{
		QFont const& ladder_font = pr.aids.font_2.font;
		float const ladder_digit_width = pr.aids.font_2.digit_width;
		float const ladder_digit_height = pr.aids.font_2.digit_height;

		pr.painter.setFont (ladder_font);

		pr.painter.setTransform (_transform);
		pr.painter.setClipPath (_ladder_clip_path, Qt::IntersectClip);
		pr.painter.translate (2.f * x, 0.f);

		pr.painter.setPen (_scale_pen);
		// -+line_every is to have drawn also numbers that barely fit the scale.
		for (int kt = (static_cast<int> (_min_shown.in<si::Knot>()) / pr.params.vl_line_every) * pr.params.vl_line_every - pr.params.vl_line_every;
			 kt <= _max_shown.in<si::Knot>() + pr.params.vl_line_every;
			 kt += pr.params.vl_line_every)
		{
			if (kt < pr.params.vl_minimum || kt > pr.params.vl_maximum)
				continue;

			float posy = kt_to_px (pr, 1_kt * kt);
			pr.painter.paint (pr.default_shadow, [&] {
				pr.painter.drawLine (QPointF (-0.8f * x, posy), QPointF (0.f, posy));
			});

			if ((kt - pr.params.vl_minimum) % pr.params.vl_number_every == 0)
			{
				pr.painter.fast_draw_text (QRectF (-4.f * ladder_digit_width - 1.25f * x, -0.5f * ladder_digit_height + posy,
												   +4.f * ladder_digit_width, ladder_digit_height),
										   Qt::AlignVCenter | Qt::AlignRight, QString::number (kt),
										   pr.default_shadow);
			}
		}
	}
}


void
VelocityLadder::paint_speed_limits (AdiPaintRequest& pr, float const x) const
{
	if (pr.params.speed)
	{
		float const tr_right = 0.45f * x;
		float const p1w = pr.aids.pen_width (0.54f);

		QPointF const ydif (0.f, pr.aids.pen_width (0.25f));
		QPen pen_b (QColor (0, 0, 0), 1.95f * tr_right, Qt::SolidLine, Qt::FlatCap);
		QPen pen_r (QColor (255, 0, 0), 1.95f * tr_right, Qt::DashLine, Qt::FlatCap);
		pen_r.setDashPattern (QVector<qreal> { 0.5f, 0.6f });
		QPen pen_y (pr.aids.kCautionColor, pr.aids.pen_width (1.2f), Qt::SolidLine, Qt::FlatCap);

		pr.painter.setTransform (_transform);
		pr.painter.translate (tr_right, 0.f);
		pr.painter.setClipRect (_ladder_rect.adjusted (0.f, -ydif.y(), 0.f, ydif.y()));

		float const min_posy = kt_to_px (pr, pr.params.speed_minimum.value_or (0_mps));
		float const min_man_posy = kt_to_px (pr, pr.params.speed_minimum_maneuver ? *pr.params.speed_minimum_maneuver : 0_mps);
		float const max_man_posy = kt_to_px (pr, pr.params.speed_maximum_maneuver ? *pr.params.speed_maximum_maneuver : 0_mps);
		float const max_posy = kt_to_px (pr, pr.params.speed_maximum.value_or (0_mps));
		QPointF const min_point = _ladder_rect.bottomRight() + ydif;
		QPointF const max_point = _ladder_rect.topRight() - ydif;

		if (pr.params.speed_minimum_maneuver && *pr.params.speed_minimum_maneuver > _min_shown)
		{
			QPolygonF const poly ({
				QPointF (_ladder_rect.right() - tr_right, min_man_posy),
				QPointF (_ladder_rect.right() - p1w, min_man_posy),
				min_point - QPointF (p1w, 0.f)
			});
			pr.painter.setPen (pen_y);
			pr.painter.paint (pr.default_shadow, [&] {
				pr.painter.drawPolyline (poly);
			});
		}

		if (pr.params.speed_maximum_maneuver && *pr.params.speed_maximum_maneuver < _max_shown)
		{
			QPolygonF const poly ({
				QPointF (_ladder_rect.right() - tr_right, max_man_posy),
				QPointF (_ladder_rect.right() - p1w, max_man_posy),
				max_point - QPointF (p1w, 0.f)
			});
			pr.painter.setPen (pen_y);
			pr.painter.paint (pr.default_shadow, [&] {
				pr.painter.drawPolyline (poly);
			});
		}

		if (pr.params.speed_maximum && *pr.params.speed_maximum < _max_shown)
		{
			pr.painter.setPen (pen_b);
			pr.painter.drawLine (QPointF (_ladder_rect.right(), max_posy), max_point);
			pr.painter.setPen (pen_r);
			pr.painter.drawLine (QPointF (_ladder_rect.right(), max_posy), max_point);
		}

		if (pr.params.speed_minimum && *pr.params.speed_minimum > _min_shown)
		{
			pr.painter.setPen (pen_b);
			pr.painter.drawLine (QPointF (_ladder_rect.right(), min_posy), min_point);
			pr.painter.setPen (pen_r);
			pr.painter.drawLine (QPointF (_ladder_rect.right(), min_posy), min_point);
		}
	}
}


void
VelocityLadder::paint_speed_tendency (AdiPaintRequest& pr, float const x) const
{
	if (pr.params.speed_lookahead && pr.params.speed)
	{
		QPen pen (pr.aids.get_pen (pr.aids.kNavigationColor, 1.25f));
		pen.setCapStyle (Qt::RoundCap);
		pen.setJoinStyle (Qt::RoundJoin);

		using std::abs;

		pr.painter.setTransform (_transform);
		pr.painter.setPen (pen);
		pr.painter.translate (1.2f * x, 0.f);

		if (*pr.params.speed_lookahead < *pr.params.speed)
			pr.painter.scale (1.f, -1.f);

		float const lookahead_pixels = abs (kt_to_px (pr, xf::clamped<si::Velocity> (*pr.params.speed_lookahead, 1_kt * pr.params.vl_minimum, 1_kt * pr.params.vl_maximum)));
		float const length = std::min<float> (_ladder_rect.height() / 2.f, lookahead_pixels) - 0.5f * x;

		if (length > 0.2f * x)
		{
			QPolygonF const poly ({
				QPointF (0.f, 0.f),
				QPointF (0.f, -length),
				QPointF (-0.2f * x, 0.f - length),
				QPointF (0.f, -0.5f * x - length),
				QPointF (+0.2f * x, 0.f - length),
				QPointF (0.f, -length)
			});

			pr.painter.setClipRect (QRectF (_ladder_rect.topLeft(), QPointF (_ladder_rect.right(), 0.f)));
			pr.painter.paint (pr.default_shadow, [&] {
				pr.painter.drawPolygon (poly);
			});
		}
	}
}


void
VelocityLadder::paint_bugs (AdiPaintRequest& pr, float const x) const
{
	if (pr.params.speed)
	{
		QFont const& speed_bug_font = pr.aids.font_1.font;
		float const speed_bug_digit_height = pr.aids.font_1.digit_height;

		pr.painter.setClipping (false);
		pr.painter.setTransform (_transform);
		pr.painter.setFont (speed_bug_font);

		for (auto const& bug: pr.params.speed_bugs)
		{
			if (bug.second > _min_shown && bug.second < _max_shown)
			{
				float posy = kt_to_px (pr, bug.second);
				pr.painter.setPen (_speed_bug_pen);
				pr.painter.setClipRect (_ladder_rect.translated (x, 0.f));
				pr.painter.paint (pr.default_shadow, [&] {
					pr.painter.drawLine (QPointF (1.5f * x, posy), QPointF (2.25f * x, posy));
				});
				pr.painter.setClipping (false);
				pr.painter.fast_draw_text (QRectF (2.5f * x, posy - 0.5f * speed_bug_digit_height,
												   2.f * x, speed_bug_digit_height),
										   Qt::AlignVCenter | Qt::AlignLeft, bug.first, pr.default_shadow);
			}
		}

		// Speed bug:
		if (pr.params.cmd_speed)
		{
			float const cmd_speed_pixels = kt_to_px (pr, xf::clamped<si::Velocity> (*pr.params.cmd_speed, 1_kt * pr.params.vl_minimum, 1_kt * pr.params.vl_maximum));
			float const posy = xf::clamped<float> (cmd_speed_pixels, -_ladder_rect.height() / 2.0, _ladder_rect.height() / 2.0);
			pr.painter.setClipRect (_ladder_rect.translated (2.5f * x, 0.f));
			pr.painter.translate (1.25f * x, posy);
			pr.painter.setBrush (Qt::NoBrush);
			pr.painter.setPen (pr.aids.autopilot_pen_1);
			pr.painter.drawPolygon (_bug_shape);
			pr.painter.setPen (pr.aids.autopilot_pen_2);
			pr.painter.drawPolygon (_bug_shape);
		}
	}
}


void
VelocityLadder::paint_mach_or_gs (AdiPaintRequest& pr, float const x) const
{
	if (pr.params.speed_mach || pr.params.speed_ground)
	{
		pr.painter.setClipping (false);
		pr.painter.setTransform (_transform);
		pr.painter.translate (0.f, 0.75f * x);
		QPointF const paint_position (0.0, _ladder_rect.bottom() + 0.5 * pr.aids.font_5.digit_height);

		if (pr.params.speed_mach)
		{
			QFont const& font = pr.aids.font_5.font;

			QString mach_str = QString ("%1").arg (*pr.params.speed_mach, 0, 'f', 3);
			if (mach_str.left (2) == "0.")
				mach_str = mach_str.mid (1);

			pr.painter.setPen (pr.aids.get_pen (Qt::white, 1.f));
			pr.painter.setFont (font);
			pr.painter.fast_draw_text (paint_position, Qt::AlignCenter, mach_str, pr.default_shadow);
		}
		else if (pr.params.speed_ground)
		{
			xf::TextLayout layout;
			layout.set_alignment (Qt::AlignHCenter);
			layout.add_fragment ("GS", pr.aids.font_3.font, Qt::white);
			layout.add_fragment (" ", pr.aids.font_1.font, Qt::white);
			layout.add_fragment (QString::number (static_cast<int> (pr.params.speed_ground->in<si::Knot>())), pr.aids.font_5.font, Qt::white);
			layout.paint (paint_position, Qt::AlignCenter, pr.painter, pr.default_shadow);
		}
	}
}


void
VelocityLadder::paint_ap_setting (AdiPaintRequest& pr) const
{
	if (pr.params.cmd_speed || pr.params.cmd_mach)
	{
		QFont const& actual_speed_font = pr.aids.font_5.font;
		float const digit_width = pr.aids.font_5.digit_width;
		float const digit_height = pr.aids.font_5.digit_height;
		float const margin = 0.2f * digit_width;
		int digits = 0;
		QString value;

		// Mach info has priority:
		if (pr.params.cmd_mach)
		{
			value = QString::fromStdString ((boost::format ("%5.3f") % *pr.params.cmd_mach).str());

			if (value.size() > 0 && value[0] == '0')
				value = value.mid (1);

			digits = value.size();
		}
		else if (pr.params.cmd_speed)
		{
			value = QString::number (std::abs (static_cast<int> (pr.params.cmd_speed->in<si::Knot>())));
			digits = 4;
		}

		QRectF digits_box (0.f, 0.f, digits * digit_width + 2.f * margin, 1.3f * digit_height);
		QRectF box_rect (_ladder_rect.right() - digits_box.width(), _ladder_rect.top() - 1.4f * digits_box.height(),
						 digits_box.width(), digits_box.height());

		pr.painter.setClipping (false);
		pr.painter.setTransform (_transform);
		pr.painter.setPen (pr.aids.get_pen (QColor (0, 0, 0), 0.5f));
		pr.painter.setBrush (QBrush (QColor (0, 0, 0)));
		pr.painter.drawRect (box_rect);

		pr.painter.setPen (pr.aids.get_pen (pr.aids.kAutopilotColor, 1.f));
		pr.painter.setFont (actual_speed_font);

		QRectF box = box_rect.adjusted (margin, margin, -margin, -margin);
		pr.painter.fast_draw_text (box, Qt::AlignVCenter | Qt::AlignRight, value, pr.default_shadow);
	}
}


void
VelocityLadder::paint_novspd_flag (AdiPaintRequest& pr) const
{
	if (pr.params.novspd_flag)
	{
		float const margin = 0.025f * pr.q;
		QString const sa = "NO";
		QString const sb = "VSPD";
		QFont const font = pr.aids.scaled_default_font (1.8f);
		QFontMetricsF const metrics (font);
		float const font_height = 0.9f * metrics.height();

		QRectF rect (0.f, 0.f, metrics.width (sa), font_height * (sb.size() + 1));
		rect.moveLeft (0.9f * pr.q);
		rect.moveBottom (-0.4f * pr.q);

		pr.painter.setClipping (false);
		pr.painter.setTransform (_transform);
		pr.painter.setPen (Qt::NoPen);
		pr.painter.setBrush (Qt::black);
		pr.painter.drawRect (rect.adjusted (-margin, 0.f, +margin, 0.f));
		pr.painter.setPen (pr.aids.get_pen (pr.aids.kCautionColor, 1.f));
		pr.painter.setFont (font);

		QPointF const c (rect.center().x(), rect.top());
		QPointF const h (0.f, font_height);

		pr.painter.fast_draw_text (c + 0.5f * h, Qt::AlignHCenter | Qt::AlignVCenter, sa, pr.default_shadow);

		for (int i = 0; i < sb.size(); ++i)
			pr.painter.fast_draw_text (c + 1.5f * h + i * h, Qt::AlignHCenter | Qt::AlignVCenter, sb.mid (i, 1), pr.default_shadow);
	}
}


void
VelocityLadder::paint_failure (AdiPaintRequest& pr) const
{
	pr.painter.setClipping (false);
	pr.painter.setTransform (_transform);
	pr.paint_vertical_failure_flag ("SPD", QPointF (0.f, 0.f), pr.aids.scaled_default_font (2.0f), pr.aids.kCautionColor, pr.params.speed_failure_focus);
}


inline float
VelocityLadder::kt_to_px (AdiPaintRequest& pr, si::Velocity const speed) const
{
	return -0.5 * _ladder_rect.height() * (speed - pr.params.speed.value_or (0_mps)) / (0.5 * pr.params.vl_extent);
}


void
AltitudeLadder::paint (AdiPaintRequest& pr) const
{
	precompute (pr);

	float const x = _ladder_rect.width() / 4.0f;

	if (pr.params.vertical_speed_failure)
		paint_vertical_speed_failure (pr, x);
	else
	{
		pr.painter.setClipping (false);
		pr.painter.setTransform (_transform);

		paint_vertical_speed (pr, x);
		paint_vertical_ap_setting (pr, x);
	}

	if (pr.params.altitude_failure)
		paint_failure (pr);
	else
	{
		pr.painter.setClipping (false);
		pr.painter.setTransform (_transform);

		pr.painter.setPen (_ladder_pen);
		pr.painter.setBrush (pr.kLadderColor);
		pr.painter.drawRect (_ladder_rect);

		paint_ladder_scale (pr, x);
		paint_bugs (pr, x);
		paint_altitude_tendency (pr, x);
		paint_black_box (pr, x);
		paint_altitude_disagree (pr, x);
	}

	pr.painter.setClipping (false);
	pr.painter.setTransform (_transform);

	paint_pressure (pr, x);
	paint_ap_setting (pr);
	paint_ldgalt_flag (pr, x);
}


void
AltitudeLadder::precompute (AdiPaintRequest& pr) const
{
	(*_mutable_this.lock())->precompute (pr);
}


void
AltitudeLadder::precompute (AdiPaintRequest& pr)
{
	auto const altitude_amsl = pr.params.altitude_amsl.value_or (0_ft);
	float const sgn = altitude_amsl < 0_ft ? -1.f : 1.f;
	auto const ld = pr.aids.lesser_dimension();

	_min_shown = altitude_amsl - 0.5f * pr.params.al_extent;
	_max_shown = altitude_amsl + 0.5f * pr.params.al_extent;
	_rounded_altitude = static_cast<int> (altitude_amsl.in<si::Foot>() + sgn * 10.f) / 20 * 20;

	_transform = pr.precomputed.center_transform;
	_transform.translate (+0.4f * ld, 0.f);

	if (pr.paint_request.size_changed() || _previous_show_metric != pr.params.show_metric)
	{
		auto const ld = pr.aids.lesser_dimension();

		_previous_show_metric = pr.params.show_metric;

		_ladder_rect = QRectF (-0.0675f * ld, -0.375 * ld, 0.135 * ld, 0.75f * ld);
		_ladder_pen = QPen (pr.kLadderBorderColor, pr.aids.pen_width (0.75f), Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin);
		_black_box_pen = pr.aids.get_pen (Qt::white, 1.2f, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
		_scale_pen_1 = pr.aids.get_pen (Qt::white, 1.f);
		_scale_pen_2 = pr.aids.get_pen (Qt::white, 3.f, Qt::SolidLine, Qt::SquareCap);
		_altitude_bug_pen = pr.aids.get_pen (QColor (0, 255, 0), 1.5f);
		_ldg_alt_pen = pr.aids.get_pen (QColor (255, 220, 0), 1.5f);
		_ldg_alt_pen.setCapStyle (Qt::RoundCap);

		float const b_digit_width = pr.aids.font_5.digit_width;
		float const b_digit_height = pr.aids.font_5.digit_height;
		float const s_digit_width = pr.aids.font_3.digit_width;
		int const b_digits = 2;
		int const s_digits = 3;
		_margin = 0.25f * b_digit_width;

		float const box_height_factor = 2.35;
		_b_digits_box = QRectF (0.f, 0.f, b_digits * b_digit_width, box_height_factor * b_digit_height - 2.f * _margin);
		_s_digits_box = QRectF (0.f, 0.f, s_digits * s_digit_width, box_height_factor * b_digit_height - 2.f * _margin);
		_black_box_rect = QRectF (0.f, -0.5f * _b_digits_box.height() - _margin,
								  _b_digits_box.width() + _s_digits_box.width() + 2.f * _margin, _b_digits_box.height() + 2.f * _margin);
		_metric_box_rect = QRectF (_black_box_rect.topLeft() - QPointF (0.f, 1.25f * pr.aids.font_3.digit_height), _black_box_rect.topRight());
		_b_digits_box.translate (_margin, -0.5f * _b_digits_box.height());
		_s_digits_box.translate (_margin + _b_digits_box.width(), -0.5f * _s_digits_box.height());

		float const x = _ladder_rect.width() / 4.0f;

		// Special clipping that leaves some margin around black indicator:
		QMarginsF const clearance_margins (0.f, +0.3f * x, 0.f, +0.3f * x);
		QPainterPath black_box_clearance_path;
		QRectF clearance_rect = _black_box_rect;

		if (pr.params.show_metric)
			clearance_rect = clearance_rect.united (_metric_box_rect);

		black_box_clearance_path.addRect (clearance_rect.translated (-x, 0.0f) + clearance_margins);
		_ladder_clip_path = QPainterPath();
		_ladder_clip_path.addRect (_ladder_rect);
		_ladder_clip_path -= black_box_clearance_path;

		_decision_height_clip_path = QPainterPath();
		_decision_height_clip_path.addRect (_ladder_rect.adjusted (-2.5f * x, 0.f, 0.f, 0.f));
		_decision_height_clip_path -= black_box_clearance_path;
	}
}


void
AltitudeLadder::paint_black_box (AdiPaintRequest& pr, float const x) const
{
	QFont const& b_font = pr.aids.font_5.font;
	float const b_digit_width = pr.aids.font_5.digit_width;
	float const b_digit_height = pr.aids.font_5.digit_height;

	QFont const& s_font = pr.aids.font_3.font;
	float const s_digit_width = pr.aids.font_3.digit_width;
	float const s_digit_height = pr.aids.font_3.digit_height;

	QFont const& m_font = pr.aids.font_2.font;

	QFontMetricsF const s_metrics (s_font);
	QFontMetricsF const m_metrics (m_font);

	if (pr.params.altitude_amsl)
	{
		pr.painter.setClipping (false);
		pr.painter.setTransform (_transform);
		pr.painter.translate (-0.75f * x, 0.f);

		QPolygonF const black_box_polygon ({
			QPointF (-0.5f * x, 0.f),
			QPointF (0.f, -0.5f * x),
			_black_box_rect.topLeft(),
			_black_box_rect.topRight(),
			_black_box_rect.bottomRight(),
			_black_box_rect.bottomLeft(),
			QPointF (0.f, +0.5f * x)
		});

		if (pr.params.show_metric)
		{
			pr.painter.setPen (_black_box_pen);
			pr.painter.setBrush (Qt::black);

			// Metric box:
			pr.painter.paint (pr.black_shadow, [&] {
				pr.painter.drawRect (_metric_box_rect);
			});

			// Metric value:
			float xcorr = 0.25f * m_metrics.width (" ");
			QPointF m_pos (_metric_box_rect.right() - 1.5f * m_metrics.width ("M"), _metric_box_rect.center().y());
			pr.painter.setPen (pr.aids.get_pen (QColor (0x00, 0xee, 0xff), 1.f));
			pr.painter.setFont (m_font);
			pr.painter.fast_draw_text (m_pos, Qt::AlignLeft | Qt::AlignVCenter, "M", pr.default_shadow);
			pr.painter.setPen (pr.aids.get_pen (Qt::white, 1.f));
			pr.painter.fast_draw_text (m_pos + QPointF (-xcorr, 0.f), Qt::AlignRight | Qt::AlignVCenter,
									   QString ("%1").arg (std::round (pr.params.altitude_amsl->in<si::Meter>()), 0, 'f', 0), pr.default_shadow);
		}

		pr.painter.setPen (_black_box_pen);
		pr.painter.setBrush (Qt::black);

		// Feet box:
		pr.painter.paint (pr.black_shadow, [&] {
			pr.painter.drawPolygon (black_box_polygon);
		});

		// Feet value:
		QRectF box_10000 = QRectF (_b_digits_box.topLeft(), QSizeF (b_digit_width, _b_digits_box.height()));
		QRectF box_01000 = box_10000.translated (b_digit_width, 0.f);
		QRectF box_00100 = QRectF (_s_digits_box.topLeft(), QSizeF (s_digit_width, _b_digits_box.height()));
		QRectF box_00011 = box_00100.translated (s_digit_width, 0.f).adjusted (0.f, 0.f, s_digit_width, 0.f);

		// 11100 part:
		pr.painter.setFont (b_font);
		pr.paint_rotating_digit (box_10000, pr.params.altitude_amsl->in<si::Foot>(), 10000, 1.25f * s_digit_height / b_digit_height, 0.0005f, 5.f, true, true);
		pr.paint_rotating_digit (box_01000, pr.params.altitude_amsl->in<si::Foot>(), 1000, 1.25f * s_digit_height / b_digit_height, 0.005f, 5.f, false, false);
		pr.painter.setFont (s_font);
		pr.paint_rotating_digit (box_00100, pr.params.altitude_amsl->in<si::Foot>(), 100, 1.25f, 0.05f, 5.f, false, false);

		// 00011 part:
		float pos_00011 = (_rounded_altitude - pr.params.altitude_amsl->in<si::Foot>()) / 20.f;
		pr.paint_rotating_value (box_00011, pos_00011, 0.75f,
								 QString::number (static_cast<int> (std::abs (std::fmod (_rounded_altitude / 10.f + 2.f, 10.f)))) + "0",
								 QString::number (static_cast<int> (std::abs (std::fmod (_rounded_altitude / 10.f + 0.f, 10.f)))) + "0",
								 QString::number (static_cast<int> (std::abs (std::fmod (_rounded_altitude / 10.f - 2.f, 10.f)))) + "0");
	}
}


void
AltitudeLadder::paint_altitude_disagree (AdiPaintRequest& pr, float const x) const
{
	if (pr.params.altitude_disagree)
	{
		pr.painter.setClipping (false);
		pr.painter.setTransform (_transform);
		pr.painter.setFont (pr.aids.font_0.font);
		pr.painter.setPen (pr.aids.get_pen (pr.aids.kCautionColor, 1.f));

		QPointF const position (-1.75f * x, 9.5f * x);

		pr.painter.paint (pr.default_shadow, [&] {
			pr.painter.fast_draw_text (position, Qt::AlignVCenter | Qt::AlignLeft, "ALT", pr.black_shadow);
			pr.painter.fast_draw_text (position + QPointF (0.f, 0.9f * x), Qt::AlignVCenter | Qt::AlignLeft, "DISAGREE", pr.black_shadow);
		});
	}
}


void
AltitudeLadder::paint_ladder_scale (AdiPaintRequest& pr, float const x) const
{
	if (pr.params.altitude_amsl)
	{
		QFont const& b_ladder_font = pr.aids.font_2.font;
		float const b_ladder_digit_width = pr.aids.font_2.digit_width;
		float const b_ladder_digit_height = pr.aids.font_2.digit_height;

		QFont const& s_ladder_font = pr.aids.font_1.font;
		float const s_ladder_digit_width = pr.aids.font_1.digit_width;
		float const s_ladder_digit_height = pr.aids.font_1.digit_height;

		pr.painter.setTransform (_transform);
		pr.painter.setClipPath (_ladder_clip_path, Qt::IntersectClip);
		pr.painter.translate (-2.f * x, 0.f);

		// -+line_every is to have drawn also numbers that barely fit the scale.
		for (int ft = (static_cast<int> (_min_shown.in<si::Foot>()) / pr.params.al_line_every) * pr.params.al_line_every - pr.params.al_line_every;
			 ft <= _max_shown.in<si::Foot>() + pr.params.al_line_every;
			 ft += pr.params.al_line_every)
		{
			if (ft > 100000.f)
				continue;

			float posy = ft_to_px (pr, 1_ft * ft);

			pr.painter.setPen (ft % pr.params.al_bold_every == 0 ? _scale_pen_2 : _scale_pen_1);
			pr.painter.paint (pr.default_shadow, [&] {
				pr.painter.drawLine (QPointF (0.f, posy), QPointF (0.8f * x, posy));
			});

			if (ft % pr.params.al_number_every == 0)
			{
				QRectF big_text_box (1.1f * x, -0.5f * b_ladder_digit_height + posy,
									 2.f * b_ladder_digit_width, b_ladder_digit_height);
				if (std::abs (ft) / 1000 > 0)
				{
					QString big_text = QString::number (ft / 1000);
					pr.painter.setFont (b_ladder_font);
					pr.painter.fast_draw_text (big_text_box, Qt::AlignVCenter | Qt::AlignRight, big_text, pr.default_shadow);
				}

				QString small_text = QString ("%1").arg (QString::number (std::abs (ft % 1000)), 3, '0');
				if (ft == 0)
					small_text = "0";
				pr.painter.setFont (s_ladder_font);
				QRectF small_text_box (1.1f * x + 2.1f * b_ladder_digit_width, -0.5f * s_ladder_digit_height + posy,
									   3.f * s_ladder_digit_width, s_ladder_digit_height);
				pr.painter.fast_draw_text (small_text_box, Qt::AlignVCenter | Qt::AlignRight, small_text, pr.default_shadow);
				// Minus sign?
				if (ft < 0)
				{
					if (ft > -1000)
						pr.painter.fast_draw_text (small_text_box.adjusted (-s_ladder_digit_width, 0.f, 0.f, 0.f),
												   Qt::AlignVCenter | Qt::AlignLeft, pr.aids.kMinusSignStrUTF8, pr.default_shadow);
				}

				// Additional lines above/below every 1000 ft:
				if (ft % pr.params.al_emphasis_every == 0)
				{
					pr.painter.setPen (pr.aids.get_pen (Qt::white, 1.0));
					float r, y;
					r = big_text_box.left() + 4.0 * x;
					y = posy - 0.75f * big_text_box.height();
					pr.painter.paint (pr.default_shadow, [&] {
						pr.painter.drawLine (QPointF (big_text_box.left(), y), QPointF (r, y));
					});
					y = posy + 0.75f * big_text_box.height();
					pr.painter.paint (pr.default_shadow, [&] {
						pr.painter.drawLine (QPointF (big_text_box.left(), y), QPointF (r, y));
					});
				}
			}
		}
	}
}


void
AltitudeLadder::paint_altitude_tendency (AdiPaintRequest& pr, float const x) const
{
	if (pr.params.altitude_lookahead && pr.params.altitude_amsl)
	{
		QPen pen (pr.aids.get_pen (pr.aids.kNavigationColor, 1.25f));
		pen.setCapStyle (Qt::RoundCap);
		pen.setJoinStyle (Qt::RoundJoin);

		pr.painter.setTransform (_transform);
		pr.painter.translate (-1.2f * x, 0.f);
		pr.painter.setPen (pen);

		if (*pr.params.altitude_lookahead < *pr.params.altitude_amsl)
			pr.painter.scale (1.f, -1.f);

		float length = std::min<float> (_ladder_rect.height() / 2.f, 1.f * std::abs (ft_to_px (pr, *pr.params.altitude_lookahead))) - 0.5f * x;

		if (length > 0.2f * x)
		{
			QPolygonF const poly ({
				QPointF (0.f, 0.f),
				QPointF (0.f, -length),
				QPointF (-0.2f * x, 0.f - length),
				QPointF (0.f, -0.5f * x - length),
				QPointF (+0.2f * x, 0.f - length),
				QPointF (0.f, -length)
			});
			pr.painter.setClipRect (QRectF (_ladder_rect.topLeft(), QPointF (_ladder_rect.right(), 0.f)));
			pr.painter.paint (pr.default_shadow, [&] {
				pr.painter.drawPolygon (poly);
			});
		}
	}
}


void
AltitudeLadder::paint_bugs (AdiPaintRequest& pr, float const x) const
{
	if (pr.params.altitude_amsl)
	{
		QFont const& altitude_bug_font = pr.aids.font_1.font;
		float const altitude_bug_digit_height = pr.aids.font_1.digit_height;

		pr.painter.setClipping (false);
		pr.painter.setTransform (_transform);
		pr.painter.setFont (altitude_bug_font);

		for (auto& bug: pr.params.altitude_bugs)
		{
			if (bug.second > _min_shown && bug.second < _max_shown)
			{
				float posy = ft_to_px (pr, bug.second);
				QRectF text_rect (-4.5f * x, posy - 0.5f * altitude_bug_digit_height,
								  +2.f * x, altitude_bug_digit_height);
				pr.painter.setClipRect (_ladder_rect.adjusted (-x, 0.f, 0.f, 0.f));

				pr.painter.setPen (_altitude_bug_pen);
				pr.painter.paint (pr.default_shadow, [&] {
					pr.painter.drawLine (QPointF (-1.5f * x, posy), QPointF (-2.25f * x, posy));
				});

				pr.painter.setClipping (false);
				pr.painter.fast_draw_text (text_rect, Qt::AlignVCenter | Qt::AlignRight, bug.first, pr.default_shadow);
			}
		}

		// Altitude warning:
		if (pr.params.landing_amsl)
		{
			QPointF const p1 (-2.05f * x, ft_to_px (pr, *pr.params.landing_amsl + pr.params.altitude_landing_warning_lo));
			QPointF const p2 (-2.05f * x, ft_to_px (pr, *pr.params.landing_amsl + pr.params.altitude_landing_warning_hi));
			QPointF const p0 (-2.05f * x, ft_to_px (pr, *pr.params.landing_amsl));

			QPen w = _ldg_alt_pen;
			w.setColor (Qt::white);
			w.setCapStyle (Qt::SquareCap);

			pr.painter.setClipRect (_ladder_rect.adjusted (-x, 0.f, 0.f, 0.f));
			pr.painter.setPen (w);
			pr.painter.paint (pr.default_shadow, [&] {
				pr.painter.drawPolyline (QPolygonF ({ p1, p2, p2 + QPointF (0.25f * x, 0.f) }));
			});
			pr.painter.setPen (_ldg_alt_pen);
			pr.painter.paint (pr.default_shadow, [&] {
				pr.painter.drawLine (p0, p1);
			});

			// Landing altitude ground level:
			if (_min_shown < *pr.params.landing_amsl && *pr.params.landing_amsl < _max_shown)
			{
				float const posy = ft_to_px (pr, *pr.params.landing_amsl);

				pr.painter.setClipPath (_ladder_clip_path, Qt::IntersectClip);
				pr.painter.setPen (_ldg_alt_pen);
				pr.painter.drawLine (QPointF (+2.25f * x, posy), QPointF (-2.25f * x, posy));

				for (int i = -8; i <= 4; ++i)
				{
					QPointF const p (0.4f * i * x + 0.125f * x, posy + 0.1f * x);
					pr.painter.drawLine (p, p + QPointF (x, x));
				}
			}
		}

		// AP bug:
		if (pr.params.cmd_altitude)
		{
			si::Length cmd_altitude = xf::clamped<si::Length> (*pr.params.cmd_altitude, -99999_ft, +99999_ft);
			float posy = xf::clamped (ft_to_px (pr, cmd_altitude),
									  static_cast<float> (-_ladder_rect.height() / 2), static_cast<float> (_ladder_rect.height() / 2));
			QPolygonF const bug_shape ({
				QPointF (0.f, 0.f),
				QPointF (-0.5f * x, -0.5f * x),
				QPointF (-0.5f * x, _black_box_rect.top()),
				QPointF (+1.3f * x, _black_box_rect.top()),
				QPointF (+1.3f * x, _black_box_rect.bottom()),
				QPointF (-0.5f * x, _black_box_rect.bottom()),
				QPointF (-0.5f * x, +0.5f * x)
			});
			pr.painter.setClipRect (_ladder_rect.translated (-x, 0.f));
			pr.painter.translate (-2.f * x, posy);
			pr.painter.setBrush (Qt::NoBrush);
			pr.painter.setPen (pr.aids.autopilot_pen_1);
			pr.painter.drawPolygon (bug_shape);
			pr.painter.setPen (pr.aids.autopilot_pen_2);
			pr.painter.drawPolygon (bug_shape);
		}

		// Baro bug:
		if (pr.params.decision_height_amsl)
		{
			if (_min_shown < *pr.params.decision_height_amsl && *pr.params.decision_height_amsl < _max_shown)
			{
				if (!pr.decision_height_warning_blinker.active() || pr.decision_height_warning_blinker.visibility_state())
				{
					float const posy = ft_to_px (pr, *pr.params.decision_height_amsl);

					pr.painter.setTransform (_transform);
					pr.painter.setClipPath (_decision_height_clip_path);

					QPen pen = pr.aids.get_pen (pr.get_decision_height_color(), 1.25f);
					pen.setMiterLimit (0.35f);

					pr.painter.setPen (pen);
					pr.painter.setBrush (Qt::NoBrush);

					QPointF const a (_ladder_rect.left(), posy);
					QPointF const b (_ladder_rect.left() - 0.65f * x, posy - 0.65f * x);
					QPointF const c (_ladder_rect.left() - 0.65f * x, posy + 0.65f * x);
					QPolygonF const poly ({ a, b, c });

					pr.painter.paint (pr.default_shadow, [&] {
						pr.painter.drawLine (a, QPointF (_ladder_rect.right(), posy));
						pr.painter.drawPolygon (poly);
					});
				}
			}
		}
	}
}


void
AltitudeLadder::paint_vertical_speed (AdiPaintRequest& pr, float const x) const
{
	QPen bold_white_pen = pr.aids.get_pen (Qt::white, 1.25f);
	QPen thin_white_pen = pr.aids.get_pen (Qt::white, 0.50f);

	float const y = x * 4.f;
	float const line_w = 0.2f * x;

	pr.painter.setClipping (false);
	pr.painter.setTransform (_transform);
	pr.painter.translate (4.f * x, 0.f);

	pr.painter.setPen (_ladder_pen);
	pr.painter.setBrush (pr.kLadderColor);
	pr.painter.drawPolygon (QPolygonF ({
		QPointF (0.0f, -0.6 * y),
		QPointF (-x, -0.6 * y - x),
		QPointF (-x, -1.9f * y - x),
		QPointF (+0.3f * x, -1.9f * y - x),
		QPointF (1.66f * x, -y - x),
		QPointF (1.66f * x, +y + x),
		QPointF (+0.3f * x, +1.9f * y + x),
		QPointF (-x, +1.9f * y + x),
		QPointF (-x, +0.6 * y + x),
		QPointF (0.0f, +0.6 * y)
	}));

	// Scale:
	pr.painter.setFont (pr.aids.font_1.font);
	pr.painter.setPen (bold_white_pen);
	pr.painter.paint (pr.default_shadow, [&] {
		pr.painter.drawLine (QPointF (0.f, 0.f), QPointF (0.5f * x, 0.f));
	});

	for (float kfpm: { -6.f, -2.f, -1.f, +1.f, +2.f, +6.f })
	{
		float const posy = -2.f * y * scale_vertical_speed (kfpm * 1000_fpm);
		QRectF num_rect (-1.55f * x, posy - x, 1.3f * x, 2.f * x);

		pr.painter.paint (pr.default_shadow, [&] {
			pr.painter.drawLine (QPointF (0.f, posy), QPointF (line_w, posy));
		});
		pr.painter.fast_draw_text (num_rect, Qt::AlignVCenter | Qt::AlignRight, QString::number (std::abs (static_cast<int> (kfpm))), pr.default_shadow);
	}

	pr.painter.setPen (thin_white_pen);

	for (float kfpm: { -4.f, -1.5f, -0.5f, +0.5f, +1.5f, +4.f })
	{
		float const posy = -2.f * y * scale_vertical_speed (kfpm * 1000_fpm);

		pr.painter.paint (pr.default_shadow, [&] {
			pr.painter.drawLine (QPointF (0.f, posy), QPointF (line_w, posy));
		});
	}

	// Variometer:
	if (pr.params.energy_variometer_rate)
	{
		pr.painter.setClipping (false);

		float const posy = -2.f * y * scale_energy_variometer (pr, *pr.params.energy_variometer_rate);
		float const pw = pr.aids.pen_width (2.0);

		pr.painter.setPen (QPen (pr.aids.kNavigationColor, pr.aids.pen_width (1.0)));
		pr.painter.setBrush (pr.aids.kNavigationColor);

		QPolygonF const rhomb ({
			QPointF (0.f, +1.5f * pw),
			QPointF (-pw, 0.f),
			QPointF (0.f, -1.5f * pw),
			QPointF (+pw, 0.f),
			QPointF (0.f, +1.5f * pw)
		});

		pr.painter.paint (pr.default_shadow, [&] {
			pr.painter.drawPolyline (rhomb.translated (1.25f * pw, posy));
		});
	}

	// TCAS
	pr.painter.setPen (Qt::NoPen);
	pr.painter.setBrush (Qt::red);

	auto paint_red_lines = [&] (si::Velocity speed1, si::Velocity speed2)
	{
		pr.painter.setTransform (_transform);
		pr.painter.translate (4.f * x, 0.f);

		float const s1 = -2.f * y * scale_vertical_speed (speed1, 1.015f);
		float const s2 = -2.f * y * scale_vertical_speed (speed2, 1.015f);
		float const ys = 0.875f;

		QPolygonF const figure ({
			QPointF (0.35f * x, s1),
			QPointF (0.75f * x, ys * s1),
			QPointF (0.75f * x, ys * s2),
			QPointF (0.35f * x, s2)
		});

		pr.painter.paint (pr.default_shadow, [&] {
			pr.painter.drawPolygon (figure);
		});
	};

	si::Velocity min_vspd = 5500_fpm;

	if (pr.params.tcas_ra_vertical_speed_minimum)
		paint_red_lines (*pr.params.tcas_ra_vertical_speed_minimum - 20000_fpm,
						 std::max (*pr.params.tcas_ra_vertical_speed_minimum, -min_vspd));

	if (pr.params.tcas_ra_vertical_speed_maximum)
		paint_red_lines (*pr.params.tcas_ra_vertical_speed_maximum + 20000_fpm,
						 std::min (*pr.params.tcas_ra_vertical_speed_maximum, +min_vspd));

	// Pointer:
	if (pr.params.vertical_speed)
	{
		pr.painter.setClipRect (QRectF (0.15f * x, -2.75f * y - x, (1.66f - 0.15f) * x, 5.5f * y + 2.f * x));
		QPen indicator_pen = bold_white_pen;
		indicator_pen.setCapStyle (Qt::FlatCap);
		pr.painter.setPen (indicator_pen);
		pr.painter.paint (pr.default_shadow, [&] {
			pr.painter.drawLine (QPointF (3.f * x, 0.f),
								 QPointF (line_w, -2.f * y * scale_vertical_speed (*pr.params.vertical_speed)));
		});

		// Numeric indicators above and below:
		pr.painter.setPen (bold_white_pen);
		int abs_vertical_speed = static_cast<int> (std::abs (pr.params.vertical_speed->in<si::FootPerMinute>())) / 10 * 10;

		if (abs_vertical_speed >= 100)
		{
			QString str = QString::number (abs_vertical_speed);
			if (str.size() == 2)
				str = "  " + str;
			else if (str.size() == 3)
				str = " " + str;

			float const fh = pr.aids.font_2.digit_height;
			float const sgn = *pr.params.vertical_speed > 0_fpm ? 1.f : -1.f;
			pr.painter.setClipping (false);
			pr.painter.setFont (pr.aids.font_2.font);
			pr.painter.translate (-1.05f * x, sgn * -2.35f * y);
			pr.painter.fast_draw_text (QRectF (0.f, -0.5f * fh, 4.f * fh, fh), Qt::AlignVCenter | Qt::AlignLeft, str, pr.default_shadow);
		}
	}
}


void
AltitudeLadder::paint_vertical_ap_setting (AdiPaintRequest& pr, float const x) const
{
	// Vertical speed bug (vertical AP setting):
	if (pr.params.cmd_vertical_speed && pr.params.vertical_speed)
	{
		pr.painter.setClipping (false);
		pr.painter.setTransform (_transform);
		pr.painter.translate (4.15f * x, 0.f);

		float const posy = -8.f * x * scale_vertical_speed (*pr.params.cmd_vertical_speed);

		for (auto const& pen: { pr.aids.autopilot_pen_1, pr.aids.autopilot_pen_2 })
		{
			pr.painter.setPen (pen);

			for (auto y: { posy - 0.2f * x, posy + 0.2f * x })
				pr.painter.drawLine (QPointF (-0.25 * x, y), QPointF (0.2f * x, y));
		}
	}
}


void
AltitudeLadder::paint_pressure (AdiPaintRequest& pr, float const x) const
{
	if (pr.params.pressure_qnh)
	{
		pr.painter.setClipping (false);
		pr.painter.setTransform (_transform);
		pr.painter.translate (0.f, 0.75f * x);

		QFont const& font_a = pr.params.use_standard_pressure ? pr.aids.font_2.font : pr.aids.font_3.font;
		QFont const& font_b = pr.aids.font_2.font;
		QFontMetricsF const metrics_a (font_a);
		QFontMetricsF const metrics_b (font_b);

		QString unit_str = pr.params.pressure_display_hpa? " HPA" : " IN";
		int precision = pr.params.pressure_display_hpa ? 0 : 2;
		QString pressure_str = QString ("%1").arg (pr.params.pressure_display_hpa ? pr.params.pressure_qnh->in<si::HectoPascal>() : pr.params.pressure_qnh->in<si::InchOfMercury>(), 0, 'f', precision);

		QRectF nn_rect (0.f, _ladder_rect.bottom(), metrics_a.width (pressure_str), 1.2f * pr.aids.font_3.digit_height);
		QRectF uu_rect (0.f, nn_rect.top(), metrics_b.width (unit_str), nn_rect.height());
		nn_rect.moveLeft (-0.5f * (uu_rect.width() + nn_rect.width()));
		// Correct position of uu_rect to get correct baseline position:
		uu_rect.translate (0.f, metrics_b.descent() - metrics_a.descent());
		uu_rect.moveLeft (nn_rect.right());

		pr.painter.setPen (QPen (pr.aids.kNavigationColor, pr.aids.pen_width (1.f), Qt::SolidLine, Qt::RoundCap));

		if (pr.params.use_standard_pressure)
		{
			pr.painter.setFont (pr.aids.font_3.font);
			pr.painter.fast_draw_text (QPointF (0.5f * (nn_rect.left() + uu_rect.right()), nn_rect.bottom()), Qt::AlignHCenter | Qt::AlignBottom, "STD", pr.default_shadow);
			pr.painter.translate (0.f, 0.9f * metrics_a.height());
			pr.painter.setPen (QPen (Qt::white, 1.f, Qt::SolidLine, Qt::RoundCap));
		}

		pr.painter.setFont (font_a);
		pr.painter.fast_draw_text (nn_rect, Qt::AlignBottom | Qt::AlignRight, pressure_str, pr.default_shadow);
		pr.painter.setFont (font_b);
		pr.painter.fast_draw_text (uu_rect, Qt::AlignBottom | Qt::AlignLeft, unit_str, pr.default_shadow);
	}
}


void
AltitudeLadder::paint_ap_setting (AdiPaintRequest& pr) const
{
	if (pr.params.cmd_altitude)
	{
		si::Length cmd_altitude = xf::clamped<si::Length> (*pr.params.cmd_altitude, -99999_ft, +99999_ft);

		QFont const& b_font = pr.aids.font_5.font;
		QFontMetricsF const b_metrics (b_font);
		float const b_digit_width = pr.aids.font_5.digit_width;
		float const b_digit_height = pr.aids.font_5.digit_height;

		QFont const& s_font = pr.aids.font_3.font;
		QFontMetricsF const s_metrics (s_font);
		float const s_digit_width = pr.aids.font_3.digit_width;

		QFont const& m_font = pr.aids.font_2.font;
		QFontMetricsF const m_metrics (m_font);

		int const b_digits = 2;
		int const s_digits = 3;
		float const margin = 0.2f * b_digit_width;

		QRectF b_digits_box (0.f, 0.f, b_digits * b_digit_width + margin, 1.3f * b_digit_height);
		QRectF s_digits_box (0.f, 0.f, s_digits * s_digit_width + margin, 1.3f * b_digit_height);
		QRectF box_rect (_ladder_rect.left(), _ladder_rect.top() - 1.4f * b_digits_box.height(),
						 b_digits_box.width() + s_digits_box.width(), b_digits_box.height());
		QRectF metric_rect (box_rect.topLeft() - QPointF (0.f, 1.25f * m_metrics.height()), box_rect.topRight());
		b_digits_box.translate (box_rect.left(), box_rect.top());
		s_digits_box.translate (b_digits_box.right(), b_digits_box.top());

		pr.painter.setClipping (false);
		pr.painter.setTransform (_transform);

		if (pr.params.show_metric)
		{
			if (!pr.params.old_style)
			{
				auto background_color = pr.kLadderColor.darker (150);
				pr.painter.setPen (pr.aids.get_pen (background_color, 0.5f));
				pr.painter.setBrush (QBrush (background_color));

				// Metric box:
				pr.painter.drawRect (metric_rect);
			}

			// Metric value:
			float xcorr = 0.25f * m_metrics.width (" ");
			QPointF const m_pos (metric_rect.right() - 1.4f * m_metrics.width ("M"), metric_rect.center().y());
			pr.painter.setPen (pr.aids.get_pen (QColor (0x00, 0xee, 0xff), 1.f));
			pr.painter.setFont (m_font);
			pr.painter.fast_draw_text (m_pos, Qt::AlignLeft | Qt::AlignVCenter, "M", pr.default_shadow);
			pr.painter.setPen (pr.aids.get_pen (pr.aids.kAutopilotColor, 1.f));
			pr.painter.fast_draw_text (m_pos + QPointF (-xcorr, 0.f), Qt::AlignRight | Qt::AlignVCenter,
									   QString ("%1").arg (std::round (cmd_altitude.in<si::Meter>()), 0, 'f', 0), pr.default_shadow);
		}

		pr.painter.setPen (pr.aids.get_pen (Qt::black, 0.5f));
		pr.painter.setBrush (QBrush (Qt::black));
		pr.painter.drawRect (box_rect);

		if (pr.params.cmd_altitude_acquired)
		{
			float const z = 0.5f * margin;
			QRectF em_box_rect = box_rect.adjusted (-z, -z, z, z);

			pr.painter.setBrush (Qt::NoBrush);

			for (QPen pen: { pr.aids.get_pen (Qt::black, 1.8f), pr.aids.get_pen (Qt::white, 1.4f) })
			{
				pr.painter.setPen (pen);
				pr.painter.drawRect (em_box_rect);
			}
		}

		pr.painter.setPen (pr.aids.get_pen (pr.aids.kAutopilotColor, 1.f));
		pr.painter.setFont (b_font);

		// 11000 part of the altitude setting:
		QRectF box_11000 = b_digits_box.adjusted (margin, margin, 0.f, -margin);
		QString minus_sign_s = cmd_altitude < -0.5_ft ? pr.aids.kMinusSignStrUTF8 : "";
		pr.painter.fast_draw_text (box_11000, Qt::AlignVCenter | Qt::AlignRight,
								   minus_sign_s + QString::number (std::abs (xf::symmetric_round (cmd_altitude.in<si::Foot>()) / 1000)),
								   pr.default_shadow);

		pr.painter.setFont (s_font);

		// 00111 part of the altitude setting:
		QRectF box_00111 = s_digits_box.adjusted (0.f, margin, -margin, -margin);
		pr.painter.fast_draw_text (box_00111, Qt::AlignVCenter | Qt::AlignLeft,
								   QString ("%1").arg (static_cast<int> (std::round (std::abs (cmd_altitude.in<si::Foot>()))) % 1000, 3, 'f', 0, '0'),
								   pr.default_shadow);
	}
}


void
AltitudeLadder::paint_ldgalt_flag (AdiPaintRequest& pr, float const x) const
{
	if (pr.params.ldgalt_flag)
	{
		pr.painter.setClipping (false);
		pr.painter.setTransform (_transform);
		pr.painter.setPen (pr.aids.kCautionColor);
		pr.painter.setFont (pr.aids.font_1.font);
		pr.painter.fast_draw_text (QPoint (2.2f * x, 10.4f * x), Qt::AlignVCenter | Qt::AlignLeft, "LDG", pr.default_shadow);
		pr.painter.fast_draw_text (QPoint (2.2f * x, 10.4f * x + 1.1f * pr.aids.font_2.digit_height), Qt::AlignVCenter | Qt::AlignLeft, "ALT", pr.default_shadow);
	}
}


void
AltitudeLadder::paint_vertical_speed_failure (AdiPaintRequest& pr, float const x) const
{
	pr.painter.setClipping (false);
	pr.painter.setTransform (_transform);
	pr.painter.translate (4.75f * x, 0.0f);
	pr.paint_vertical_failure_flag ("VERT", QPointF (0.f, 0.f), pr.aids.scaled_default_font (2.0f), pr.aids.kCautionColor, pr.params.vertical_speed_failure_focus);
}


void
AltitudeLadder::paint_failure (AdiPaintRequest& pr) const
{
	pr.painter.setClipping (false);
	pr.painter.setTransform (_transform);
	pr.paint_vertical_failure_flag ("ALT", QPointF (0.f, 0.f), pr.aids.scaled_default_font (2.0f), pr.aids.kCautionColor, pr.params.altitude_failure_focus);
}


float
AltitudeLadder::scale_vertical_speed (si::Velocity vertical_speed, float const max_value) const
{
	float vspd = std::abs (vertical_speed.in<si::FootPerMinute>());

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
AltitudeLadder::scale_energy_variometer (AdiPaintRequest& pr, si::Power const power, float const max_value) const
{
	si::Velocity equivalent_speed = power / pr.params.energy_variometer_1000_fpm_power * 1000_fpm;
	return scale_vertical_speed (equivalent_speed, max_value);
}


inline float
AltitudeLadder::ft_to_px (AdiPaintRequest& pr, si::Length const length) const
{
	return -0.5 * _ladder_rect.height() * (length - pr.params.altitude_amsl.value_or (0_ft)) / (0.5 * pr.params.al_extent);
}


PaintingWork::PaintingWork (xf::Graphics const& graphics):
	_instrument_support (graphics)
{ }


void
PaintingWork::paint (xf::PaintRequest const& paint_request, Parameters const& params) const
{
	AdiPaintRequest pr (paint_request, _instrument_support, _parameters, _precomputed, _speed_warning_blinker, _decision_height_warning_blinker);

	precompute (pr, params);

	if (_parameters.input_alert_visible)
		paint_input_alert (pr);
	else
	{
		_artificial_horizon.paint (pr);

		paint_nav (pr);
		paint_center_cross (pr, false, true);
		paint_flight_director (pr);
		paint_control_surfaces (pr);
		paint_center_cross (pr, true, false);

		if (pr.params.altitude_agl_failure)
			paint_radar_altimeter_failure (pr);
		else
			paint_altitude_agl (pr);

		paint_decision_height_setting (pr);
		paint_hints (pr);
		paint_critical_aoa (pr);

		_velocity_ladder.paint (pr);
		_altitude_ladder.paint (pr);
	}
}


void
PaintingWork::precompute (AdiPaintRequest& pr, Parameters const& params) const
{
	(*_mutable_this.lock())->precompute (pr, params);
}


void
PaintingWork::precompute (AdiPaintRequest& pr, Parameters const& params)
{
	_parameters = params;
	_parameters.sanitize();

	if (pr.paint_request.size_changed())
	{
		_precomputed.center_transform.reset();
		_precomputed.center_transform.translate (0.5f * pr.aids.width(), 0.5f * pr.aids.height());
	}

	_speed_warning_blinker.update_current_time (_parameters.timestamp);
	_speed_warning_blinker.update (_parameters.speed &&
								   ((_parameters.speed_minimum && *_parameters.speed < *_parameters.speed_minimum) ||
									(_parameters.speed_maximum && *_parameters.speed > *_parameters.speed_maximum)));
	_decision_height_warning_blinker.update_current_time (_parameters.timestamp);
	_decision_height_warning_blinker.update (_parameters.altitude_amsl && _parameters.decision_height_amsl &&
											 *_parameters.altitude_amsl < *_parameters.decision_height_amsl &&
											 _parameters.decision_height_focus_short);
}


void
PaintingWork::paint_center_cross (AdiPaintRequest& pr, bool const center_box, bool const rest) const
{
	float const w = pr.aids.lesser_dimension() * 3.f / 9.f;

	QPointF const x (0.025f * w, 0.f);
	QPointF const y (0.f, 0.025f * w);
	QPolygonF const a ({
		-x - y,
		+x - y,
		+x + y,
		-x + y
	});
	QPolygonF const b ({
		-27.f * x - y,
		-11.f * x - y,
		-11.f * x + 4.f * y,
		-13.f * x + 4.f * y,
		-13.f * x + y,
		-27.f * x + y
	});

	pr.painter.setClipping (false);
	pr.painter.setTransform (pr.precomputed.center_transform);

	if (rest)
	{
		pr.painter.setBrush (QBrush (QColor (0, 0, 0)));
		pr.painter.setPen (Qt::NoPen);
		pr.painter.drawPolygon (a);
		pr.painter.setPen (pr.aids.get_pen (Qt::white, 1.5f));
		pr.painter.paint (pr.default_shadow, [&] {
			pr.painter.drawPolygon (b);
			pr.painter.scale (-1.f, 1.f);
			pr.painter.drawPolygon (b);
		});
	}

	if (center_box)
	{
		pr.painter.setPen (pr.aids.get_pen (Qt::white, 1.5f));
		pr.painter.setBrush (Qt::NoBrush);
		pr.painter.paint (pr.default_shadow, [&] {
			pr.painter.drawPolygon (a);
		});
	}
}


void
PaintingWork::paint_flight_director (AdiPaintRequest& pr) const
{
	using xf::sgn;
	using std::abs;

	float const w = pr.aids.lesser_dimension() * 1.4f / 9.f;
	si::Angle const range = pr.params.fov / 4.f;

	if (pr.params.flight_director_guidance_visible && pr.params.orientation_pitch && pr.params.orientation_roll)
	{
		auto const pens = {
			pr.aids.get_pen (pr.aids.autopilot_pen_1.color(), 2.3f),
			pr.aids.get_pen (pr.aids.autopilot_pen_2.color(), 1.65f)
		};

		pr.painter.setClipping (false);
		pr.painter.setTransform (pr.precomputed.center_transform);

		if (pr.params.flight_director_pitch)
		{
			si::Angle pitch = si::cos (*pr.params.orientation_roll) * (*pr.params.flight_director_pitch - *pr.params.orientation_pitch);
			pitch = xf::clamped (pitch, -range, +range);
			float const ypos = pr.pitch_to_px (pitch);

			for (auto const& pen: pens)
			{
				pr.painter.setPen (pen);
				pr.painter.drawLine (QPointF (-w, ypos), QPointF (+w, ypos));
			}
		}

		if (pr.params.flight_director_roll)
		{
			si::Angle roll = *pr.params.flight_director_roll - *pr.params.orientation_roll;

			if (abs (roll) > 180_deg)
				roll = roll - sgn (roll.in<si::Degree>()) * 360_deg;

			roll = xf::clamped (roll, -range, +range);

			float const xpos = pr.heading_to_px (roll) / 2.f;

			for (auto const& pen: pens)
			{
				pr.painter.setPen (pen);
				pr.painter.drawLine (QPointF (xpos, -w), QPointF (xpos, +w));
			}
		}
	}

	if (pr.params.flight_director_active_name)
	{
		pr.painter.setPen (pr.aids.kNavigationColor);
		pr.painter.setFont (pr.aids.font_2.font);
		pr.painter.fast_draw_text (QPointF (2.95 * pr.q, 4.385 * pr.q),
								   Qt::AlignRight | Qt::AlignBottom,
								   QString::fromStdString (*pr.params.flight_director_active_name),
								   pr.default_shadow);
	}
}


void
PaintingWork::paint_control_surfaces (AdiPaintRequest& pr) const
{
	if (pr.params.control_surfaces_visible)
	{
		float const w = pr.aids.lesser_dimension() * 0.2f / 9.f;
		si::Angle const range = 17.5_deg;

		si::Angle pitch = xf::renormalize (xf::clamped (pr.params.control_surfaces_elevator, -1.0f, +1.0f), -1.0f, +1.0f, -range, +range);
		si::Angle roll = xf::renormalize (xf::clamped (pr.params.control_surfaces_ailerons, -1.0f, +1.0f), -1.0f, +1.0f, -range, +range);

		float ypos = pr.pitch_to_px (pitch);
		float xpos = pr.heading_to_px (roll);

		pr.painter.setClipping (false);
		pr.painter.setTransform (pr.precomputed.center_transform);

		// Four corners:
		float const z = 0.25 * pr.q;
		QPolygonF const corner ({
			QPointF (pr.heading_to_px (-range), pr.pitch_to_px (-range) - z),
			QPointF (pr.heading_to_px (-range), pr.pitch_to_px (-range)),
			QPointF (pr.heading_to_px (-range) + z, pr.pitch_to_px (-range))
		});

		for (auto const& pen: { pr.aids.get_pen (pr.aids.kNavigationColor.darker (300), 2.25f),
								pr.aids.get_pen (pr.aids.kNavigationColor, 1.25f) })
		{
			pr.painter.setPen (pen);
			pr.painter.drawPolyline (corner);
			pr.painter.scale (-1.0, +1.0);
			pr.painter.drawPolyline (corner);
			pr.painter.scale (+1.0, -1.0);
			pr.painter.drawPolyline (corner);
			pr.painter.scale (-1.0, +1.0);
			pr.painter.drawPolyline (corner);
			pr.painter.scale (+1.0, -1.0);
		}

		// Pointer:
		for (auto const& pen: { pr.aids.get_pen (pr.aids.kNavigationColor.darker (300), 2.5f),
								pr.aids.get_pen (pr.aids.kNavigationColor, 1.5f) })
		{
			pr.painter.setPen (pen);
			pr.painter.drawLine (QPointF (xpos, ypos - w), QPointF (xpos, ypos + w));
			pr.painter.drawLine (QPointF (xpos - w, ypos), QPointF (xpos + w, ypos));
		}
	}
}


void
PaintingWork::paint_altitude_agl (AdiPaintRequest& pr) const
{
	if (pr.params.altitude_agl)
	{
		si::Length aagl = xf::clamped<si::Length> (*pr.params.altitude_agl, -9999_ft, +99999_ft);
		QFont const& radar_altimeter_font = pr.aids.font_5.font;
		float const digit_width = pr.aids.font_5.digit_width;
		float const digit_height = pr.aids.font_5.digit_height;
		float const v = 0.03f * pr.q;

		int digits = *pr.params.altitude_agl > 9999_ft ? 5 : 4;
		float const margin = 0.2f * digit_width;

		QRectF box_rect (0.f, 0.f, digits * digit_width + 2.f * margin, 1.3f * digit_height);
		box_rect.translate (-box_rect.width() / 2.f, 0.35f * pr.aids.lesser_dimension());
		box_rect.adjust (-v, -v, +v, +v);

		pr.painter.setClipping (false);
		pr.painter.setTransform (pr.precomputed.center_transform);

		if (pr.params.altitude_agl_focus)
			pr.painter.setPen (pr.aids.get_pen (Qt::white, 1.25f));
		else
			pr.painter.setPen (Qt::NoPen);

		pr.painter.setBrush (QBrush (Qt::black));
		pr.painter.drawRect (box_rect);

		pr.painter.setPen (pr.aids.get_pen (Qt::white, 1.f));
		pr.painter.setFont (radar_altimeter_font);

		QRectF box = box_rect.adjusted (margin, margin, -margin, -margin);
		pr.painter.fast_draw_text (box, Qt::AlignVCenter | Qt::AlignHCenter, QString ("%1").arg (std::round (aagl.in<si::Foot>())), pr.default_shadow);
	}
}


void
PaintingWork::paint_decision_height_setting (AdiPaintRequest& pr) const
{
	if (pr.params.decision_height_amsl)
	{
		float x = 0.18f * pr.aids.lesser_dimension();

		pr.painter.setClipping (false);
		pr.painter.setTransform (pr.precomputed.center_transform);

		QFont const& font_a = pr.aids.font_1.font;
		QFont const& font_b = pr.aids.font_3.font;
		QFontMetricsF const metrics_a (font_a);
		QFontMetricsF const metrics_b (font_b);

		QString mins_str = pr.params.decision_height_type;
		QString alt_str = QString ("%1").arg (pr.params.decision_height_setting.in<si::Foot>(), 0, 'f', 0);

		QRectF mins_rect (1.35f * x, 1.8f * x, metrics_a.width (mins_str), metrics_a.height());
		mins_rect.moveRight (mins_rect.left());
		QRectF alt_rect (0.f, 0.f, metrics_b.width (alt_str), metrics_b.height());
		alt_rect.moveTopRight (mins_rect.bottomRight());

		QPen decision_height_pen = pr.aids.get_pen (pr.get_decision_height_color(), 1.f);

		if (!pr.decision_height_warning_blinker.active() || pr.decision_height_warning_blinker.visibility_state())
		{
			pr.painter.setPen (decision_height_pen);
			pr.painter.setFont (font_a);
			pr.painter.fast_draw_text (mins_rect, Qt::AlignVCenter | Qt::AlignRight, mins_str, pr.default_shadow);
			pr.painter.setFont (font_b);
			pr.painter.fast_draw_text (alt_rect, Qt::AlignVCenter | Qt::AlignRight, alt_str, pr.default_shadow);
		}

		if (pr.params.decision_height_focus)
		{
			float v = 0.06f * pr.q;
			QRectF frame = alt_rect.united (mins_rect).adjusted (-2.f * v, -0.75f * v, +2.f * v, 0.f);
			pr.painter.setPen (decision_height_pen);
			pr.painter.setBrush (Qt::NoBrush);
			pr.painter.paint (pr.default_shadow, [&] {
				pr.painter.drawRect (frame);
			});
		}
	}
}


void
PaintingWork::paint_nav (AdiPaintRequest& pr) const
{
	using std::abs;

	auto const ld = pr.aids.lesser_dimension();

	pr.painter.setClipping (false);
	pr.painter.setTransform (pr.precomputed.center_transform);

	if (pr.params.navaid_reference_visible)
	{
		QString loc_str = pr.params.navaid_identifier;
		if (pr.params.navaid_course_magnetic)
		{
			int course_int = xf::symmetric_round (pr.params.navaid_course_magnetic->in<si::Degree>());
			if (course_int == 0)
				course_int = 360;
			loc_str += QString::fromStdString ((boost::format ("/%03d°") % course_int).str());
		}

		pr.painter.setPen (Qt::white);
		pr.painter.setFont (pr.aids.font_1.font);
		pr.painter.fast_draw_text (QPointF (-0.24f * ld, -0.3925f * ld), Qt::AlignTop | Qt::AlignLeft, loc_str, pr.default_shadow);

		if (pr.params.navaid_hint != "")
		{
			pr.painter.setPen (Qt::white);
			pr.painter.setFont (pr.aids.font_3.font);
			pr.painter.fast_draw_text (QPointF (-0.24f * ld, -0.32f * ld), Qt::AlignTop | Qt::AlignLeft, pr.params.navaid_hint, pr.default_shadow);
		}

		QString dme_val = "DME ---";
		if (pr.params.navaid_distance)
			dme_val = QString::fromStdString ((boost::format ("DME %.1f") % pr.params.navaid_distance->in<si::NauticalMile>()).str());

		pr.painter.setPen (Qt::white);
		pr.painter.setFont (pr.aids.font_1.font);
		pr.painter.fast_draw_text (QPointF (-0.24f * ld, -0.36f * ld), Qt::AlignTop | Qt::AlignLeft, dme_val, pr.default_shadow);

		QPen ladder_pen (pr.kLadderBorderColor, pr.aids.pen_width (0.75f), Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin);

		auto paint_ladder = [&](std::optional<si::Angle> original_approach_deviation, std::optional<si::Angle> original_path_deviation) -> void
		{
			si::Angle approach_deviation;

			if (original_approach_deviation)
				approach_deviation = xf::clamped<si::Angle> (*original_approach_deviation, -2.25_deg, +2.25_deg);

			si::Angle path_deviation;

			if (original_path_deviation)
				path_deviation = xf::clamped<si::Angle> (*original_path_deviation, -2.25_deg, +2.25_deg);

			QRectF rect (0.f, 0.f, 0.385f * ld, 0.055f * ld);
			pr.aids.centrify (rect);

			QRectF elli (0.f, 0.f, 0.015f * ld, 0.015f * ld);
			pr.aids.centrify (elli);

			if (!pr.params.old_style)
			{
				pr.painter.setPen (ladder_pen);
				pr.painter.setBrush (pr.kLadderColor);
				pr.painter.drawRect (rect);
			}

			QPolygonF pink_pointer;
			QPolygonF white_pointer;
			bool pink_filled = false;
			bool pink_visible = false;
			bool white_visible = false;

			if (!pr.params.deviation_mixed_mode)
			{
				// Only ILS:
				float w = 0.012f * ld;
				pink_pointer = QPolygonF ({
					QPointF (0.f, -w),
					QPointF (+1.6f * w, 0.f),
					QPointF (0.f, +w),
					QPointF (-1.6f * w, 0.f),
					QPointF (0.f, -w)
				});
				pink_pointer.translate (approach_deviation.in<si::Degree>() * 0.075f * ld, 0.f);
				pink_visible = !!original_approach_deviation;
				pink_filled = original_approach_deviation && abs (*original_approach_deviation) <= abs (approach_deviation);
				white_visible = false;
			}
			else
			{
				// ILS and flight path:
				float w = 0.012f * ld;
				pink_pointer = QPolygonF ({
					QPointF (0.f, -0.2f * w),
					QPointF (+1.0f * w, 2.0f * w),
					QPointF (-1.0f * w, 2.0f * w)
				});
				pink_pointer.translate (path_deviation.in<si::Degree>() * 0.075f * ld, 0.f);
				pink_visible = !!original_path_deviation;
				pink_filled = original_path_deviation && abs (*original_path_deviation) <= abs (path_deviation);
				white_pointer = QPolygonF ({
					QPointF (0.f, -0.8f * w),
					QPointF (+1.6f * w, 0.f),
					QPointF (0.f, +0.8f * w),
					QPointF (-1.6f * w, 0.f),
					QPointF (0.f, -0.8f * w)
				});
				white_pointer.translate (approach_deviation.in<si::Degree>() * 0.075f * ld, -0.65f * w);
				white_visible = !!original_approach_deviation;
			}

			if (pink_visible)
			{
				for (QColor color: { pr.aids.autopilot_pen_1.color(), pr.aids.autopilot_pen_2.color() })
				{
					pr.painter.setPen (pr.aids.get_pen (color, 1.f));

					if (pink_filled)
						pr.painter.setBrush (color);
					else
						pr.painter.setBrush (Qt::NoBrush);

					pr.painter.drawPolygon (pink_pointer);
				}
			}

			if (white_visible)
			{
				pr.painter.setPen (pr.aids.get_pen (Qt::white, 1.f));
				pr.painter.setBrush (Qt::NoBrush);
				pr.painter.drawPolyline (white_pointer);
			}

			if (!pr.params.deviation_mixed_mode)
			{
				// Paint ILS deviation scale:
				pr.painter.setPen (pr.aids.get_pen (Qt::white, 1.5f));
				pr.painter.setBrush (Qt::NoBrush);

				for (float x: { -1.f, -0.5f, +0.5f, +1.f })
					pr.painter.drawEllipse (elli.translated (0.15f * ld * x, 0.f));

				pr.painter.paint (pr.default_shadow, [&] {
					pr.painter.drawLine (QPointF (0.f, -rect.height() / 3.f), QPointF (0.f, +rect.height() / 3.f));
				});
			}
			else
			{
				// Paint path deviation scale:
				pr.painter.setPen (pr.aids.get_pen (Qt::white, 1.2f));
				pr.painter.setBrush (Qt::NoBrush);

				for (float x: { -1.f, +1.f })
				{
					float const sx = 0.15f * ld * x;

					pr.painter.paint (pr.default_shadow, [&] {
						pr.painter.drawLine (QPointF (sx, -rect.height() / 2.75f), QPointF (sx, +rect.height() / 8.f));
					});
				}

				pr.painter.paint (pr.default_shadow, [&] {
					pr.painter.drawLine (QPointF (0.f, -rect.height() / 2.1f), QPointF (0.f, +rect.height() / 6.f));
				});
			}
		};

		pr.painter.setTransform (pr.precomputed.center_transform);
		pr.painter.translate (0.f, 0.452f * ld);

		if (pr.params.deviation_lateral_failure)
			pr.paint_horizontal_failure_flag ("LOC", QPointF (0.f, 0.f), pr.aids.scaled_default_font (1.8f), pr.aids.kCautionColor, pr.params.deviation_lateral_failure_focus);
		else
			paint_ladder (pr.params.deviation_lateral_approach, pr.params.deviation_lateral_flight_path);

		pr.painter.setTransform (pr.precomputed.center_transform);
		pr.painter.translate (0.28f * ld, 0.f);

		if (pr.params.deviation_vertical_failure)
			pr.paint_vertical_failure_flag ("G/S", QPointF (0.f, 0.f), pr.aids.scaled_default_font (1.8f), pr.aids.kCautionColor, pr.params.deviation_vertical_failure_focus);
		else
		{
			pr.painter.rotate (-90);
			paint_ladder (pr.params.deviation_vertical_approach, pr.params.deviation_vertical_flight_path);
		}
	}

	if (pr.params.raising_runway_position && !pr.params.deviation_lateral_failure && pr.params.deviation_lateral_approach)
	{
		float w = 0.15f * ld;
		float h = 0.05f * ld;
		float p = 1.3f;
		float offset = 0.5f * xf::clamped (pr.params.deviation_lateral_approach->in<si::Degree>(), -1.5, +1.5);
		float ypos = -pr.pitch_to_px (xf::clamped<si::Angle> (*pr.params.raising_runway_position + 3.5_deg, 3.5_deg, 25_deg));

		pr.painter.setTransform (pr.precomputed.center_transform);
		pr.painter.translate (0.f, ypos);

		QPointF tps[] = { QPointF (-w, 0.f), QPointF (0.f, 0.f), QPointF (+w, 0.f) };
		QPointF bps[] = { QPointF (-w * p, h), QPointF (0.f, h), QPointF (+w * p, h) };

		for (QPointF& point: tps)
			point += QPointF (2.5f * w * offset, 0);

		for (QPointF& point: bps)
			point += QPointF (2.5f * p * w * offset, 0);

		pr.painter.setClipRect (QRectF (-1.675f * w, -0.2f * h, 3.35f * w, 1.4f * h));

		QPolygonF const runway ({ tps[0], tps[2], bps[2], bps[0] });

		pr.painter.setBrush (Qt::NoBrush);
		for (auto pen: { QPen (pr.aids.kNavigationColor.darker (400), pr.aids.pen_width (2.f)),
								QPen (pr.aids.kNavigationColor, pr.aids.pen_width (1.33f)) })
		{
			pen.setCapStyle (Qt::RoundCap);
			pr.painter.setPen (pen);
			pr.painter.drawPolygon (runway);

			pen.setCapStyle (Qt::FlatCap);
			pr.painter.setPen (pen);
			pr.painter.drawLine (tps[1], bps[1]);
		}
	}
}


void
PaintingWork::paint_hints (AdiPaintRequest& pr) const
{
	if (pr.params.control_hint)
	{
		pr.painter.setClipping (false);
		pr.painter.setTransform (pr.precomputed.center_transform);
		pr.painter.setFont (pr.aids.font_5.font);
		pr.painter.setBrush (Qt::NoBrush);
		pr.painter.setPen (pr.aids.get_pen (pr.aids.kNavigationColor, 1.0));

		QPointF const text_hook = QPointF (0.f, -3.1f * pr.q);

		pr.painter.fast_draw_text (text_hook, Qt::AlignVCenter | Qt::AlignHCenter, *pr.params.control_hint, pr.default_shadow);

		if (pr.params.control_hint_focus)
		{
			float a = 0.055f * pr.q;
			QRectF frame (text_hook, QSizeF (2.25f * pr.q, pr.aids.font_5.digit_height));
			pr.aids.centrify (frame);
			frame.adjust (0.f, -a, 0.f, +a);
			pr.painter.paint (pr.default_shadow, [&] {
				pr.painter.drawRect (frame);
			});
		}
	}

	if (pr.params.fma_visible)
	{
		QRectF rect (0.f, 0.f, 6.3f * pr.q, 0.65f * pr.q);
		pr.aids.centrify (rect);

		float x16 = rect.left() + 1.f / 6.f * rect.width();
		float x26 = rect.left() + 2.f / 6.f * rect.width();
		float x36 = rect.left() + 3.f / 6.f * rect.width();
		float x46 = rect.left() + 4.f / 6.f * rect.width();
		float x56 = rect.left() + 5.f / 6.f * rect.width();
		float y13 = rect.top() + 8.5f / 30.f * rect.height();
		float y23 = rect.top() + 23.5f / 30.f * rect.height();

		QPointF const b1 (x16, y13);
		QPointF const b2 (x36, y13);
		QPointF const b3 (x56, y13);

		QPointF const s1 (x16, y23);
		QPointF const s2 (x36, y23);
		QPointF const s3 (x56, y23);

		QFont const font_big = pr.aids.scaled_default_font (1.5f);
		QFont const font_small = pr.aids.scaled_default_font (1.1f);

		auto paint_big_rect = [&](QPointF point) -> void
		{
			float v = 0.03f * pr.q;
			QRectF frame (point, QSizeF (1.9f * pr.q, xf::InstrumentAids::FontInfo::get_digit_height (font_big)));
			pr.aids.centrify (frame);
			frame.adjust (0.f, -v, 0.f, +v);
			pr.painter.drawRect (frame);
		};

		auto paint_armed_rect = [&](QPointF point) -> void
		{
			float v = 0.025f * pr.q;
			QRectF frame (point, QSizeF (1.9f * pr.q, xf::InstrumentAids::FontInfo::get_digit_height (font_small)));
			pr.aids.centrify (frame);
			frame.adjust (0.f, -v, 0.f, +v);
			pr.painter.drawRect (frame);
		};

		pr.painter.setClipping (false);
		pr.painter.setTransform (pr.precomputed.center_transform);
		pr.painter.translate (0.f, -4.575f * pr.q);
		pr.painter.setPen (QPen (pr.kLadderBorderColor, pr.aids.pen_width (0.75f), Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin));
		pr.painter.setBrush (pr.kLadderColor);
		pr.painter.drawRect (rect);
		pr.painter.setPen (pr.aids.get_pen (Qt::white, 1.2f, Qt::SolidLine, Qt::FlatCap));
		pr.painter.drawLine (QPointF (x26, rect.top()), QPointF (x26, rect.bottom()));
		pr.painter.drawLine (QPointF (x46, rect.top()), QPointF (x46, rect.bottom()));
		pr.painter.setBrush (Qt::NoBrush);

		pr.painter.setPen (pr.aids.get_pen (pr.aids.kNavigationColor, 1.0f));

		if (pr.params.fma_speed_focus)
			paint_big_rect (b1);

		if (pr.params.fma_lateral_focus)
			paint_big_rect (b2);

		if (pr.params.fma_vertical_focus)
			paint_big_rect (b3);

		pr.painter.setPen (pr.aids.get_pen (Qt::white, 1.0f));

		if (pr.params.fma_speed_armed_focus)
			paint_armed_rect (s1);

		if (pr.params.fma_lateral_armed_focus)
			paint_armed_rect (s2);

		if (pr.params.fma_vertical_armed_focus)
			paint_armed_rect (s3);

		pr.painter.setPen (pr.aids.get_pen (pr.aids.kNavigationColor, 1.0f));
		pr.painter.setFont (font_big);
		pr.painter.fast_draw_text (b1, Qt::AlignVCenter | Qt::AlignHCenter, pr.params.fma_speed_hint, pr.default_shadow);
		pr.painter.fast_draw_text (b2, Qt::AlignVCenter | Qt::AlignHCenter, pr.params.fma_lateral_hint, pr.default_shadow);
		pr.painter.fast_draw_text (b3, Qt::AlignVCenter | Qt::AlignHCenter, pr.params.fma_vertical_hint, pr.default_shadow);

		pr.painter.setPen (pr.aids.get_pen (Qt::white, 1.0f));
		pr.painter.setFont (font_small);
		pr.painter.fast_draw_text (s1, Qt::AlignVCenter | Qt::AlignHCenter, pr.params.fma_speed_armed_hint, pr.default_shadow);
		pr.painter.fast_draw_text (s2, Qt::AlignVCenter | Qt::AlignHCenter, pr.params.fma_lateral_armed_hint, pr.default_shadow);
		pr.painter.fast_draw_text (s3, Qt::AlignVCenter | Qt::AlignHCenter, pr.params.fma_vertical_armed_hint, pr.default_shadow);
	}
}


void
PaintingWork::paint_critical_aoa (AdiPaintRequest& pr) const
{
	if (pr.params.critical_aoa && pr.params.aoa_alpha && pr.params.orientation_pitch)
	{
		pr.painter.setClipping (false);
		pr.painter.setTransform (pr.precomputed.center_transform);
		pr.painter.translate (0.f, pr.pitch_to_px (xf::clamped<si::Angle> (*pr.params.critical_aoa - *pr.params.aoa_alpha, -20_deg, +16_deg)));

		float const w = pr.aids.lesser_dimension() * 3.f / 9.f;

		QPointF const x (0.025f * w, 0.f);
		QPointF const y (0.f, 0.025f * w);
		QColor const selected_color =
			*pr.params.critical_aoa > *pr.params.aoa_alpha
				? pr.aids.kCautionColor
				: pr.aids.kWarningColor;

		auto paint = [&] (QColor color, float pen_add) -> void
		{
			pr.painter.setPen (pr.aids.get_pen (color, 1.8f + pen_add));
			pr.painter.drawPolyline (QPolygonF ({
				-11.f * x + y,
				-11.f * x - y,
				-17.f * x - y
			}));
			pr.painter.setPen (pr.aids.get_pen (color, 1.35f + pen_add));
			pr.painter.drawLine (-12.5f * x - y, -14.f * x - 3.65f * y);
			pr.painter.drawLine (-14.f * x - y, -15.5f * x - 3.65f * y);
			pr.painter.drawLine (-15.5f * x - y, -17.f * x - 3.65f * y);
		};

		paint (pr.default_shadow.color(), 1.0f);
		paint (selected_color, 0.0f);
		pr.painter.scale (-1.f, 1.f);
		paint (pr.default_shadow.color(), 1.0f);
		paint (selected_color, 0.0f);
	}
}


void
PaintingWork::paint_input_alert (AdiPaintRequest& pr) const
{
	QFont const font = pr.aids.scaled_default_font (3.0f);
	QString alert = "NO INPUT";

	QFontMetricsF const font_metrics (font);
	int width = font_metrics.width (alert);

	QPen pen = pr.aids.get_pen (Qt::white, 2.f);

	pr.painter.setClipping (false);

	pr.painter.setTransform (pr.precomputed.center_transform);
	pr.painter.setPen (Qt::NoPen);
	pr.painter.setBrush (Qt::black);
	pr.painter.drawRect (QRect (QPoint (0, 0), pr.paint_request.metric().canvas_size()));

	pr.painter.setTransform (pr.precomputed.center_transform);
	pr.painter.setPen (pen);
	pr.painter.setBrush (QBrush (QColor (0xdd, 0, 0)));
	pr.painter.setFont (font);

	QRectF rect (-0.6f * width, -0.5f * font_metrics.height(), 1.2f * width, 1.2f * font_metrics.height());

	pr.painter.drawRect (rect);
	pr.painter.fast_draw_text (rect, Qt::AlignVCenter | Qt::AlignHCenter, alert, pr.default_shadow);
}


void
PaintingWork::paint_radar_altimeter_failure (AdiPaintRequest& pr) const
{
	auto const& font_info = pr.aids.font_5;
	float const digit_height = font_info.digit_height;

	pr.painter.setClipping (false);
	pr.painter.setTransform (pr.precomputed.center_transform);
	pr.paint_horizontal_failure_flag (" RA ", QPointF (0.f, 0.35f * pr.aids.lesser_dimension() + 0.5f * 1.3f * digit_height), font_info.font, pr.aids.kCautionColor, pr.params.altitude_agl_failure_focus);
}

} // namespace adi_detail


ADI::ADI (xf::Graphics const& graphics, std::string_view const& instance):
	ADI_IO (instance),
	_painting_work (graphics)
{
	_fpv_computer.set_callback (std::bind (&ADI::compute_fpv, this));
	_fpv_computer.observe ({
		&_io.orientation_heading_magnetic,
		&_io.orientation_heading_true,
		&_io.orientation_pitch,
		&_io.orientation_roll,
		&_io.track_lateral_magnetic,
		&_io.track_lateral_true,
		&_io.track_vertical,
		&_io.fpv_visible,
		&_io.weight_on_wheels,
	});
}


void
ADI::process (xf::Cycle const& cycle)
{
	_fpv_computer.process (cycle.update_time());

	adi_detail::Parameters params;
	params.timestamp = cycle.update_time();
	params.fov = *_io.field_of_view;
	params.show_vertical_speed_ladder = *_io.show_vertical_speed_ladder;
	params.focus_duration = *_io.focus_duration;
	params.focus_short_duration = *_io.focus_short_duration;
	params.old_style = _io.style_old.value_or (false);
	params.show_metric = _io.style_show_metric.value_or (false);
	// Speed
	params.speed_failure = !is_sane (_io.speed_ias, { 0_mps, 1000_mps });
	_speed_failure_timestamp.update (cycle.update_time(), [&] {
		return params.speed_failure;
	});
	params.speed_failure_focus = _speed_failure_timestamp.shorter_than (*_io.focus_duration);
	params.speed = _io.speed_ias.get_optional();
	params.speed_lookahead = _io.speed_ias_lookahead.get_optional();

	if (*_io.show_minimum_speeds_only_if_no_weight_on_wheels)
	{
		if (_io.weight_on_wheels.value_or (false))
		{
			params.speed_minimum.reset();
			params.speed_minimum_maneuver.reset();
		}
		else
		{
			params.speed_minimum = _io.speed_ias_minimum.get_optional();
			params.speed_minimum_maneuver = _io.speed_ias_minimum_maneuver.get_optional();
		}
	}

	params.speed_maximum_maneuver = _io.speed_ias_maximum_maneuver.get_optional();
	params.speed_maximum = _io.speed_ias_maximum.get_optional();
	params.speed_mach =
		_io.speed_mach && *_io.speed_mach > *_io.show_mach_above
			? _io.speed_mach.get_optional()
			: std::nullopt;
	params.speed_ground = _io.speed_ground.get_optional();

	// V1
	if (_io.speed_v1)
		params.speed_bugs["V1"] = *_io.speed_v1;
	else
		params.speed_bugs.erase ("V1");

	// Vr
	if (_io.speed_vr)
		params.speed_bugs["VR"] = *_io.speed_vr;
	else
		params.speed_bugs.erase ("VR");

	// Vref
	if (_io.speed_vref)
		params.speed_bugs["REF"] = *_io.speed_vref;
	else
		params.speed_bugs.erase ("REF");

	// Flaps UP bug:
	if (_io.speed_flaps_up_speed && _io.speed_flaps_up_label)
	{
		_speed_flaps_up_current_label = QString::fromStdString (*_io.speed_flaps_up_label);
		params.speed_bugs[_speed_flaps_up_current_label] = *_io.speed_flaps_up_speed;
	}
	else
		params.speed_bugs.erase (_speed_flaps_up_current_label);

	// Flaps "a" bug:
	if (_io.speed_flaps_a_speed && _io.speed_flaps_a_label)
	{
		_speed_flaps_a_current_label = QString::fromStdString (*_io.speed_flaps_a_label);
		params.speed_bugs[_speed_flaps_a_current_label] = *_io.speed_flaps_a_speed;
	}
	else
		params.speed_bugs.erase (_speed_flaps_a_current_label);

	// Flaps "b" bug:
	if (_io.speed_flaps_b_speed && _io.speed_flaps_b_label)
	{
		_speed_flaps_b_current_label = QString::fromStdString (*_io.speed_flaps_b_label);
		params.speed_bugs[_speed_flaps_b_current_label] = *_io.speed_flaps_b_speed;
	}
	else
		params.speed_bugs.erase (_speed_flaps_b_current_label);

	// Orientation
	params.orientation_failure = !is_sane (_io.orientation_pitch) || !is_sane (_io.orientation_roll);
	_orientation_failure_timestamp.update (cycle.update_time(), [&] {
		return params.orientation_failure;
	});
	params.orientation_failure_focus = _orientation_failure_timestamp.shorter_than (*_io.focus_duration);
	params.orientation_pitch = _io.orientation_pitch.get_optional();
	params.orientation_roll = _io.orientation_roll.get_optional();
	params.orientation_heading = _io.orientation_heading_magnetic.get_optional();
	params.orientation_heading_numbers_visible = _io.orientation_heading_numbers_visible.value_or (false);
	// Slip-skid
	params.slip_skid = _io.slip_skid.get_optional();
	// Flight path vector:
	params.flight_path_marker_failure = _computed_fpv_failure;
	_flight_path_marker_failure_timestamp.update (cycle.update_time(), [&] {
		return params.flight_path_marker_failure;
	});
	params.flight_path_marker_failure_focus = _flight_path_marker_failure_timestamp.shorter_than (*_io.focus_duration);
	params.flight_path_alpha = _computed_fpv_alpha;
	params.flight_path_beta = _computed_fpv_beta;
	// AOA limit
	params.aoa_alpha = _io.aoa_alpha.get_optional();
	params.critical_aoa =
		_io.aoa_alpha_visible.value_or (false) && _io.aoa_alpha_maximum && (*_io.aoa_alpha_maximum - *_io.aoa_alpha <= *_io.aoa_visibility_threshold)
			? _io.aoa_alpha_maximum.get_optional()
			: std::nullopt;
	// Altitude
	params.altitude_failure = !is_sane (_io.altitude_amsl);
	_altitude_failure_timestamp.update (cycle.update_time(), [&] {
		return params.altitude_failure;
	});
	params.altitude_failure_focus = _altitude_failure_timestamp.shorter_than (*_io.focus_duration);
	params.altitude_amsl = _io.altitude_amsl.get_optional();
	params.altitude_lookahead = _io.altitude_amsl_lookahead.get_optional();
	params.altitude_agl_failure = !_io.altitude_agl_serviceable.value_or (true) || !is_sane (_io.altitude_agl);
	_altitude_agl_failure_timestamp.update (cycle.update_time(), [&] {
		return params.altitude_agl_failure;
	});
	params.altitude_agl_failure_focus = _altitude_agl_failure_timestamp.shorter_than (*_io.focus_duration);
	params.altitude_agl = _io.altitude_agl.get_optional();
	_altitude_agl_became_visible.update (cycle.update_time(), [&] {
		return _io.altitude_agl_serviceable && *_io.altitude_agl_serviceable && _io.altitude_agl;
	});
	params.altitude_agl_focus = _altitude_agl_became_visible.shorter_than (*_io.focus_duration);
	params.altitude_landing_warning_hi = *_io.altitude_landing_warning_hi;
	params.altitude_landing_warning_lo = *_io.altitude_landing_warning_lo;
	// Decision height
	params.decision_height_type = QString::fromStdString (_io.decision_height_type.value_or (""));
	params.decision_height_amsl =
		_io.decision_height_setting
			? _io.decision_height_amsl.get_optional()
			: std::nullopt;
	_decision_height_became_visible.update (cycle.update_time(), [&] {
		return _io.altitude_amsl && _io.decision_height_amsl &&
			   *_io.altitude_amsl < *_io.decision_height_amsl;
	});
	params.decision_height_focus = _decision_height_became_visible.shorter_than (*_io.focus_duration);
	params.decision_height_focus_short = _decision_height_became_visible.shorter_than (*_io.focus_short_duration);
	params.decision_height_setting = _io.decision_height_setting.value_or (0_ft);
	// Landing altitude:
	params.landing_amsl = _io.landing_amsl.get_optional();
	// Vertical speed
	params.vertical_speed_failure = !is_sane (_io.vertical_speed);
	_vertical_speed_failure_timestamp.update (cycle.update_time(), [&] {
		return params.vertical_speed_failure;
	});
	params.vertical_speed_failure_focus = _vertical_speed_failure_timestamp.shorter_than (*_io.focus_duration);
	params.vertical_speed = _io.vertical_speed.get_optional();
	params.energy_variometer_rate = _io.vertical_speed_energy_variometer.get_optional();
	params.energy_variometer_1000_fpm_power = *_io.power_eq_1000_fpm;
	// Pressure settings
	params.pressure_qnh = _io.pressure_qnh.get_optional();
	params.pressure_display_hpa = _io.pressure_display_hpa.value_or (false);
	params.use_standard_pressure = _io.pressure_use_std.value_or (false);
	// Command settings
	bool cmd_visible = _io.flight_director_cmd_visible.value_or (false);

	if (cmd_visible)
	{
		params.cmd_speed = _io.flight_director_cmd_ias.get_optional();
		params.cmd_mach = _io.flight_director_cmd_mach.get_optional();
		params.cmd_altitude = _io.flight_director_cmd_altitude.get_optional();
		params.cmd_vertical_speed = _io.flight_director_cmd_vertical_speed.get_optional();
		params.cmd_fpa = _io.flight_director_cmd_fpa.get_optional();
	}
	else
	{
		params.cmd_speed.reset();
		params.cmd_mach.reset();
		params.cmd_altitude.reset();
		params.cmd_vertical_speed.reset();
		params.cmd_fpa.reset();
	}

	params.cmd_altitude_acquired = _io.flight_director_cmd_altitude_acquired.value_or (false);
	// Flight director
	bool guidance_visible = _io.flight_director_guidance_visible.value_or (false);
	params.flight_director_guidance_visible = guidance_visible;
	params.flight_director_active_name = _io.flight_director_active_name.get_optional();
	params.flight_director_failure =
		guidance_visible &&
		(!_io.flight_director_serviceable.value_or (true) ||
		 !is_sane (_io.flight_director_cmd_altitude) ||
		 !is_sane (_io.flight_director_cmd_ias) ||
		 !is_sane (_io.flight_director_cmd_mach) ||
		 !is_sane (_io.flight_director_cmd_vertical_speed) ||
		 !is_sane (_io.flight_director_cmd_fpa) ||
		 (!is_sane (_io.flight_director_guidance_pitch) || !is_sane (_io.flight_director_guidance_roll)));
	_flight_director_failure_timestamp.update (cycle.update_time(), [&] {
		return params.flight_director_failure;
	});
	params.flight_director_failure_focus = _flight_director_failure_timestamp.shorter_than (*_io.focus_duration);
	params.flight_director_pitch = _io.flight_director_guidance_pitch.get_optional();
	params.flight_director_roll = _io.flight_director_guidance_roll.get_optional();
	// Control stick
	params.control_surfaces_visible = _io.control_surfaces_visible.value_or (false) && _io.control_surfaces_elevator && _io.control_surfaces_ailerons;
	params.control_surfaces_elevator = _io.control_surfaces_elevator.value_or (0.0f);
	params.control_surfaces_ailerons = _io.control_surfaces_ailerons.value_or (0.0f);
	// Approach/navaid reference
	params.navaid_reference_visible = _io.navaid_reference_visible.value_or (false);
	params.navaid_course_magnetic = _io.navaid_course_magnetic.get_optional();
	params.navaid_distance = _io.navaid_distance.get_optional();
	params.navaid_hint = QString::fromStdString (_io.navaid_type_hint.value_or (""));
	params.navaid_identifier = QString::fromStdString (_io.navaid_identifier.value_or (""));
	// Approach, flight path deviations
	params.deviation_vertical_failure =
		!_io.flight_path_deviation_vertical_serviceable.value_or (true) ||
		!is_sane (_io.flight_path_deviation_vertical) ||
		!is_sane (_io.flight_path_deviation_vertical_approach) ||
		!is_sane (_io.flight_path_deviation_vertical_flight_path);
	_deviation_vertical_failure_timestamp.update (cycle.update_time(), [&] {
		return params.deviation_vertical_failure;
	});
	params.deviation_vertical_failure_focus = _deviation_vertical_failure_timestamp.shorter_than (*_io.focus_duration);
	params.deviation_vertical_approach = _io.flight_path_deviation_vertical_approach.get_optional();
	params.deviation_vertical_flight_path = _io.flight_path_deviation_vertical_flight_path.get_optional();
	params.deviation_lateral_failure =
		!_io.flight_path_deviation_lateral_serviceable.value_or (true) ||
		!is_sane (_io.flight_path_deviation_lateral_approach) ||
		!is_sane (_io.flight_path_deviation_lateral_flight_path);
	_deviation_lateral_failure_timestamp.update (cycle.update_time(), [&] {
		return params.deviation_lateral_failure;
	});
	params.deviation_lateral_failure_focus = _deviation_lateral_failure_timestamp.shorter_than (*_io.focus_duration);
	params.deviation_lateral_approach = _io.flight_path_deviation_lateral_approach.get_optional();
	params.deviation_lateral_flight_path = _io.flight_path_deviation_lateral_flight_path.get_optional();
	params.deviation_mixed_mode = _io.flight_path_deviation_mixed_mode.value_or (false);
	// Raising runway
	if (*_io.enable_raising_runway && _io.navaid_reference_visible.value_or (false) && _io.altitude_agl &&
		_io.flight_path_deviation_lateral_approach && *_io.altitude_agl <= *_io.raising_runway_visibility)
	{
		params.raising_runway_position = xf::clamped<si::Length> (_io.altitude_agl.value_or (0_ft), 0_ft, *_io.raising_runway_threshold) / *_io.raising_runway_threshold * 25_deg;
	}
	else
		params.raising_runway_position.reset();
	// Control hint
	if (_io.flight_mode_hint_visible.value_or (false))
		params.control_hint = QString::fromStdString (_io.flight_mode_hint.value_or (""));
	else
		params.control_hint.reset();

	params.control_hint_focus = _io.flight_mode_hint_visible.modification_age() < *_io.focus_duration || _io.flight_mode_hint.modification_age() < *_io.focus_duration;
	// FMA
	params.fma_visible = _io.flight_mode_fma_visible.value_or (false);
	params.fma_speed_hint = QString::fromStdString (_io.flight_mode_fma_speed_hint.value_or (""));
	params.fma_speed_focus = _io.flight_mode_fma_speed_hint.modification_age() < *_io.focus_duration;
	params.fma_speed_armed_hint = QString::fromStdString (_io.flight_mode_fma_speed_armed_hint.value_or (""));
	params.fma_speed_armed_focus = _io.flight_mode_fma_speed_armed_hint.modification_age() < *_io.focus_duration;
	params.fma_lateral_hint = QString::fromStdString (_io.flight_mode_fma_lateral_hint.value_or (""));
	params.fma_lateral_focus = _io.flight_mode_fma_lateral_hint.modification_age() < *_io.focus_duration;
	params.fma_lateral_armed_hint = QString::fromStdString (_io.flight_mode_fma_lateral_armed_hint.value_or (""));
	params.fma_lateral_armed_focus = _io.flight_mode_fma_lateral_armed_hint.modification_age() < *_io.focus_duration;
	params.fma_vertical_hint = QString::fromStdString (_io.flight_mode_fma_vertical_hint.value_or (""));
	params.fma_vertical_focus = _io.flight_mode_fma_vertical_hint.modification_age() < *_io.focus_duration;
	params.fma_vertical_armed_hint = QString::fromStdString (_io.flight_mode_fma_vertical_armed_hint.value_or (""));
	params.fma_vertical_armed_focus = _io.flight_mode_fma_vertical_armed_hint.modification_age() < *_io.focus_duration;
	// TCAS
	params.tcas_ra_pitch_minimum = _io.tcas_resolution_advisory_pitch_minimum.get_optional();
	params.tcas_ra_pitch_maximum = _io.tcas_resolution_advisory_pitch_maximum.get_optional();
	params.tcas_ra_vertical_speed_minimum = _io.tcas_resolution_advisory_vertical_speed_minimum.get_optional();
	params.tcas_ra_vertical_speed_maximum = _io.tcas_resolution_advisory_vertical_speed_maximum.get_optional();
	// Warning flags
	params.novspd_flag = _io.warning_novspd_flag.value_or (false);
	params.ldgalt_flag = _io.warning_ldgalt_flag.value_or (false);
	params.pitch_disagree = _io.warning_pitch_disagree.value_or (false);
	_pitch_disagree_timestamp.update (cycle.update_time(), [&] {
		return params.pitch_disagree;
	});
	params.pitch_disagree_focus = _pitch_disagree_timestamp.shorter_than (*_io.focus_duration);
	params.roll_disagree = _io.warning_roll_disagree.value_or (false);
	_roll_disagree_timestamp.update (cycle.update_time(), [&] {
		return params.roll_disagree;
	});
	params.roll_disagree_focus = _roll_disagree_timestamp.shorter_than (*_io.focus_duration);
	params.ias_disagree = _io.warning_ias_disagree.value_or (false);
	params.altitude_disagree = _io.warning_altitude_disagree.value_or (false);
	params.roll_warning = _io.warning_roll.value_or (false);
	params.slip_skid_warning = _io.warning_slip_skid.value_or (false);
	// Settings:
	params.vl_extent = 1_kt * *_io.speed_ladder_extent;
	params.vl_minimum = *_io.speed_ladder_minimum;
	params.vl_maximum = *_io.speed_ladder_maximum;
	params.vl_line_every = *_io.speed_ladder_line_every;
	params.vl_number_every = *_io.speed_ladder_number_every;
	params.al_extent = 1_ft * *_io.altitude_ladder_extent;
	params.al_emphasis_every = *_io.altitude_ladder_emphasis_every;
	params.al_bold_every = *_io.altitude_ladder_bold_every;
	params.al_line_every = *_io.altitude_ladder_line_every;
	params.al_number_every = *_io.altitude_ladder_number_every;

	*_parameters.lock() = params;
	mark_dirty();
}


std::packaged_task<void()>
ADI::paint (xf::PaintRequest paint_request) const
{
	return std::packaged_task<void()> ([this, pr = std::move (paint_request), pp = *_parameters.lock()] {
		_painting_work.paint (pr, pp);
	});
}


void
ADI::compute_fpv()
{
	xf::ModuleIn<si::Angle>* heading = nullptr;
	xf::ModuleIn<si::Angle>* track_lateral = nullptr;

	if (_io.orientation_heading_magnetic && _io.track_lateral_magnetic)
	{
		heading = &_io.orientation_heading_magnetic;
		track_lateral = &_io.track_lateral_magnetic;
	}
	else if (_io.orientation_heading_true && _io.track_lateral_true)
	{
		heading = &_io.orientation_heading_true;
		track_lateral = &_io.track_lateral_true;
	}

	// Hide FPV if weight-on-wheels:
	bool const hidden = _io.weight_on_wheels.value_or (false);

	if (_io.fpv_visible.value_or (false) && !hidden && _io.orientation_pitch && _io.orientation_roll && _io.track_vertical && heading && track_lateral)
	{
		si::Angle vdiff = xf::floored_mod (*_io.orientation_pitch - *_io.track_vertical, -180_deg, +180_deg);
		si::Angle hdiff = xf::floored_mod (**heading - **track_lateral, -180_deg, +180_deg);
		si::Angle roll = *_io.orientation_roll;

		_computed_fpv_alpha = vdiff * si::cos (roll) + hdiff * si::sin (roll);
		_computed_fpv_beta = -vdiff * si::sin (roll) + hdiff * si::cos (roll);
		_computed_fpv_failure = false;
	}
	else
	{
		_computed_fpv_failure = !hidden;
		_computed_fpv_alpha.reset();
		_computed_fpv_beta.reset();
	}
}


template<class FloatingPoint>
	bool
	ADI::is_sane (xf::Socket<FloatingPoint> const& socket)
	{
		using std::isfinite;
		using si::isfinite;

		return socket && isfinite (*socket);
	}


template<class FloatingPoint>
	bool
	ADI::is_sane (xf::Socket<FloatingPoint> const& socket, xf::Range<FloatingPoint> const& sane_range)
	{
		return is_sane (socket) && sane_range.includes (*socket);
	}


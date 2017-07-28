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
#include <vector>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/numeric.h>
#include <xefis/utility/painter.h>

// Local:
#include "radial_indicator.h"


RadialIndicator::RadialIndicator (std::unique_ptr<RadialIndicatorIO> module_io,
								  v2::PropertyDigitizer value_digitizer,
								  v2::PropertyDigitizer value_target_digitizer,
								  v2::PropertyDigitizer value_reference_digitizer,
								  v2::PropertyDigitizer value_automatic_digitizer,
								  std::string const& instance):
	InstrumentAids (0.9f),
	BasicIndicator (std::move (module_io), instance),
	_value_digitizer (value_digitizer),
	_value_target_digitizer (value_target_digitizer),
	_value_reference_digitizer (value_reference_digitizer),
	_value_automatic_digitizer (value_automatic_digitizer)
{
	_inputs_observer.set_callback ([&]{ update(); });
	_inputs_observer.observe ({
		&_value_digitizer.property(),
		&_value_target_digitizer.property(),
		&_value_reference_digitizer.property(),
		&_value_automatic_digitizer.property(),
	});
}


void
RadialIndicator::process (v2::Cycle const& cycle)
{
	_inputs_observer.process (cycle.update_dt());
}


void
RadialIndicator::paintEvent (QPaintEvent*)
{
	auto painting_token = get_token (this);

	float const w = width();
	float const h = height();

	clear_background();

	painter().translate (w / 2.f, h / 2.4f);

	float q = 0.068f * wh();
	float r = 6.5f * q;

	paint_text (q, r);
	paint_indicator (q, r);
}


void
RadialIndicator::paint_text (float q, float)
{
	std::optional<double> value = _value_digitizer.to_numeric();
	std::optional<double> reference_value = _value_reference_digitizer.to_numeric();

	QString text;
	if (value)
		text = stringify_value (*value);

	QFont font (_font_20);
	QFontMetricsF metrics (font);
	QFont small_font (_font_16);
	QFontMetricsF small_metrics (small_font);

	QPen pen = get_pen (Qt::white, 0.8f);
	pen.setCapStyle (Qt::RoundCap);

	float margin = 0.4f * q;
	float zero_width = metrics.width ('0');
	float small_zero_width = small_metrics.width ('0');

	QRectF text_rect (0.5f * pen.width(), -0.6f * q, metrics.width ("000.0"), 0.9f * metrics.height());
	text_rect.translate (margin, -text_rect.height());
	QRectF rect = text_rect.adjusted (-margin, 0, margin, 0);

	painter().save();

	painter().setFont (font);
	if (value)
	{
		painter().setPen (pen);
		painter().drawRect (rect);
		painter().fast_draw_text (text_rect, Qt::AlignRight | Qt::AlignVCenter, text);
	}
	else
	{
		painter().setPen (pen);
		painter().drawRect (rect);
	}

	if (reference_value)
	{
		painter().setFont (small_font);
		painter().setPen (get_pen (Qt::green, 1.0f));
		painter().fast_draw_text (QPointF (text_rect.right() - zero_width + small_zero_width, text_rect.top()),
								  Qt::AlignBottom | Qt::AlignRight,
								  stringify_value (*reference_value));
	}

	painter().restore();
}


void
RadialIndicator::paint_indicator (float, float r)
{
	std::optional<double> value = _value_digitizer.to_numeric();
	std::optional<double> value_target = _value_target_digitizer.to_numeric();
	std::optional<double> value_reference = _value_reference_digitizer.to_numeric();
	std::optional<double> value_automatic = _value_automatic_digitizer.to_numeric();
	xf::Range<double> range { *io.value_minimum, *io.value_maximum };

	QColor silver (0xbb, 0xbd, 0xbf);
	QColor gray (0x7a, 0x7a, 0x7a);
	QColor yellow (255, 220, 0);
	QColor orange (255, 150, 0);
	QColor red (255, 0, 0);

	QPen silver_pen = get_pen (silver, 1.0f);
	silver_pen.setCapStyle (Qt::RoundCap);

	QPen pen = get_pen (Qt::white, 1.0f);
	pen.setCapStyle (Qt::RoundCap);

	QPen pointer_pen = get_pen (Qt::white, 1.1f);
	pointer_pen.setCapStyle (Qt::RoundCap);

	QPen warning_pen = get_pen (yellow, 1.f);
	warning_pen.setCapStyle (Qt::RoundCap);

	QPen critical_pen = get_pen (red, 1.f);
	critical_pen.setCapStyle (Qt::RoundCap);

	QPen green_pen = get_pen (QColor (0x00, 0xff, 0x00), 1.f);
	green_pen.setCapStyle (Qt::RoundCap);

	QPen gray_pen = get_pen (QColor (0xb0, 0xb0, 0xb0), 1.f);
	gray_pen.setCapStyle (Qt::RoundCap);

	QPen automatic_pen = get_pen (QColor (0x22, 0xaa, 0xff), 1.1f);
	automatic_pen.setCapStyle (Qt::RoundCap);

	QBrush brush (gray, Qt::SolidPattern);
	QRectF rect (-r, -r, 2.f * r, 2.f * r);

	double value_span_angle = 210.f;
	double v_value = value ? xf::clamped (*value, range) : 0.f;
	double v_warning = *io.value_maximum_warning ? xf::clamped (**io.value_maximum_warning, range) : 0.f;
	double v_critical = *io.value_maximum_critical ? xf::clamped (**io.value_maximum_critical, range) : 0.f;
	double v_target = value_target ? xf::clamped (*value_target, range) : 0.f;
	double v_reference = value_reference ? xf::clamped (*value_reference, range) : 0.f;
	double v_automatic = value_automatic ? xf::clamped (*value_automatic, range) : 0.f;

	if (!*io.value_maximum_warning)
		v_warning = range.max();

	if (!*io.value_maximum_critical)
		v_critical = range.max();

	// Fill colors:
	if (*io.value_maximum_warning && v_value >= v_warning)
		brush.setColor (orange.darker (100));

	if (*io.value_maximum_critical && v_value >= v_critical)
		brush.setColor (red);

	double value_angle = value_span_angle * (v_value - range.min()) / range.extent();
	double warning_angle = value_span_angle * (v_warning - range.min()) / range.extent();
	double critical_angle = value_span_angle * (v_critical - range.min()) / range.extent();
	double reference_angle = value_span_angle * (v_reference - range.min()) / range.extent();
	double target_angle = value_span_angle * (v_target - range.min()) / range.extent();
	double automatic_angle = value_span_angle * (v_automatic - range.min()) / range.extent();

	painter().save();

	if (value)
	{
		painter().save();
		painter().setPen (Qt::NoPen);
		painter().setBrush (brush);
		painter().drawPie (rect, -16.f * 0.f, -16.f * value_angle);
		painter().setPen (gray_pen);
		painter().drawLine (QPointF (0.f, 0.f), QPointF (r, 0.f));
		painter().restore();
	}

	// Warning/critical bugs:

	painter().save();

	float gap_degs = 4;

	_points.clear();
	_points.emplace_back (0.f, silver_pen, 0.f);

	if (*io.value_maximum_warning)
		_points.emplace_back (warning_angle, warning_pen, 0.1f * r);

	if (*io.value_maximum_critical)
		_points.emplace_back (critical_angle, critical_pen, 0.2f * r);

	_points.emplace_back (value_span_angle, critical_pen, 0.f);

	for (auto i = 0u; i < _points.size() - 1; ++i)
	{
		PointInfo curr = _points[i];
		PointInfo next = _points[i + 1];
		float is_last = i == _points.size() - 2;

		painter().save();
		painter().setPen (curr.pen);
		painter().drawArc (rect,
						   -16.f * curr.angle,
						   -16.f * (next.angle - curr.angle - (is_last ? 0 : gap_degs)));
		painter().rotate (curr.angle);
		painter().drawLine (QPointF (r, 0.f), QPointF (r + curr.tick_len, 0.f));
		painter().restore();
	}

	// Normal value bug:
	if (value_reference)
	{
		painter().setPen (green_pen);
		painter().rotate (reference_angle);
		painter().drawLine (QPointF (r + pen_width (1.f), 0.f), QPointF (1.17f * r, 0.f));
		painter().drawLine (QPointF (1.15f * r, 0.f), QPointF (1.3f * r, -0.14f * r));
		painter().drawLine (QPointF (1.15f * r, 0.f), QPointF (1.3f * r, +0.14f * r));
	}

	painter().restore();

	// Needle:
	if (value)
	{
		painter().rotate (value_angle);
		painter().set_shadow_color (Qt::black);
		painter().set_shadow_width (1.9f);

		auto draw_outside_arc = [&] (double angle, double ext_adj, double intr, double extr, bool with_core_pointer)
		{
			if (with_core_pointer)
				painter().draw_outlined_line (QPointF (0.0, 0.0), QPointF (extr, 0.0));
			else
				painter().draw_outlined_line (QPointF (1.0, 0.0), QPointF (extr, 0.0));

			painter().rotate (angle - value_angle);
			painter().draw_outlined_line (QPointF (intr, 0.0), QPointF (extr, 0.0));
			painter().drawArc (rect.adjusted (-ext_adj, -ext_adj, +ext_adj, +ext_adj), arc_degs (90_deg), arc_span (1_deg * (value_angle - angle)));
		};

		painter().save();
		painter().setPen (automatic_pen);

		if (value_automatic)
			draw_outside_arc (automatic_angle, 0.10 * r, 0.95 * r, 1.10 * r, false);

		painter().restore();
		painter().setPen (pointer_pen);

		if (value_target)
			draw_outside_arc (target_angle, 0.15 * r, 1.01 * r, 1.15 * r, true);
		else
			painter().draw_outlined_line (QPointF (0.0, 0.0), QPointF (0.99f * r, 0.0));
	}

	painter().restore();
}



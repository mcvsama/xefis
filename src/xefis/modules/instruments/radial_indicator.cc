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

// Local:
#include "radial_indicator.h"


RadialIndicator::RadialIndicator (std::unique_ptr<RadialIndicatorIO> module_io,
								  xf::PropertyDigitizer value_digitizer,
								  xf::PropertyDigitizer value_target_digitizer,
								  xf::PropertyDigitizer value_reference_digitizer,
								  xf::PropertyDigitizer value_automatic_digitizer,
								  std::string const& instance):
	BasicIndicator (std::move (module_io), instance),
	_value_digitizer (value_digitizer),
	_value_target_digitizer (value_target_digitizer),
	_value_reference_digitizer (value_reference_digitizer),
	_value_automatic_digitizer (value_automatic_digitizer)
{
	_inputs_observer.set_callback ([&]{
		mark_dirty();
	});
	_inputs_observer.observe ({
		&_value_digitizer.property(),
		&_value_target_digitizer.property(),
		&_value_reference_digitizer.property(),
		&_value_automatic_digitizer.property(),
	});
}


void
RadialIndicator::process (xf::Cycle const& cycle)
{
	_inputs_observer.process (cycle.update_dt());
}


void
RadialIndicator::paint (xf::PaintRequest& paint_request) const
{
	auto aids = get_aids (paint_request);
	auto painter = get_painter (paint_request);

	float const w = aids->width();
	float const h = aids->height();

	painter.translate (w / 2.f, h / 2.4f);

	float q = 0.068f * aids->lesser_dimension();
	float r = 6.5f * q;

	paint_text (*aids, painter, q, r);
	paint_indicator (*aids, painter, q, r);
}


void
RadialIndicator::paint_text (xf::InstrumentAids& aids, xf::InstrumentPainter& painter, float q, float) const
{
	std::optional<double> value = _value_digitizer.to_numeric();
	std::optional<double> reference_value = _value_reference_digitizer.to_numeric();

	QString text;
	if (value)
		text = stringify_value (*value);

	QFont font (aids.font_5.font);
	QFontMetricsF metrics (font);
	QFont small_font (aids.font_3.font);
	QFontMetricsF small_metrics (small_font);

	QPen pen = aids.get_pen (Qt::white, 0.8f);
	pen.setCapStyle (Qt::RoundCap);

	float margin = 0.4f * q;
	float zero_width = metrics.width ('0');
	float small_zero_width = small_metrics.width ('0');

	QRectF text_rect (0.5f * pen.width(), -0.6f * q, metrics.width ("000.0"), 0.9f * metrics.height());
	text_rect.translate (margin, -text_rect.height());
	QRectF rect = text_rect.adjusted (-margin, 0, margin, 0);

	painter.save_context ([&] {
		painter.setFont (font);
		if (value)
		{
			painter.setPen (pen);
			painter.drawRect (rect);
			painter.fast_draw_text (text_rect, Qt::AlignRight | Qt::AlignVCenter, text);
		}
		else
		{
			painter.setPen (pen);
			painter.drawRect (rect);
		}

		if (reference_value)
		{
			painter.setFont (small_font);
			painter.setPen (aids.get_pen (Qt::green, 1.0f));
			painter.fast_draw_text (QPointF (text_rect.right() - zero_width + small_zero_width, text_rect.top()),
									Qt::AlignBottom | Qt::AlignRight,
									stringify_value (*reference_value));
		}
	});
}


void
RadialIndicator::paint_indicator (xf::InstrumentAids& aids, xf::InstrumentPainter& painter, float, float r) const
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

	QPen silver_pen = aids.get_pen (silver, 1.0f);
	silver_pen.setCapStyle (Qt::RoundCap);

	QPen pen = aids.get_pen (Qt::white, 1.0f);
	pen.setCapStyle (Qt::RoundCap);

	QPen pointer_pen = aids.get_pen (Qt::white, 1.1f);
	pointer_pen.setCapStyle (Qt::RoundCap);

	QPen warning_pen = aids.get_pen (yellow, 1.f);
	warning_pen.setCapStyle (Qt::RoundCap);

	QPen critical_pen = aids.get_pen (red, 1.f);
	critical_pen.setCapStyle (Qt::RoundCap);

	QPen green_pen = aids.get_pen (QColor (0x00, 0xff, 0x00), 1.f);
	green_pen.setCapStyle (Qt::RoundCap);

	QPen gray_pen = aids.get_pen (QColor (0xb0, 0xb0, 0xb0), 1.f);
	gray_pen.setCapStyle (Qt::RoundCap);

	QPen automatic_pen = aids.get_pen (QColor (0x22, 0xaa, 0xff), 1.1f);
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

	painter.save_context ([&] {
		if (value)
		{
			painter.save_context ([&] {
				painter.setPen (Qt::NoPen);
				painter.setBrush (brush);
				painter.drawPie (rect, -16.f * 0.f, -16.f * value_angle);
				painter.setPen (gray_pen);
				painter.drawLine (QPointF (0.f, 0.f), QPointF (r, 0.f));
			});
		}

		// Warning/critical bugs:

		painter.save_context ([&] {
			float gap_degs = 4;

			std::vector<PointInfo> points;
			points.emplace_back (0.f, silver_pen, 0.f);

			if (*io.value_maximum_warning)
				points.emplace_back (warning_angle, warning_pen, 0.1f * r);

			if (*io.value_maximum_critical)
				points.emplace_back (critical_angle, critical_pen, 0.2f * r);

			points.emplace_back (value_span_angle, critical_pen, 0.f);

			for (auto i = 0u; i < points.size() - 1; ++i)
			{
				PointInfo curr = points[i];
				PointInfo next = points[i + 1];
				float is_last = i == points.size() - 2;

				painter.save_context ([&] {
					painter.setPen (curr.pen);
					painter.drawArc (rect,
									 -16.f * curr.angle,
									 -16.f * (next.angle - curr.angle - (is_last ? 0 : gap_degs)));
					painter.rotate (curr.angle);
					painter.drawLine (QPointF (r, 0.f), QPointF (r + curr.tick_len, 0.f));
				});
			}

			// Normal value bug:
			if (value_reference)
			{
				painter.setPen (green_pen);
				painter.rotate (reference_angle);
				painter.drawLine (QPointF (r + aids.pen_width (1.f), 0.f), QPointF (1.17f * r, 0.f));
				painter.drawLine (QPointF (1.15f * r, 0.f), QPointF (1.3f * r, -0.14f * r));
				painter.drawLine (QPointF (1.15f * r, 0.f), QPointF (1.3f * r, +0.14f * r));
			}
		});

		// Needle:
		if (value)
		{
			xf::Shadow outline_shadow;
			outline_shadow.set_color (Qt::black);
			outline_shadow.set_width (1.9f);

			painter.rotate (value_angle);

			auto draw_outside_arc = [&] (double angle, double ext_adj, double intr, double extr, bool with_core_pointer)
			{
				if (with_core_pointer)
				{
					painter.paint (outline_shadow, [&] {
						painter.drawLine (QPointF (0.0, 0.0), QPointF (extr, 0.0));
					});
				}
				else
				{
					painter.paint (outline_shadow, [&] {
						painter.drawLine (QPointF (1.0, 0.0), QPointF (extr, 0.0));
					});
				}

				painter.rotate (angle - value_angle);
				painter.paint (outline_shadow, [&] {
					painter.drawLine (QPointF (intr, 0.0), QPointF (extr, 0.0));
				});
				painter.drawArc (rect.adjusted (-ext_adj, -ext_adj, +ext_adj, +ext_adj),
								 aids.angle_for_qpainter (0_deg),
								 aids.angle_for_qpainter (-1_deg * (value_angle - angle)));
			};

			painter.save_context ([&] {
				painter.setPen (automatic_pen);

				if (value_automatic)
					draw_outside_arc (automatic_angle, 0.10 * r, 0.95 * r, 1.10 * r, false);
			});

			painter.setPen (pointer_pen);

			if (value_target)
				draw_outside_arc (target_angle, 0.15 * r, 1.01 * r, 1.15 * r, true);
			else
			{
				painter.paint (outline_shadow, [&] {
					painter.drawLine (QPointF (0.0, 0.0), QPointF (0.99f * r, 0.0));
				});
			}
		}
	});
}



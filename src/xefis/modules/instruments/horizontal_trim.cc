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
#include "horizontal_trim.h"

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/numeric.h>

// Lib:
#include <boost/format.hpp>

// Standard:
#include <cstddef>


HorizontalTrim::HorizontalTrim (xf::Graphics const& graphics, std::string_view const& instance):
	HorizontalTrimIO (instance),
	InstrumentSupport (graphics)
{
	_inputs_observer.set_callback ([&]{
		mark_dirty();
	});
	_inputs_observer.observe ({
		&_io.trim_value,
		&_io.trim_reference,
		&_io.trim_reference_minimum,
		&_io.trim_reference_maximum,
	});
}


void
HorizontalTrim::process (xf::Cycle const& cycle)
{
	_inputs_observer.process (cycle.update_time());
}


std::packaged_task<void()>
HorizontalTrim::paint (xf::PaintRequest paint_request) const
{
	PaintingParams params;
	params.label = *_io.label;
	params.label_min = *_io.label_min;
	params.label_max = *_io.label_max;
	params.trim_value = _io.trim_value.get_optional();
	params.trim_reference = _io.trim_reference.get_optional();
	params.trim_reference_minimum = _io.trim_reference_minimum.get_optional();
	params.trim_reference_maximum = _io.trim_reference_maximum.get_optional();

	return std::packaged_task<void()> ([this, pr = std::move (paint_request), pp = std::move (params)] {
		async_paint (pr, pp);
	});
}


void
HorizontalTrim::async_paint (xf::PaintRequest const& paint_request, PaintingParams const& pp) const
{
	auto aids = get_aids (paint_request);
	auto painter = get_painter (paint_request);
	auto trim = pp.trim_value;
	auto& ref = pp.trim_reference;
	auto& ref_min = pp.trim_reference_minimum;
	auto& ref_max = pp.trim_reference_maximum;

	if (trim)
		trim = xf::clamped (*trim, -1.0, +1.0);

	double h = aids->font_2.digit_height;
	double v = aids->width() - h;
	bool within_reference = trim && ref_min && ref_max && (*ref_min <= *trim) && (*trim <= *ref_max);

	QFont label_font = aids->font_2.font;
	QFont min_max_labels_font = aids->font_2.font;
	QFont value_font = aids->font_4.font;
	QFont reference_font = aids->font_2.font;
	QColor cyan = aids->kCyan;
	QTransform center_point_transform;
	center_point_transform.translate (0.5 * aids->width(), 0.6 * aids->height());

	// Scale line:
	QPointF lt (0.5 * (h - aids->width()), -h);
	QPointF rt (0.5 * (aids->width() - h), -h);
	QPolygonF line = QPolygonF()
		<< rt
		<< rt + QPointF (0.0, h)
		<< lt + QPointF (0.0, h)
		<< lt;
	painter.setPen (aids->get_pen (Qt::white, 1.0));
	painter.setFont (min_max_labels_font);
	painter.setTransform (center_point_transform);
	painter.drawPolyline (line);
	painter.drawLine (QPointF (0.0, -0.5 * h), QPointF (0.0, 0.0));
	painter.fast_draw_text (lt + QPointF (-0.5 * h, -0.25 * h), Qt::AlignBottom | Qt::AlignLeft, *pp.label_min);
	painter.fast_draw_text (rt + QPointF (+0.5 * h, -0.25 * h), Qt::AlignBottom | Qt::AlignRight, *pp.label_max);

	// Reference range:
	if (ref_min && ref_max)
	{
		painter.setPen (Qt::NoPen);
		painter.setBrush (Qt::green);
		painter.drawRect (QRectF (QPointF (*ref_min * 0.5 * v, aids->pen_width (0.5)),
								  QPointF (*ref_max * 0.5 * v, aids->pen_width (5.0))));
	}

	// Reference value:
	if (ref)
	{
		painter.setPen (aids->get_pen (aids->kAutopilotColor, 2.0));
		painter.paint (aids->default_shadow(), [&] {
			painter.drawLine (QPointF (*ref * 0.5 * v, aids->pen_width (0.5)),
							  QPointF (*ref * 0.5 * v, aids->pen_width (7.5)));
		});
	}

	// Cyan label:
	painter.setFont (label_font);
	painter.setPen (cyan);
	painter.fast_draw_text (QPointF (0.0, 1.0 * h), Qt::AlignTop | Qt::AlignHCenter, *pp.label);

	// Pointer:
	if (trim)
	{
		QPolygonF triangle = QPolygonF()
			<< QPointF (-0.35 * h, -1.0 * h)
			<< QPointF (0.0, 0.0)
			<< QPointF (+0.35 * h, -1.0 * h);
		triangle << triangle[0];
		QColor color = within_reference ? Qt::green : Qt::white;
		painter.setPen (aids->get_pen (color, 1.0));
		painter.setBrush (color);
		painter.paint (aids->default_shadow(), [&] {
			painter.drawPolygon (triangle.translated (*trim * 0.5 * v, 0.0));
		});
	}

	// Numerical value:
	QString value_str = "   ";

	if (trim)
		value_str = stringify (*trim);

	double x = 0.25 * h;
	QPointF text_hook = QPointF (0.0, -2.0 * h);
	Qt::Alignment alignment = Qt::AlignHCenter | Qt::AlignBottom;
	painter.setPen (aids->get_pen (within_reference ? Qt::green : Qt::white, 1.0));
	painter.setBrush (Qt::NoBrush);
	painter.setFont (value_font);
	QRectF box = painter.get_text_box (text_hook, alignment, value_str).adjusted (-x, 0.0, x, 0.0);
	painter.fast_draw_text (text_hook, alignment, value_str);
	painter.drawRect (box);

	// Numerical reference:
	if (ref)
	{
		QString ref_str = stringify (*ref);
		painter.setPen (aids->get_pen (aids->kAutopilotColor, 1.0));
		painter.setFont (reference_font);
		painter.fast_draw_text (QPointF (box.center().x(), box.top()), Qt::AlignBottom | Qt::AlignHCenter, ref_str);
	}
}


QString
HorizontalTrim::stringify (double value)
{
	QString result = QString::fromStdString ((boost::format ("%+03d") % (std::round (100.0 * value))).str());
	// Remove +/- sign when 00:
	if (result.mid (1, 2) == "00")
		result[0] = ' ';
	return result;
}


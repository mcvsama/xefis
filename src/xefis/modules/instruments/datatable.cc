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

// Qt:
#include <QtWidgets/QLayout>

// Lib:
#include <boost/format.hpp>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/exception_support.h>
#include <xefis/utility/types.h>

// Local:
#include "datatable.h"


Datatable::Line::Line (std::string const& label, xf::PropertyStringifier const& stringifier):
	label (label),
	value (stringifier)
{ }


Datatable::Line::Line (std::string const& label, xf::PropertyStringifier const& stringifier,
					   QColor label_and_value_color):
	label (label),
	label_color (label_and_value_color),
	value (stringifier),
	value_color (label_and_value_color)
{ }


Datatable::Line::Line (std::string const& label, xf::PropertyStringifier const& stringifier,
					   std::optional<QColor> label_color,
					   std::optional<QColor> value_color):
	label (label),
	label_color (label_color.value_or (Qt::white)),
	value (stringifier),
	value_color (value_color.value_or (Qt::white))
{ }


QString
Datatable::Line::stringify() const
{
	return QString::fromStdString (value.to_string());
}


Datatable::Datatable (xf::Xefis*, std::string const& instance):
	Instrument (instance)
{
	_inputs_observer.set_callback ([&]{ mark_dirty(); });

	for (auto& line: _list)
		_inputs_observer.observe (line.value.property());
}


void
Datatable::set_label_font_size (float size)
{
	_label_font_size = size;
}


void
Datatable::set_value_font_size (float size)
{
	_value_font_size = size;
}


void
Datatable::set_alignment (Qt::Alignment alignment)
{
	_alignment = alignment;
}


void
Datatable::process (xf::Cycle const& cycle)
{
	_inputs_observer.process (cycle.update_dt());
}


void
Datatable::paint (xf::PaintRequest& paint_request) const
{
	auto aids = get_aids (paint_request);
	auto painter = get_painter (paint_request);

	QFont label_font = aids->font_1.font;
	QFont value_font = aids->font_1.font;
	label_font.setPixelSize (aids->font_pixel_size (_label_font_size));
	value_font.setPixelSize (aids->font_pixel_size (_value_font_size));

	double line_height = std::max (QFontMetricsF (label_font).height(), QFontMetricsF (value_font).height());
	double empty_height = aids->height() - line_height * _list.size();

	if (_alignment & Qt::AlignVCenter)
		painter.translate (QPointF (0.0, 0.5 * empty_height));
	else if (_alignment & Qt::AlignBottom)
		painter.translate (QPointF (0.0, empty_height));

	for (std::size_t i = 0; i < _list.size(); ++i)
	{
		Line const& line = _list[i];

		QPointF left (0.0, (i + 1) * line_height);
		QPointF right (aids->width(), left.y());

		// Label:
		painter.setFont (label_font);
		painter.setPen (aids->get_pen (line.label_color, 1.0));
		painter.fast_draw_text (left, Qt::AlignLeft | Qt::AlignBottom, QString::fromStdString (line.label));
		// Valu
		painter.setFont (value_font);
		painter.setPen (aids->get_pen (line.value_color, 1.0));
		std::string str_to_paint;

		auto error = xf::handle_format_exception([&] {
			str_to_paint = line.value.to_string();
		});

		if (error)
		{
			painter.setPen (aids->get_pen (Qt::red, 1.0));
			str_to_paint = *error;
		}

		painter.fast_draw_text (right, Qt::AlignRight | Qt::AlignBottom, QString::fromStdString (str_to_paint));
	}
}


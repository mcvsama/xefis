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
#include <xefis/core/v1/window.h>
#include <xefis/utility/format_exception.h>
#include <xefis/utility/types.h>

// Local:
#include "datatable.h"


Datatable::Line::Line (std::string const& label, v2::PropertyStringifier const& stringifier):
	label (label),
	value (stringifier)
{ }


Datatable::Line::Line (std::string const& label, v2::PropertyStringifier const& stringifier,
					   QColor label_and_value_color):
	label (label),
	label_color (label_and_value_color),
	value (stringifier),
	value_color (label_and_value_color)
{ }


Datatable::Line::Line (std::string const& label, v2::PropertyStringifier const& stringifier,
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
	Instrument (instance),
	InstrumentAids (0.5f)
{
	_inputs_observer.set_callback ([&]{ update(); });

	for (auto& line: _list)
		_inputs_observer.observe (line.value.property());
}


void
Datatable::set_label_font_size (xf::FontSize size)
{
	_label_font_size = size;
}


void
Datatable::set_value_font_size (xf::FontSize size)
{
	_value_font_size = size;
}


void
Datatable::set_alignment (Qt::Alignment alignment)
{
	_alignment = alignment;
}


void
Datatable::process (v2::Cycle const& cycle)
{
	_inputs_observer.process (cycle.update_dt());
}


void
Datatable::resizeEvent (QResizeEvent*)
{
	auto xw = dynamic_cast<v1::Window*> (window());
	if (xw)
		set_scaling (xw->pen_scale(), xw->font_scale());

	InstrumentAids::update_sizes (size(), window()->size());
}


void
Datatable::paintEvent (QPaintEvent*)
{
	auto painting_token = get_token (this);
	clear_background();

	QFont label_font = _font_10;
	QFont value_font = _font_10;
	label_font.setPixelSize (*_label_font_size * _master_font_scale);
	value_font.setPixelSize (*_value_font_size * _master_font_scale);

	double line_height = std::max (QFontMetricsF (label_font).height(), QFontMetricsF (value_font).height());
	double empty_height = height() - line_height * _list.size();

	if (_alignment & Qt::AlignVCenter)
		painter().translate (QPointF (0.0, 0.5 * empty_height));
	else if (_alignment & Qt::AlignBottom)
		painter().translate (QPointF (0.0, empty_height));

	for (std::size_t i = 0; i < _list.size(); ++i)
	{
		Line const& line = _list[i];

		QPointF left (0.0, (i + 1) * line_height);
		QPointF right (rect().width(), left.y());

		// Label:
		painter().setFont (label_font);
		painter().setPen (get_pen (line.label_color, 1.0));
		painter().fast_draw_text (left, Qt::AlignLeft | Qt::AlignBottom, QString::fromStdString (line.label));
		// Value:
		painter().setFont (value_font);
		painter().setPen (get_pen (line.value_color, 1.0));
		std::string str_to_paint;

		auto error = xf::handle_format_exception([&] {
			str_to_paint = line.value.to_string();
		});

		if (error)
		{
			painter().setPen (get_pen (Qt::red, 1.0));
			str_to_paint = *error;
		}

		painter().fast_draw_text (right, Qt::AlignRight | Qt::AlignBottom, QString::fromStdString (str_to_paint));
	}
}


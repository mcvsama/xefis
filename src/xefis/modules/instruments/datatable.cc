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
#include "datatable.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/types.h>

// Neutrino:
#include <neutrino/exception_support.h>

// Lib:
#include <boost/format.hpp>

// Qt:
#include <QtWidgets/QLayout>

// Standard:
#include <cstddef>


Datatable::Line::Line (std::string_view const& label, xf::BasicSocket const& socket):
	label (label),
	socket (socket)
{ }


Datatable::Line::Line (std::string_view const& label, xf::BasicSocket const& socket,
					   QColor label_and_value_color):
	label (label),
	label_color (label_and_value_color),
	value_color (label_and_value_color),
	socket (socket)
{ }


Datatable::Line::Line (std::string_view const& label, xf::BasicSocket const& socket,
					   std::optional<QColor> label_color,
					   std::optional<QColor> value_color):
	label (label),
	label_color (label_color.value_or (Qt::white)),
	value_color (value_color.value_or (Qt::white)),
	socket (socket)
{ }


void
Datatable::Line::read()
{
	*_stringified.lock() = QString::fromStdString (socket.to_string());
}


QString
Datatable::Line::stringified() const
{
	return *_stringified.lock();
}


Datatable::Datatable (xf::Graphics const& graphics, std::string_view const& instance):
	Instrument (instance),
	InstrumentSupport (graphics)
{
	_inputs_observer.set_callback ([&]{
		// Read lines from main thread:
		for (auto& line: _list)
			line.read();

		mark_dirty();
	});

	for (auto& line: _list)
		_inputs_observer.observe (line.socket);
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
	_inputs_observer.process (cycle.update_time());
}


std::packaged_task<void()>
Datatable::paint (xf::PaintRequest paint_request) const
{
	return std::packaged_task<void()> ([this, pr = std::move (paint_request)] {
		async_paint (pr);
	});
}


void
Datatable::async_paint (xf::PaintRequest const& paint_request) const
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
		QString str_to_paint;

		auto error = xf::handle_format_exception([&] {
			str_to_paint = line.stringified();
		});

		if (error)
		{
			painter.setPen (aids->get_pen (Qt::red, 1.0));
			str_to_paint = QString::fromStdString (*error);
		}

		painter.fast_draw_text (right, Qt::AlignRight | Qt::AlignBottom, str_to_paint);
	}
}


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
#include <QTimer>
#include <QPainter>
#include <QPaintEvent>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/services.h>
#include <xefis/core/v2/module.h>
#include <xefis/utility/responsibility.h>

// Local:
#include "screen.h"


namespace xf {

Screen::Screen (QRect rect, si::Frequency refresh_rate):
	QWidget (nullptr)
{
	move (rect.topLeft());
	resize (rect.size());
	setFont (xf::Services::instrument_font());
	setCursor (QCursor (Qt::CrossCursor));
	show();

	_refresh_timer = new QTimer (this);
	_refresh_timer->setSingleShot (false);
	_refresh_timer->setInterval ((1.0 / refresh_rate).in<si::Millisecond>());
	QObject::connect (_refresh_timer, &QTimer::timeout, this, &Screen::refresh);
	_refresh_timer->start();
}


void
Screen::set (BasicInstrument const& instrument, QRect rect)
{
	for (auto& disclosure: *this)
	{
		if (&disclosure.registrant() == &instrument)
		{
			disclosure.details().rect = rect;
			break;
		}
	}
}


void
Screen::paintEvent (QPaintEvent* paint_event)
{
	QPainter painter (this);
	auto const rect = paint_event->region().boundingRect();
	painter.drawImage (rect, _canvas, rect);
}


void
Screen::resizeEvent (QResizeEvent* resize_event)
{
	_canvas = QImage (resize_event->size(), QImage::Format_ARGB32_Premultiplied);
	_canvas.fill (Qt::black);
}


void
Screen::paint_instruments_to_buffer()
{
	// Collect images from all managed instruments.
	// Compose them into one big image.
	_canvas.fill (Qt::black);

	// Ask instruments to paint themselves:
	for (auto& disclosure: *this)
	{
		auto const& instrument = disclosure.registrant();
		auto& details = disclosure.details();

		if (details.rect.isValid())
		{
			prepare_canvas_for_instrument (details.canvas, details.rect.size());
			instrument.paint (details.canvas);
		}
		else
			std::clog << "Instrument " << identifier (instrument) << " has invalid size/position." << std::endl;
	}

	// Merge all images into our painting buffer:
	{
		QPainter canvas_painter (&_canvas);

		for (auto& disclosure: *this)
		{
			auto& instrument = disclosure.registrant();
			auto const& details = disclosure.details();

			if (details.rect.isValid() && instrument.dirty_since_last_check())
				canvas_painter.drawImage (details.rect, details.canvas, QRect (QPoint (0, 0), details.rect.size()));
		}
	}
}


void
Screen::prepare_canvas_for_instrument (QImage& canvas, QSize size)
{
	if (canvas.size() != size)
		canvas = QImage (size, QImage::Format_ARGB32_Premultiplied);

	canvas.fill (Qt::transparent);
}


void
Screen::refresh()
{
	paint_instruments_to_buffer();
	update();
}

} // namespace xf


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
#include <functional>
#include <algorithm>

// Qt:
#include <QTimer>
#include <QPainter>
#include <QPaintEvent>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/services.h>
#include <xefis/utility/qutils.h>

// Local:
#include "screen.h"


namespace xf {

Screen::Screen (ScreenSpec const& spec):
	QWidget (nullptr),
	Registry ([&](typename Registry::Disclosure& disclosure) { instrument_registered (disclosure); },
			  [&](typename Registry::Disclosure& disclosure) { instrument_unregistered (disclosure); }),
	_screen_spec (spec)
{
	auto rect = _screen_spec.position_and_size();

	if (!rect)
		rect = { 0, 0, 1920, 1080 }; // TODO unhardcode - read Screen information; use QDesktopWidget to read screen info.

	move (rect->topLeft());
	resize (rect->size());
	update_canvas (rect->size());
	setFont (xf::Services::instrument_font());
	setCursor (QCursor (Qt::CrossCursor));
	show();

	_refresh_timer = new QTimer (this);
	_refresh_timer->setSingleShot (false);
	_refresh_timer->setInterval ((1.0 / _screen_spec.refresh_rate()).in<si::Millisecond>());
	QObject::connect (_refresh_timer, &QTimer::timeout, this, &Screen::refresh);
	_refresh_timer->start();
}


Screen::~Screen()
{
	// TODO wait for all async jobs to be finished
}


Screen::RegistrationProof
Screen::register_instrument (BasicInstrument& instrument)
{
	// Don't give access to public to Registry interface, so that user doesn't
	// register instrument with some weird Details value.
	return register_object (instrument);
}


void
Screen::set (BasicInstrument const& instrument, QRectF const requested_position)
{
	for (auto& disclosure: *this)
	{
		if (&disclosure.registrant() == &instrument)
		{
			disclosure.details().requested_position = requested_position;
			disclosure.details().computed_position.reset();
			break;
		}
	}
}


void
Screen::set_z_index (BasicInstrument const& instrument, int const new_z_index)
{
	auto found = std::find_if (_z_index_sorted_disclosures.begin(), _z_index_sorted_disclosures.end(),
							   [&instrument](auto const* disclosure) { return &disclosure->registrant() == &instrument; });

	if (found != _z_index_sorted_disclosures.end())
	{
		(*found)->details().z_index = new_z_index;
		sort_by_z_index();
	}
}


inline si::PixelDensity
Screen::pixel_density() const
{
	auto const pos = _screen_spec.position_and_size();
	auto const dia = _screen_spec.diagonal_length();

	if (pos && dia)
		return xf::diagonal (pos->size()) / *dia;
	else
		return logicalDpiY() / 1_in;
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
	update_canvas (resize_event->size());
}


void
Screen::update_canvas (QSize size)
{
	if (_canvas.isNull() || _canvas.size() != size)
	{
		_canvas = allocate_image (size);
		_canvas.fill (Qt::black);

		for (auto& disclosure: *this)
			disclosure.details().computed_position.reset();
	}
}


void
Screen::paint_instruments_to_buffer()
{
	// Collect images from all managed instruments.
	// Compose them into one big image.

	QSize const canvas_size = _canvas.size();

	_canvas.fill (Qt::black);

	// Ask instruments to paint themselves:
	for (auto* disclosure: _z_index_sorted_disclosures)
	{
		auto& instrument = disclosure->registrant();
		auto& details = disclosure->details();

		if (!details.computed_position)
		{
			QPointF const top_left = details.requested_position.topLeft();
			QPointF const bottom_right = details.requested_position.bottomRight();
			auto const w = canvas_size.width();
			auto const h = canvas_size.height();

			details.computed_position = QRect {
				QPoint (top_left.x() * w, top_left.y() * h),
				QPoint (bottom_right.x() * w, bottom_right.y() * h),
			};
		}

		if (details.computed_position->isValid())
		{
			PaintRequest::Metric metric { details.computed_position->size(), pixel_density(), _screen_spec.base_pen_width(), _screen_spec.base_font_height() };

			if (!details.paint_request || details.paint_request->finished())
			{
				// If paint_request exists (and it's finished()), it means previous painting asynchronous.
				if (details.paint_request)
				{
					// TODO DRY
					std::swap (details.canvas, details.ready_canvas);
					details.paint_request.reset();
					details.previous_size = details.computed_position->size();
				}

				// If size changed:
				if (!details.canvas || details.canvas->size() != details.computed_position->size())
					instrument.mark_dirty();

				// If needs repainting:
				if (instrument.dirty_since_last_check())
				{
					prepare_canvas_for_instrument (details.canvas, details.computed_position->size());
					details.paint_request.emplace (*details.canvas, metric, details.previous_size);

					instrument.paint (*details.paint_request);

					if (details.paint_request->finished())
					{
						std::swap (details.canvas, details.ready_canvas);
						details.paint_request.reset();
						details.previous_size = details.computed_position->size();
					}

					// Unfinished PaintRequests will be checked during next paint_instruments_to_buffer().
				}
			}
		}
		else
			std::clog << "Instrument " << identifier (instrument) << " has invalid size/position." << std::endl;
	}

	// Compose all images into our painting buffer:
	{
		QPainter canvas_painter (&_canvas);

		for (auto* disclosure: _z_index_sorted_disclosures)
		{
			auto const& details = disclosure->details();

			if (details.computed_position && details.computed_position->isValid() && details.ready_canvas)
			{
				// Discard images that have different size than requested computed_position->size(), beacuse
				// it means a resize happened during async painting of the instrument.
				if (details.computed_position->size() == details.ready_canvas->size())
					canvas_painter.drawImage (*details.computed_position, *details.ready_canvas, QRect (QPoint (0, 0), details.computed_position->size()));
			}
		}
	}
}


void
Screen::prepare_canvas_for_instrument (std::unique_ptr<QImage>& canvas, QSize size)
{
	if (!canvas)
		canvas = std::make_unique<QImage>();

	if (canvas->isNull() || canvas->size() != size)
		*canvas = allocate_image (size);

	canvas->fill (Qt::transparent);
}


QImage
Screen::allocate_image (QSize size) const
{
	QImage image (size, QImage::Format_ARGB32_Premultiplied);
	int const dots_per_meter = pixel_density().in<si::DotsPerMeter>();

	image.setDotsPerMeterX (dots_per_meter);
	image.setDotsPerMeterY (dots_per_meter);

	return image;
}


void
Screen::instrument_registered (typename Registry::Disclosure& disclosure)
{
	_z_index_sorted_disclosures.push_back (&disclosure);
	sort_by_z_index();
}


void
Screen::instrument_unregistered (typename Registry::Disclosure& disclosure)
{
	auto new_end = std::remove (_z_index_sorted_disclosures.begin(), _z_index_sorted_disclosures.end(), &disclosure);
	_z_index_sorted_disclosures.resize (new_end - _z_index_sorted_disclosures.begin());
}


void
Screen::sort_by_z_index()
{
	std::stable_sort (_z_index_sorted_disclosures.begin(), _z_index_sorted_disclosures.end(),
					  [](auto const* a, auto const* b) { return a->details().z_index < b->details().z_index; });
}


void
Screen::refresh()
{
	paint_instruments_to_buffer();
	update();
}

} // namespace xf


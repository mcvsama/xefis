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
#include <thread>

// Qt:
#include <QPainter>
#include <QPaintEvent>
#include <QShortcut>
#include <QTimer>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/utility/time_helper.h>

// Local:
#include "screen.h"


namespace xf {
namespace detail {

InstrumentDetails::InstrumentDetails (BasicInstrument& instrument, WorkPerformer& work_performer):
	instrument (instrument),
	work_performer (&work_performer)
{ }

} // namespace detail


Screen::Screen (ScreenSpec const& spec, Graphics const& graphics, Machine& machine, std::string_view const& instance, Logger const& logger):
	QWidget (nullptr),
	NamedInstance (instance),
	_machine (machine),
	_logger (logger.with_scope ("<screen>")),
	_instrument_tracker ([&](InstrumentTracker::Disclosure& disclosure) { instrument_registered (disclosure); },
						 [&](InstrumentTracker::Disclosure& disclosure) { instrument_deregistered (disclosure); }),
	_screen_spec (spec),
	_frame_time (1 / spec.refresh_rate())
{
	QRect rect = _screen_spec.position_and_size();

	move (rect.topLeft());
	resize (rect.size());
	update_canvas (rect.size());
	setFont (graphics.instrument_font());
	setCursor (QCursor (Qt::CrossCursor));
	setMouseTracking (true);
	setAttribute (Qt::WA_TransparentForMouseEvents);

	_refresh_timer = new QTimer (this);
	_refresh_timer->setSingleShot (false);
	_refresh_timer->setTimerType (Qt::PreciseTimer);
	_refresh_timer->setInterval ((1.0 / _screen_spec.refresh_rate()).in<si::Millisecond>());
	QObject::connect (_refresh_timer, &QTimer::timeout, this, &Screen::refresh);
	_refresh_timer->start();

	auto* esc = new QShortcut (this);
	esc->setKey (Qt::Key_Escape);
	QObject::connect (esc, &QShortcut::activated, this, &Screen::show_configurator);
}


Screen::~Screen()
{
	wait();
}


void
Screen::set (BasicInstrument const& instrument, QRectF const requested_position, QPointF const anchor_position)
{
	for (auto& disclosure: _instrument_tracker)
	{
		if (&disclosure.value() == &instrument)
		{
			disclosure.details().requested_position = requested_position;
			disclosure.details().anchor_position = anchor_position;
			disclosure.details().computed_position.reset();
			break;
		}
	}
}


void
Screen::set_centered (BasicInstrument const& instrument, QRectF const requested_position)
{
	set (instrument, requested_position, { 0.5f, 0.5f });
}


void
Screen::set_z_index (BasicInstrument const& instrument, int const new_z_index)
{
	auto found = std::find_if (_z_index_sorted_disclosures.begin(), _z_index_sorted_disclosures.end(),
							   [&instrument](auto const* disclosure) { return &disclosure->value() == &instrument; });

	if (found != _z_index_sorted_disclosures.end())
	{
		(*found)->details().z_index = new_z_index;
		sort_by_z_index();
	}
}


void
Screen::set_paint_bounding_boxes (bool enable)
{
	_paint_bounding_boxes = enable;
}


void
Screen::wait()
{
	// Make sure all async painting is finished:
	for (auto& disclosure: _instrument_tracker)
		wait_for_async_paint (disclosure);
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

		for (auto& disclosure: _instrument_tracker)
			disclosure.details().computed_position.reset();
	}
}


void
Screen::paint_instruments_to_buffer()
{
	QSize const canvas_size = _canvas.size();

	_canvas.fill (Qt::black);

	// Ask instruments to paint themselves:
	for (auto* disclosure: _z_index_sorted_disclosures)
	{
		auto& instrument = disclosure->value();
		auto& details = disclosure->details();

		if (!details.computed_position)
		{
			auto const w = canvas_size.width();
			auto const h = canvas_size.height();
			QPointF const top_left { w * details.requested_position.left(), h * details.requested_position.top() };
			QPointF const bottom_right { w * details.requested_position.right(), h * details.requested_position.bottom() };
			QPointF const anchor_position { details.anchor_position.x() * details.requested_position.size().width() * w,
											details.anchor_position.y() * details.requested_position.size().height() * h };

			details.computed_position = QRectF { top_left, bottom_right }.translated (-anchor_position).toRect();
			details.instrument.mark_dirty();
		}

		if (details.computed_position->isValid())
		{
			if (details.result.valid() && is_ready (details.result))
			{
				Exception::catch_and_log (_logger, [&] {
					auto const painting_time = details.result.get();
					auto accounting_api = BasicInstrument::AccountingAPI (instrument);
					accounting_api.set_frame_time (_frame_time);
					accounting_api.add_painting_time (painting_time);
				});

				std::swap (details.canvas, details.canvas_to_use);
			}

			// Start new painting job:
			if (!details.result.valid() && instrument.dirty_since_last_check())
			{
				prepare_canvas_for_instrument (details.canvas, details.computed_position->size());

				PaintRequest::Metric metric (details.computed_position->size(), _screen_spec.pixel_density(), _screen_spec.base_pen_width(), _screen_spec.base_font_height());
				PaintRequest paint_request (*details.canvas, metric, details.previous_size);

				details.previous_size = details.computed_position->size();

				auto task = instrument.paint (std::move (paint_request));

				details.result = details.work_performer->submit ([t = std::move (task)]() mutable noexcept {
					return TimeHelper::measure (t);
				});
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
			auto& details = disclosure->details();

			if (details.computed_position && details.computed_position->isValid())
			{
				if (auto* painted_image = details.canvas_to_use.get())
				{
					// Discard images that have different size than requested computed_position->size(), beacuse
					// it means a resize happened during async painting of the instrument.
					if (details.computed_position->size() == painted_image->size())
						canvas_painter.drawImage (*details.computed_position, *painted_image, QRect (QPoint (0, 0), details.computed_position->size()));

					if (_paint_bounding_boxes)
					{
						canvas_painter.setPen (QPen (QBrush (Qt::red), 2.0));
						canvas_painter.drawRect (*details.computed_position);
					}
				}
			}
		}
	}
}


void
Screen::wait_for_async_paint (InstrumentTracker::Disclosure& disclosure)
{
	auto const& result = disclosure.details().result;

	while (result.valid() && !is_ready (result))
		result.wait();
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
	int const dots_per_meter = _screen_spec.pixel_density().in<si::DotsPerMeter>();

	image.setDotsPerMeterX (dots_per_meter);
	image.setDotsPerMeterY (dots_per_meter);

	return image;
}


void
Screen::instrument_registered (InstrumentTracker::Disclosure& disclosure)
{
	_z_index_sorted_disclosures.push_back (&disclosure);
	sort_by_z_index();
}


void
Screen::instrument_deregistered (InstrumentTracker::Disclosure& disclosure)
{
	wait_for_async_paint (disclosure);
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


void
Screen::show_configurator()
{
	_machine.show_configurator();
}

} // namespace xf


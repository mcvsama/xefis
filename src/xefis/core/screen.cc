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
#include <QTimer>
#include <QPainter>
#include <QPaintEvent>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/utility/time_helper.h>

// Local:
#include "screen.h"


namespace xf {
namespace detail {

static constexpr float kMaxAvailableTimeFactor = 0.75f;


void
InstrumentDetails::handle_finish()
{
	if (paint_request && paint_request->finished())
		get_ready();
}


inline void
InstrumentDetails::get_ready()
{
	std::swap (canvas, ready_canvas);

	if (!paint_request)
		previous_size = computed_position->size();
	else
		previous_size = paint_request->metric().canvas_size();

	paint_request.reset();
}

} // namespace detail


Screen::Screen (ScreenSpec const& spec, Graphics const& graphics):
	QWidget (nullptr),
	_instrument_tracker ([&](InstrumentTracker::Disclosure& disclosure) { instrument_registered (disclosure); },
						 [&](InstrumentTracker::Disclosure& disclosure) { instrument_deregistered (disclosure); }),
	_screen_spec (spec)
{
	QRect rect = _screen_spec.position_and_size();

	move (rect.topLeft());
	resize (rect.size());
	update_canvas (rect.size());
	setFont (graphics.instrument_font());
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
	// Make sure all async painting is finished:
	for (auto& disclosure: _instrument_tracker)
		wait_for_async_paint (disclosure);
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
	auto start_timestamp = TimeHelper::now();

	// Collect images from all managed instruments.
	// Compose them into one big image.

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
		}

		if (details.computed_position->isValid())
		{
			if (!details.paint_request || details.paint_request->finished())
			{
				// If paint_request exists (and it's finished()), it means previous painting was asynchronous.
				details.handle_finish();

				// If size changed:
				if (!details.canvas || details.canvas->size() != details.computed_position->size())
					instrument.mark_dirty();

				// If needs repainting:
				if (instrument.dirty_since_last_check())
				{
					PaintRequest::Metric metric { details.computed_position->size(), _screen_spec.pixel_density(), _screen_spec.base_pen_width(), _screen_spec.base_font_height() };

					prepare_canvas_for_instrument (details.canvas, details.computed_position->size());
					details.paint_request = std::make_unique<PaintRequest> (*details.canvas, metric, details.previous_size);
					instrument.paint (*details.paint_request);
					details.handle_finish();
					// Unfinished PaintRequests will be checked later and also during next paint_instruments_to_buffer().
				}
			}
		}
		else
			std::clog << "Instrument " << identifier (instrument) << " has invalid size/position." << std::endl;
	}

	// Wait at most kMaxAvailableTimeFactor of available frame time to allow checking if
	// asynchronous instruments painting is complete.
	using namespace std::literals::chrono_literals;

	auto const frame_time = 1 / _screen_spec.refresh_rate();
	auto const sync_painting_done = TimeHelper::now();
	auto const available_time = start_timestamp + detail::kMaxAvailableTimeFactor * frame_time - sync_painting_done;

	if (available_time > 0_s)
		std::this_thread::sleep_for (1s * available_time.in<Second>());

	// Compose all images into our painting buffer:
	{
		QPainter canvas_painter (&_canvas);

		for (auto* disclosure: _z_index_sorted_disclosures)
		{
			auto& details = disclosure->details();

			// Perhaps the instrument has finished asynchronous painting?
			details.handle_finish();

			if (details.computed_position && details.computed_position->isValid() && details.ready_canvas)
			{
				// Discard images that have different size than requested computed_position->size(), beacuse
				// it means a resize happened during async painting of the instrument.
				if (details.computed_position->size() == details.ready_canvas->size())
					canvas_painter.drawImage (*details.computed_position, *details.ready_canvas, QRect (QPoint (0, 0), details.computed_position->size()));

				if (_paint_bounding_boxes)
				{
					canvas_painter.setPen (QPen (QBrush (Qt::red), 2.0));
					canvas_painter.drawRect (*details.computed_position);
				}
			}
		}
	}
}


void
Screen::wait_for_async_paint (InstrumentTracker::Disclosure& disclosure)
{
	using namespace std::chrono_literals;

	auto const& paint_request = disclosure.details().paint_request;

	// Wait actively for all painters to finish:
	while (paint_request && !paint_request->finished())
		std::this_thread::sleep_for (0.01s);
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

} // namespace xf


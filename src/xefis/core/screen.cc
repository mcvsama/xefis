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
#include "screen.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>

// Neutrino:
#include <neutrino/time_helper.h>

// Qt:
#include <QPainter>
#include <QPaintEvent>
#include <QShortcut>
#include <QSvgRenderer>
#include <QTimer>

// Standard:
#include <cstddef>
#include <functional>
#include <algorithm>
#include <thread>


namespace xf {
namespace detail {

InstrumentDetails::InstrumentDetails (Instrument& instrument, WorkPerformer& work_performer):
	instrument (instrument),
	work_performer (&work_performer)
{ }


void
InstrumentDetails::compute_position (QSize const canvas_size)
{
	auto const w = canvas_size.width();
	auto const h = canvas_size.height();
	QPointF const top_left { w * this->requested_position.left(), h * this->requested_position.top() };
	QPointF const bottom_right { w * this->requested_position.right(), h * this->requested_position.bottom() };
	QPointF const anchor_position { this->anchor_position.x() * this->requested_position.size().width() * w,
									this->anchor_position.y() * this->requested_position.size().height() * h };

	this->computed_position = QRectF { top_left, bottom_right }.translated (-anchor_position).toRect();
	this->instrument.mark_dirty();
}

} // namespace detail


static constexpr char		kLogoPath[]			= "share/images/xefis.svg";
static constexpr si::Time	kLogoDisplayTime	= 2_s;


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

	_hide_logo_timer = new QTimer (this);
	_hide_logo_timer->setSingleShot (true);
	_hide_logo_timer->setInterval (kLogoDisplayTime.in<si::Millisecond>());
	QObject::connect (_hide_logo_timer, &QTimer::timeout, this, &Screen::hide_logo);
	// Start will be called in the showEvent.

	_refresh_timer = new QTimer (this);
	_refresh_timer->setSingleShot (false);
	_refresh_timer->setTimerType (Qt::PreciseTimer);
	_refresh_timer->setInterval ((1.0 / _screen_spec.refresh_rate()).in<si::Millisecond>());
	QObject::connect (_refresh_timer, &QTimer::timeout, this, &Screen::refresh);
	_refresh_timer->start();

	{
		auto* esc = new QShortcut (this);
		esc->setKey (Qt::Key_Escape);
		QObject::connect (esc, &QShortcut::activated, this, &Screen::show_configurator);
	}
}


Screen::~Screen()
{
	wait();
}


void
Screen::set (Instrument const& instrument, QRectF const requested_position, QPointF const anchor_position)
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
Screen::set_centered (Instrument const& instrument, QRectF const requested_position)
{
	set (instrument, requested_position, { 0.5f, 0.5f });
}


void
Screen::set_z_index (Instrument const& instrument, int const new_z_index)
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


WorkPerformerMetrics const*
Screen::work_performer_metrics_for (WorkPerformer const* work_performer)
{
	if (auto metrics = _work_performer_metrics.find (work_performer);
		metrics != _work_performer_metrics.end())
	{
		return &metrics->second;
	}
	else
		return nullptr;
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
Screen::showEvent (QShowEvent* event)
{
	QWidget::showEvent (event);
	_hide_logo_timer->start();
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
Screen::paint_logo_to_buffer()
{
	auto const lesser_dim = 0.5 * std::min (_canvas.width(), _canvas.height());

	if (!_logo_image)
	{
		_logo_image = allocate_image (QSize (lesser_dim, lesser_dim));
		_logo_image->fill (Qt::transparent);
		QPainter logo_image_painter (&*_logo_image);
		QSvgRenderer (QString (kLogoPath)).render (&logo_image_painter);
	}

	QPainter canvas_painter (&_canvas);
	canvas_painter.drawImage (_canvas.rect().center() - 0.5 * QPoint (lesser_dim, lesser_dim), *_logo_image);
}


void
Screen::update_instruments()
{
	QSize const canvas_size = _canvas.size();

	// Ask instruments to paint themselves:
	for (auto* disclosure: _z_index_sorted_disclosures)
	{
		auto& instrument = disclosure->value();
		auto& details = disclosure->details();

		if (!details.computed_position)
			details.compute_position (canvas_size);

		if (details.computed_position->isValid())
		{
			if (details.result.valid() && is_ready (details.result))
			{
				Exception::catch_and_log (_logger, [&] {
					auto const perf_metrics = details.result.get();
					// Update per-instrument metrics:
					{
						auto accounting_api = Instrument::AccountingAPI (instrument);
						accounting_api.set_frame_time (_frame_time);
						accounting_api.add_painting_time (perf_metrics.painting_time);
					}
					// Update per-WorkPerformer metrics:
					_work_performer_metrics[details.work_performer].start_latencies.push_back (perf_metrics.start_latency);
					_work_performer_metrics[details.work_performer].total_latencies.push_back (perf_metrics.start_latency + perf_metrics.painting_time);
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
				auto request_time = TimeHelper::now();
				auto measured_task = [t = std::move (task), request_time]() mutable noexcept {
					auto const start_time = TimeHelper::now();
					auto const painting_time = TimeHelper::measure (t);

					return detail::PaintPerformanceMetrics {
						start_time - request_time,
						painting_time,
					};
				};

				details.result = details.work_performer->submit (std::move (measured_task));
			}
		}
		else
			std::clog << "Instrument " << identifier (instrument) << " has invalid size/position." << std::endl;
	}
}


void
Screen::compose_instruments()
{
	_canvas.fill (Qt::black);
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
	_z_index_sorted_disclosures.resize (neutrino::to_unsigned (new_end - _z_index_sorted_disclosures.begin()));
}


void
Screen::sort_by_z_index()
{
	std::stable_sort (_z_index_sorted_disclosures.begin(), _z_index_sorted_disclosures.end(),
					  [](auto const* a, auto const* b) { return a->details().z_index < b->details().z_index; });
}


void
Screen::hide_logo()
{
	_displaying_logo = false;
	_logo_image.reset();
}


void
Screen::refresh()
{
	update_instruments();
	compose_instruments();

	if (_displaying_logo)
		paint_logo_to_buffer();

	update();
}


void
Screen::show_configurator()
{
	_machine.show_configurator();
}

} // namespace xf


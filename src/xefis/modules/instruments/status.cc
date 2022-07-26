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
#include "status.h"

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/time_helper.h>

// Standard:
#include <cstddef>
#include <algorithm>
#include <iterator>


Status::Message::Message (std::string_view const& text, Severity severity):
	_text (text),
	_severity (severity)
{ }


void
Status::Message::process (si::Time now)
{
	bool new_condition_up = std::any_of (_conditions.begin(), _conditions.end(), [](auto o) { return o(); });

	if (new_condition_up)
		_outdated.reset();
	else
	{
		_deleted = false;
		_outdated = now;
	}

	_condition_up = new_condition_up;
	_should_be_shown = !_deleted && (_condition_up || outdated (now));
}


QColor
Status::Message::color() const noexcept
{
	if (_outdated)
		return QColor (0x70, 0x70, 0x70);

	switch (_severity)
	{
		case Severity::Notice:
			return Qt::white;

		case Severity::Caution:
			return QColor (255, 200, 50);

		case Severity::Warning:
			return Qt::red;
	}

	return Qt::white;
}


void
Status::Cache::solve_scroll_and_cursor (std::vector<Message*> const& visible_messages)
{
	// Solve cursor_pos:
	if (visible_messages.empty())
	{
		cursor_visible = false;
		cursor_pos = 0;
	}
	else if (cursor_pos >= static_cast<int> (visible_messages.size()))
		cursor_pos = visible_messages.size() - 1;

	// Solve scroll_pos:
	if (cursor_pos >= scroll_pos + max_visible_messages)
		scroll_pos = cursor_pos - max_visible_messages + 1;
	else if (cursor_pos < scroll_pos)
		scroll_pos = cursor_pos;
}


Status::Status (xf::Graphics const& graphics, std::string_view const& instance):
	StatusIO (instance),
	InstrumentSupport (graphics)
{
	_input_cursor_decoder = std::make_unique<xf::SocketDeltaDecoder<>> (_io.cursor_value, [this] (auto delta) {
		if (delta > 0)
		{
			for (int i = 0; i < delta; ++i)
				cursor_up();
		}
		else if (delta < 0)
		{
			for (int i = 0; i > delta; --i)
				cursor_down();
		}
	});

	_input_cursor_decoder->call_action (0);

	_blink_timer = std::make_unique<QTimer>();
	_blink_timer->setInterval (200);
	_blink_timer->setSingleShot (false);
	QObject::connect (_blink_timer.get(), &QTimer::timeout, [&] {
		_blink_show = !_blink_show;
		mark_dirty();
	});
	_blink_timer->start();

	_cursor_hide_timer = std::make_unique<QTimer>();
	_cursor_hide_timer->setInterval (5000);
	_cursor_hide_timer->setSingleShot (true);
	QObject::connect (_cursor_hide_timer.get(), &QTimer::timeout, [&] {
		_cache.lock()->cursor_visible = false;
		mark_dirty();
	});
}


Status::Message&
Status::add_message (std::string_view const& text, Severity severity)
{
	Message& m = _messages.emplace_back (text, severity);
	_hidden_messages.push_back (&m);
	_cache.lock()->solve_scroll_and_cursor (_visible_messages);
	mark_dirty();
	return m;
}


void
Status::process (xf::Cycle const& cycle)
{
	_input_cursor_decoder->process();

	for (auto& message: _messages)
		message.process (cycle.update_time());

	_button_cursor_del.process();
	_button_recall.process();
	_button_clear.process();
	_button_master_caution.process();
	_button_master_warning.process();

	// Move messages that need to be shown to _visible_messages and hidden to _hidden_messages:

	auto to_show = std::remove_if (_hidden_messages.begin(), _hidden_messages.end(),
								   [](Message* m) { return m->should_be_shown(); });

	// Update timestamp if there was anything new to show:
	if (to_show < _hidden_messages.end())
		_last_message_timestamp = xf::TimeHelper::now();

	std::copy (to_show, _hidden_messages.end(), std::back_inserter (_visible_messages));
	_hidden_messages.resize (neutrino::to_unsigned (std::distance (_hidden_messages.begin(), to_show)));

	auto to_hide = std::remove_if (_visible_messages.begin(), _visible_messages.end(),
								   [](Message* m) { return !m->should_be_shown(); });

	std::copy (to_hide, _visible_messages.end(), std::back_inserter (_hidden_messages));
	_visible_messages.resize (neutrino::to_unsigned (std::distance (_visible_messages.begin(), to_hide)));

	// Update CAUTION and WARNING alarms:

	_io.master_caution =
		std::any_of (_visible_messages.begin(), _visible_messages.end(),
					 [](auto const& m) { return m->severity() == Severity::Caution; });

	_io.master_warning =
		std::any_of (_visible_messages.begin(), _visible_messages.end(),
					 [](auto const& m) { return m->severity() == Severity::Warning; });
}


std::packaged_task<void()>
Status::paint (xf::PaintRequest paint_request) const
{
	PaintingParams params;
	params.visible_messages = _visible_messages;

	return std::packaged_task<void()> ([this, pr = std::move (paint_request), pp = std::move (params)] {
		async_paint (pr, pp);
	});
}


void
Status::async_paint (xf::PaintRequest const& paint_request, PaintingParams const& pp) const
{
	auto aids = get_aids (paint_request);
	auto painter = get_painter (paint_request);
	auto cache = _cache.lock();

	if (paint_request.size_changed())
	{
		cache->font = aids->font_3.font;
		float margin = aids->pen_width (2.f);
		QFontMetricsF metrics (cache->font);
		cache->line_height = 0.85 * metrics.height();
		// Compute space needed for more-up/more-down arrows and actual
		// messages viewport.
		cache->arrow_height = 0.5f * cache->line_height;
		cache->viewport = QRectF (margin, cache->arrow_height, aids->width() - 2.f * margin, aids->height() - 2.f * cache->arrow_height);

		if (cache->viewport.height() <= 0)
			cache->max_visible_messages = 0;
		else
			cache->max_visible_messages = cache->viewport.height() / cache->line_height;

		// Fix viewport size to be integral number of shown messages:
		cache->viewport.setHeight (cache->line_height * cache->max_visible_messages);
		cache->solve_scroll_and_cursor (pp.visible_messages);
	}

	// Messages:
	painter.setBrush (Qt::NoBrush);
	painter.setFont (cache->font);
	int const n = std::min<int> (static_cast<int> (pp.visible_messages.size()) - cache->scroll_pos, cache->max_visible_messages);

	for (int i = 0; i < n; ++i)
	{
		Message const* message = pp.visible_messages[neutrino::to_unsigned (i + cache->scroll_pos)];
		painter.setPen (QPen (message->color()));
		painter.fast_draw_text (QPointF (cache->viewport.left(),
										 cache->viewport.top() + cache->line_height * (i + 0.5)),
								Qt::AlignVCenter | Qt::AlignLeft,
								QString::fromStdString (message->text()));
	}

	// Cursor:
	if (cache->cursor_visible)
	{
		float margin = aids->pen_width (1.f);
		QRectF cursor (cache->viewport.left(), cache->viewport.top() + cache->line_height * (cache->cursor_pos - cache->scroll_pos), cache->viewport.width(), cache->line_height);
		cursor.adjust (-margin, 0.0, margin, 0.0);
		painter.setPen (aids->get_pen (Qt::white, 1.2f));
		painter.drawRect (cursor);
	}

	// For up/down arrows:
	painter.setPen (aids->get_pen (Qt::white, 1.f));
	painter.setBrush (Qt::white);

	// Both arrows are blinking:
	if (_blink_show)
	{
		// Up arrow:
		if (cache->scroll_pos > 0)
		{
			QPolygonF arrow = QPolygonF()
				<< QPointF (0.f, -cache->arrow_height)
				<< QPointF (-cache->arrow_height, 0.f)
				<< QPointF (+cache->arrow_height, 0.f);

			painter.drawPolygon (arrow.translated (cache->viewport.center().x(), cache->viewport.top()));
		}

		// Down arrow:
		if (cache->scroll_pos + cache->max_visible_messages < static_cast<int> (pp.visible_messages.size()))
		{
			QPolygonF arrow = QPolygonF()
				<< QPointF (-cache->arrow_height, 0.f)
				<< QPointF (+cache->arrow_height, 0.f)
				<< QPointF (0.f, cache->arrow_height);

			painter.drawPolygon (arrow.translated (cache->viewport.center().x(), cache->viewport.bottom()));
		}
	}
}


void
Status::cursor_up()
{
	auto cache = _cache.lock();

	if (!cache->cursor_visible && !_visible_messages.empty())
		cache->cursor_visible = true;
	else if (cache->cursor_pos > 0)
	{
		cache->cursor_pos -= 1;
		cache->solve_scroll_and_cursor (_visible_messages);
	}

	mark_dirty();
	_cursor_hide_timer->start();
}


void
Status::cursor_down()
{
	auto cache = _cache.lock();

	if (!cache->cursor_visible && !_visible_messages.empty())
		cache->cursor_visible = true;
	else if (cache->cursor_pos < static_cast<int> (_visible_messages.size()) - 1)
	{
		cache->cursor_pos += 1;
		cache->solve_scroll_and_cursor (_visible_messages);
	}

	mark_dirty();
	_cursor_hide_timer->start();
}


void
Status::cursor_del()
{
	if (_visible_messages.empty())
		return;

	auto cache = _cache.lock();

	if (!cache->cursor_visible)
		return;

	_hidden_messages.push_back (_visible_messages[neutrino::to_unsigned (cache->cursor_pos)]);
	_visible_messages.erase (_visible_messages.begin() + cache->cursor_pos);
	_cursor_hide_timer->start();
	cache->solve_scroll_and_cursor (_visible_messages);
	mark_dirty();
}


void
Status::recall()
{
	_visible_messages.insert (_visible_messages.end(), _hidden_messages.begin(), _hidden_messages.end());
	_hidden_messages.clear();
	_cache.lock()->solve_scroll_and_cursor (_visible_messages);
	mark_dirty();
}


void
Status::clear()
{
	if (xf::TimeHelper::now() - _last_message_timestamp > *_io.status_minimum_display_time)
	{
		_hidden_messages.insert (_hidden_messages.end(), _visible_messages.begin(), _visible_messages.end());
		_visible_messages.clear();
		_cache.lock()->solve_scroll_and_cursor (_visible_messages);
		mark_dirty();
	}
}


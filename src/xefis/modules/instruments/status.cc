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
#include <algorithm>
#include <iterator>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/xefis.h>
#include <xefis/core/v1/window.h>
#include <xefis/utility/painter.h>
#include <xefis/utility/time_helper.h>

// Local:
#include "status.h"


Status::Message::Message (std::string const& text, Severity severity):
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


Status::Status (std::unique_ptr<StatusIO> module_io, std::string const& instance):
	Instrument (std::move (module_io), instance),
	InstrumentAids (1.0)
{
	_input_cursor_decoder = std::make_unique<xf::DeltaDecoder> (io.input_cursor_value, [this](int delta) {
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

	_input_cursor_decoder->force_callback (0);

	recompute_widget();

	_blink_timer = std::make_unique<QTimer>();
	_blink_timer->setInterval (200);
	_blink_timer->setSingleShot (false);
	QObject::connect (_blink_timer.get(), &QTimer::timeout, [&] {
		_blink_show = !_blink_show;
		update();
	});
	_blink_timer->start();

	_cursor_hide_timer = std::make_unique<QTimer>();
	_cursor_hide_timer->setInterval (5000);
	_cursor_hide_timer->setSingleShot (true);
	QObject::connect (_cursor_hide_timer.get(), &QTimer::timeout, [&] {
		_cursor_visible = false;
		update();
	});
}


Status::Message*
Status::add_message (std::string const& text, Severity severity)
{
	_messages.emplace_back (text, severity);
	auto* m = &_messages.back();
	_hidden_messages.push_back (m);

	solve_scroll_and_cursor();
	update();

	return m;
}


void
Status::process (v2::Cycle const& cycle)
{
	(*_input_cursor_decoder)();

	for (auto& m: _messages)
		m.process (cycle.update_time());

	if (_button_master_caution_pressed())
		io.output_master_caution = false;

	if (_button_master_warning_pressed())
		io.output_master_warning = false;

	if (_button_cursor_del_pressed())
		cursor_del();

	if (_button_recall_pressed())
		recall();

	if (_button_clear_pressed())
		if (xf::TimeHelper::now() - _last_message_timestamp > *io.minimum_display_time)
			clear();

	// Move messages that need to be shown to _visible_messages and hidden to _hidden_messages:

	auto to_show = std::remove_if (_hidden_messages.begin(), _hidden_messages.end(),
								   [](Message* m) { return m->should_be_shown(); });

	// Update timestamp if there was anything new to show:
	if (to_show < _hidden_messages.end())
		_last_message_timestamp = xf::TimeHelper::now();

	std::copy (to_show, _hidden_messages.end(), std::back_inserter (_visible_messages));
	_hidden_messages.resize (std::distance (_hidden_messages.begin(), to_show));

	auto to_hide = std::remove_if (_visible_messages.begin(), _visible_messages.end(),
								   [](Message* m) { return !m->should_be_shown(); });

	std::copy (to_hide, _visible_messages.end(), std::back_inserter (_hidden_messages));
	_visible_messages.resize (std::distance (_visible_messages.begin(), to_hide));

	// Update CAUTION and WARNING alarms:

	io.output_master_caution =
		std::any_of (_visible_messages.begin(), _visible_messages.end(),
					 [](auto const& m) { return m->severity() == Severity::Caution; });

	io.output_master_warning =
		std::any_of (_visible_messages.begin(), _visible_messages.end(),
					 [](auto const& m) { return m->severity() == Severity::Warning; });
}


void
Status::resizeEvent (QResizeEvent*)
{
	auto xw = dynamic_cast<v1::Window*> (window());
	if (xw)
		InstrumentAids::set_scaling (1.2f * xw->pen_scale(), 0.95f * xw->font_scale());

	InstrumentAids::update_sizes (size(), window()->size());

	recompute_widget();
}


void
Status::paintEvent (QPaintEvent*)
{
	auto painting_token = get_token (this);
	clear_background();

	// Messages:
	painter().setBrush (Qt::NoBrush);
	painter().setFont (_font);
	int n = std::min<int> (static_cast<int> (_visible_messages.size()) - _scroll_pos, _max_visible_messages);
	for (int i = 0; i < n; ++i)
	{
		Message const* message = _visible_messages[i + _scroll_pos];
		painter().setPen (QPen (message->color()));
		painter().fast_draw_text (QPointF (_viewport.left(),
										   _viewport.top() + _line_height * (i + 0.5)),
								  Qt::AlignVCenter | Qt::AlignLeft,
								  QString::fromStdString (message->text()));
	}

	// Cursor:
	if (_cursor_visible)
	{
		float margin = pen_width (1.f);
		QRectF cursor (_viewport.left(), _viewport.top() + _line_height * (_cursor_pos - _scroll_pos), _viewport.width(), _line_height);
		cursor.adjust (-margin, 0.0, margin, 0.0);
		painter().setPen (get_pen (Qt::white, 1.2f));
		painter().drawRect (cursor);
	}

	// For up/down arrows:
	painter().setPen (get_pen (Qt::white, 1.f));
	painter().setBrush (Qt::white);

	// Both arrows are blinking:
	if (_blink_show)
	{
		// Up arrow:
		if (_scroll_pos > 0)
		{
			QPolygonF arrow = QPolygonF()
				<< QPointF (0.f, -_arrow_height)
				<< QPointF (-_arrow_height, 0.f)
				<< QPointF (+_arrow_height, 0.f);

			painter().drawPolygon (arrow.translated (_viewport.center().x(), _viewport.top()));
		}

		// Down arrow:
		if (_scroll_pos + _max_visible_messages < static_cast<int> (_visible_messages.size()))
		{
			QPolygonF arrow = QPolygonF()
				<< QPointF (-_arrow_height, 0.f)
				<< QPointF (+_arrow_height, 0.f)
				<< QPointF (0.f, _arrow_height);

			painter().drawPolygon (arrow.translated (_viewport.center().x(), _viewport.bottom()));
		}
	}
}


void
Status::cursor_up()
{
	if (!_cursor_visible && !_visible_messages.empty())
		_cursor_visible = true;
	else if (_cursor_pos > 0)
	{
		_cursor_pos -= 1;
		solve_scroll_and_cursor();
	}

	update();
	_cursor_hide_timer->start();
}


void
Status::cursor_down()
{
	if (!_cursor_visible && !_visible_messages.empty())
		_cursor_visible = true;
	else if (_cursor_pos < static_cast<int> (_visible_messages.size()) - 1)
	{
		_cursor_pos += 1;
		solve_scroll_and_cursor();
	}

	update();
	_cursor_hide_timer->start();
}


void
Status::cursor_del()
{
	if (_visible_messages.empty())
		return;

	if (!_cursor_visible)
		return;

	_hidden_messages.push_back (_visible_messages[_cursor_pos]);
	_visible_messages.erase (_visible_messages.begin() + _cursor_pos);

	_cursor_hide_timer->start();

	solve_scroll_and_cursor();
	update();
}


void
Status::recall()
{
	_visible_messages.insert (_visible_messages.end(), _hidden_messages.begin(), _hidden_messages.end());
	_hidden_messages.clear();

	solve_scroll_and_cursor();
	update();
}


void
Status::clear()
{
	_hidden_messages.insert (_hidden_messages.end(), _visible_messages.begin(), _visible_messages.end());
	_visible_messages.clear();

	solve_scroll_and_cursor();
	update();
}


void
Status::recompute_widget()
{
	float margin = pen_width (2.f);
	_font = _font_16;
	QFontMetricsF metrics (_font);
	_line_height = 0.85 * metrics.height();
	// Compute space needed for more-up/more-down arrows and actual
	// messages viewport.
	_arrow_height = 0.5f * _line_height;
	_viewport = QRectF (margin, _arrow_height, width() - 2.f * margin, height() - 2.f * _arrow_height);
	if (_viewport.height() <= 0)
		_max_visible_messages = 0;
	else
		_max_visible_messages = static_cast<unsigned int> (_viewport.height() / _line_height);
	// Fix viewport size to be integral number of shown messages:
	_viewport.setHeight (_line_height * _max_visible_messages);

	solve_scroll_and_cursor();
}


void
Status::solve_scroll_and_cursor()
{
	// Solve _cursor_pos:
	if (_visible_messages.empty())
	{
		_cursor_visible = false;
		_cursor_pos = 0;
	}
	else if (_cursor_pos >= static_cast<int> (_visible_messages.size()))
		_cursor_pos = _visible_messages.size() - 1;

	// Solve _scroll_pos:
	if (_cursor_pos >= _scroll_pos + _max_visible_messages)
		_scroll_pos = _cursor_pos - _max_visible_messages + 1;
	else if (_cursor_pos < _scroll_pos)
		_scroll_pos = _cursor_pos;
}


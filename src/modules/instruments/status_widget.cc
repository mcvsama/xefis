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
#include <QtGui/QPainter>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v1/window.h>

// Local:
#include "status_widget.h"


constexpr Time StatusWidget::MessageHideTimeout;


void
StatusWidget::Message::mark_as_outdated()
{
	outdated = true;
}


StatusWidget::StatusWidget (QWidget* parent):
	InstrumentWidget (parent),
	InstrumentAids (1.0f)
{
	recompute();

	_blinking_timer = std::make_unique<QTimer>();
	_blinking_timer->setInterval (200);
	_blinking_timer->setSingleShot (false);
	QObject::connect (_blinking_timer.get(), &QTimer::timeout, [&] {
		_blink_status = !_blink_status;
		update();
	});
	_blinking_timer->start();

	_cursor_hide_timer = std::make_unique<QTimer>();
	_cursor_hide_timer->setInterval (5000);
	_cursor_hide_timer->setSingleShot (true);
	QObject::connect (_cursor_hide_timer.get(), &QTimer::timeout, [&] {
		_cursor_visible = false;
		update();
	});
}


uint64_t
StatusWidget::add_message (QString const& message, QColor color)
{
	Message m { _id_generator++, message, false, color };
	_shown_messages.push_back (m);

	solve_scroll_and_cursor();
	update();

	return m.id;
}


void
StatusWidget::remove_message (uint64_t message_id)
{
	// Mark message as outdated:
	Message* msg = find_message (message_id);
	if (msg)
	{
		msg->mark_as_outdated();
		update();
	}
	// Later remove the message:
	QTimer* timer = new QTimer (this);
	timer->setInterval (MessageHideTimeout.quantity<Millisecond>());
	timer->setSingleShot (true);
	QObject::connect (timer, &QTimer::timeout, [this,timer,message_id] {
		do_remove_message (message_id);
		timer->deleteLater();
	});
	timer->start();
}


void
StatusWidget::cursor_up()
{
	if (!_cursor_visible && !_shown_messages.empty())
		_cursor_visible = true;
	else if (_cursor > 0)
	{
		_cursor -= 1;
		solve_scroll_and_cursor();
	}

	update();
	_cursor_hide_timer->start();
}


void
StatusWidget::cursor_down()
{
	if (!_cursor_visible && !_shown_messages.empty())
		_cursor_visible = true;
	else if (_cursor < static_cast<int> (_shown_messages.size()) - 1)
	{
		_cursor += 1;
		solve_scroll_and_cursor();
	}

	update();
	_cursor_hide_timer->start();
}


void
StatusWidget::cursor_del()
{
	if (_shown_messages.empty())
		return;

	if (!_cursor_visible)
		return;

	_hidden_messages.push_back (_shown_messages[_cursor]);
	_shown_messages.erase (_shown_messages.begin() + _cursor);

	_cursor_hide_timer->start();

	solve_scroll_and_cursor();
	update();
}


void
StatusWidget::recall()
{
	_shown_messages.insert (_shown_messages.end(), _hidden_messages.begin(), _hidden_messages.end());
	_hidden_messages.clear();

	solve_scroll_and_cursor();
	update();
}


void
StatusWidget::clear()
{
	_hidden_messages.insert (_hidden_messages.end(), _shown_messages.begin(), _shown_messages.end());
	_shown_messages.clear();

	solve_scroll_and_cursor();
	update();
}


void
StatusWidget::resizeEvent (QResizeEvent* event)
{
	InstrumentWidget::resizeEvent (event);

	auto xw = dynamic_cast<v1::Window*> (window());
	if (xw)
		InstrumentAids::set_scaling (1.2f * xw->pen_scale(), 0.95f * xw->font_scale());

	InstrumentAids::update_sizes (size(), window()->size());

	recompute();
}


void
StatusWidget::paintEvent (QPaintEvent*)
{
	auto painting_token = get_token (this);
	clear_background();

	// Messages:
	painter().setBrush (Qt::NoBrush);
	painter().setFont (_font);
	int n = std::min<int> (static_cast<int> (_shown_messages.size()) - _scroll, _max_shown_messages);
	for (int i = 0; i < n; ++i)
	{
		Message const& message = _shown_messages[i + _scroll];
		if (message.outdated)
			painter().setPen (QPen (QColor (0x70, 0x70, 0x70)));
		else
			painter().setPen (QPen (message.color));
		painter().fast_draw_text (QPointF (_viewport.left(), _viewport.top() + _line_height * (i + 0.5)), Qt::AlignVCenter | Qt::AlignLeft, message.message);
	}

	// Cursor:
	if (_cursor_visible)
	{
		float margin = pen_width (1.f);
		QRectF cursor (_viewport.left(), _viewport.top() + _line_height * (_cursor - _scroll), _viewport.width(), _line_height);
		cursor.adjust (-margin, 0.0, margin, 0.0);
		painter().setPen (get_pen (Qt::white, 1.2f));
		painter().drawRect (cursor);
	}

	// For up/down arrows:
	painter().setPen (get_pen (Qt::white, 1.f));
	painter().setBrush (Qt::white);

	// Both arrows are blinking:
	if (_blink_status)
	{
		// Up arrow:
		if (_scroll > 0)
		{
			QPolygonF arrow = QPolygonF()
				<< QPointF (0.f, -_arrow_height)
				<< QPointF (-_arrow_height, 0.f)
				<< QPointF (+_arrow_height, 0.f);

			painter().drawPolygon (arrow.translated (_viewport.center().x(), _viewport.top()));
		}

		// Down arrow:
		if (_scroll + _max_shown_messages < static_cast<int> (_shown_messages.size()))
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
StatusWidget::recompute()
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
		_max_shown_messages = 0;
	else
		_max_shown_messages = static_cast<unsigned int> (_viewport.height() / _line_height);
	// Fix viewport size to be integral number of shown messages:
	_viewport.setHeight (_line_height * _max_shown_messages);

	solve_scroll_and_cursor();
}


void
StatusWidget::solve_scroll_and_cursor()
{
	// Solve _cursor:
	if (_shown_messages.empty())
	{
		_cursor_visible = false;
		_cursor = 0;
	}
	else if (_cursor >= static_cast<int> (_shown_messages.size()))
		_cursor = _shown_messages.size() - 1;

	// Solve _scroll:
	if (_cursor >= _scroll + _max_shown_messages)
		_scroll = _cursor - _max_shown_messages + 1;
	else if (_cursor < _scroll)
		_scroll = _cursor;
}


void
StatusWidget::do_remove_message (uint64_t message_id)
{
	Messages* vector;
	Messages::iterator* iterator;
	Message* msg = find_message (message_id, &vector, &iterator);
	if (msg)
	{
		vector->erase (*iterator);
		solve_scroll_and_cursor();
		update();
	}
}


StatusWidget::Message*
StatusWidget::find_message (uint64_t message_id, Messages** vector_ptr, Messages::iterator** iterator_ptr)
{
	for (Messages* vector: { &_hidden_messages, &_shown_messages })
	{
		for (Messages::iterator m = vector->begin(); m != vector->end(); ++m)
		{
			if (m->id == message_id)
			{
				if (vector_ptr)
					*vector_ptr = vector;
				if (iterator_ptr)
					*iterator_ptr = &m;
				return &*m;
			}
		}
	}
	return nullptr;
}


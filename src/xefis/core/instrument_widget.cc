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
#include <QtWidgets/QApplication>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/services.h>

// Local:
#include "instrument_widget.h"


namespace xf {

InstrumentWidget::PaintWorkUnit::PaintWorkUnit (InstrumentWidget* widget):
	_widget (widget),
	_image (QSize (1, 1), QImage::Format_ARGB32_Premultiplied)
{ }


void
InstrumentWidget::PaintWorkUnit::pop_params()
{
	// Default implementation does nothing.
}


void
InstrumentWidget::PaintWorkUnit::resized()
{
	// Default implementation does nothing.
}


void
InstrumentWidget::PaintWorkUnit::execute()
{
	RecursiveMutex& m = _widget->_paint_mutex;

	for (;;)
	{
		m.synchronize ([&] {
			std::pair<QSize, QSize> sizes = _widget->threadsafe_sizes();
			if (_image.size() != sizes.first)
			{
				_size = sizes.first;
				_window_size = sizes.second;
				_image = QImage (_size, QImage::Format_ARGB32_Premultiplied);
				resized();
			}
			pop_params();
		});

		bool paint_again = false;

		if (!_image.isNull())
			paint (_image);

		m.synchronize ([&] {
			_widget->_paint_buffer = _image;
			_widget->threadsafe_update();
			paint_again = _widget->_paint_again;
			_widget->_paint_again = false;
			if (!paint_again)
				_widget->_paint_in_progress = false;
		});

		if (!paint_again)
			break;
	}

	_widget->_paint_sem.post();
}


InstrumentWidget::InstrumentWidget (QWidget* parent, WorkPerformer* work_performer):
	QWidget (parent),
	_work_performer (work_performer),
	_paint_sem (1),
	_paint_buffer (size(), QImage::Format_ARGB32_Premultiplied)
{
	setCursor (QCursor (Qt::CrossCursor));
}


void
InstrumentWidget::wait_for_painter()
{
	_paint_sem.wait();
}


std::pair<QSize, QSize>
InstrumentWidget::threadsafe_sizes() const
{
	QSize size;
	QSize window_size;
	_paint_mutex.synchronize ([&]() noexcept {
		size = _threadsafe_size;
		window_size = _threadsafe_window_size;
	});
	return { size, window_size };
}


void
InstrumentWidget::threadsafe_update()
{
	QApplication::postEvent (this, new QEvent (static_cast<QEvent::Type> (UpdateEvent)));
}


void
InstrumentWidget::request_repaint()
{
	if (!_paint_requested)
	{
		_paint_requested = true;
		handle_paint_requested();
	}
}


void
InstrumentWidget::handle_paint_requested()
{
	if (_paint_requested && _visible)
		QApplication::postEvent (this, new QEvent (static_cast<QEvent::Type> (RequestRepaintEvent)));
}


void
InstrumentWidget::push_params()
{
	// Default implementation does nothing.
}


void
InstrumentWidget::resizeEvent (QResizeEvent* event)
{
	QWidget::resizeEvent (event);
	if (_paint_work_unit)
	{
		_paint_mutex.synchronize ([&] {
			_threadsafe_size = size();
			_threadsafe_window_size = window()->size();
			_paint_buffer = QImage (size(), QImage::Format_ARGB32_Premultiplied);
			_paint_buffer.fill (Qt::black);
			request_repaint();
		});
	}
}


void
InstrumentWidget::paintEvent (QPaintEvent*)
{
	QPainter painter (this);
	_paint_mutex.synchronize ([&] {
		painter.drawImage (QPoint (0, 0), _paint_buffer);
	});
}


void
InstrumentWidget::customEvent (QEvent* event)
{
	switch (static_cast<int> (event->type()))
	{
		case UpdateEvent:
			update();
			break;

		case RequestRepaintEvent:
			_paint_requested = false;
			_paint_mutex.synchronize ([&] {
				push_params();
				if (_paint_in_progress)
					_paint_again = true;
				else
				{
					_paint_in_progress = true;
					_paint_sem.wait();
					_work_performer->add (_paint_work_unit);
				}
			});
			break;

		default:
			break;
	}
}


void
InstrumentWidget::showEvent (QShowEvent* event)
{
	_visible = true;
	QWidget::showEvent (event);
	handle_paint_requested();
}


void
InstrumentWidget::hideEvent (QHideEvent* event)
{
	_visible = false;
	QWidget::hideEvent (event);
}

} // namespace xf


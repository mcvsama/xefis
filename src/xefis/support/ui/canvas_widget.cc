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
#include "canvas_widget.h"

// Xefis:
#include <xefis/config/all.h>

// Qt:
#include <QGridLayout>
#include <QLabel>
#include <QPaintEvent>
#include <QPainter>
#include <QWidget>

// Standard:
#include <cstddef>


namespace xf {

CanvasWidget::CanvasWidget (QWidget* parent, Qt::WindowFlags flags):
	QWidget (parent, flags)
{
	setStyleSheet ("QTabWidget::pane > QWidget { margin: 0.2em; }");
}


void
CanvasWidget::resizeEvent (QResizeEvent*)
{
	_canvas.reset();
	update();
}


void
CanvasWidget::paintEvent (QPaintEvent* paint_event)
{
	ensure_canvas_exists();

	if (_dirty)
	{
		update_canvas();
		_dirty = false;
	}

	QPainter painter (this);
	auto const rect = paint_event->region().boundingRect();
	painter.drawImage (rect, *_canvas, rect);
}


void
CanvasWidget::changeEvent (QEvent* event)
{
	if (event->type() == QEvent::EnabledChange)
	{
		mark_dirty();
		update();
	}
}


QImage&
CanvasWidget::canvas()
{
	ensure_canvas_exists();
	return *_canvas;
}


void
CanvasWidget::ensure_canvas_exists()
{
	if (!_canvas)
	{
		_canvas.emplace (size(), QImage::Format_ARGB32_Premultiplied);
		mark_dirty();
	}
}

} // namespace xf


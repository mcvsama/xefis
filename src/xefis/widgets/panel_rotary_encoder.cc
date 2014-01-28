/* vim:ts=4
 *
 * Copyleft 2012…2014  Michał Gawron
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
#include <QtGui/QMouseEvent>
#include <QtGui/QWheelEvent>
#include <QtWidgets/QLayout>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/numeric.h>

// Local:
#include "panel_rotary_encoder.h"


namespace Xefis {

constexpr QSize	PanelRotaryEncoder::KnobSize;
constexpr int	PanelRotaryEncoder::Notches;


PanelRotaryEncoder::PanelRotaryEncoder (QWidget* parent, Panel* panel, QString const& knob_label,
										PropertyBoolean rotate_a, PropertyBoolean rotate_b,
										PropertyBoolean rotate_up, PropertyBoolean rotate_down,
										PropertyBoolean click_property):
	PanelWidget (parent, panel),
	_knob_label (knob_label),
	_rotate_a (rotate_a),
	_rotate_b (rotate_b),
	_rotate_up (rotate_up),
	_rotate_down (rotate_down),
	_click_property (click_property)
{
	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (0);
	layout->addItem (new QSpacerItem (KnobSize.width(), KnobSize.height(), QSizePolicy::Fixed, QSizePolicy::Fixed));

	_click_timer = std::make_unique<QTimer>();
	_click_timer->setSingleShot (true);
	_click_timer->setInterval (10);
	QObject::connect (_click_timer.get(), &QTimer::timeout, [&] {
		if (_click_property.configured())
			_click_property.write (false);
	});

	_rotate_up_timer = std::make_unique<QTimer>();
	_rotate_up_timer->setSingleShot (true);
	_rotate_up_timer->setInterval (10);
	QObject::connect (_rotate_up_timer.get(), &QTimer::timeout, [&] {
		if (_rotate_up.configured())
			_rotate_up.write (false);
	});

	_rotate_down_timer = std::make_unique<QTimer>();
	_rotate_down_timer->setSingleShot (true);
	_rotate_down_timer->setInterval (10);
	QObject::connect (_rotate_down_timer.get(), &QTimer::timeout, [&] {
		if (_rotate_down.configured())
			_rotate_down.write (false);
	});
}


void
PanelRotaryEncoder::paintEvent (QPaintEvent*)
{
	QImage image (size(), QImage::Format_ARGB32_Premultiplied);

	QPainter painter (&image);
	painter.setRenderHint (QPainter::Antialiasing, true);
	painter.setRenderHint (QPainter::SmoothPixmapTransform, true);
	painter.setRenderHint (QPainter::NonCosmeticDefaultPen, true);

	QPolygonF polygon;
	QTransform transform;
	float const g = std::min (KnobSize.width(), KnobSize.height());

	for (unsigned int notch = 0; notch < Notches; ++notch)
	{
		transform.rotate (360.0f / Notches / 2.f);
		polygon << transform.map (QPointF (0.f, 0.40f * g));
		polygon << transform.map (QPointF (0.f, 0.44f * g));
		transform.rotate (360.0f / Notches / 2.f);
		polygon << transform.map (QPointF (0.f, 0.44f * g));
		polygon << transform.map (QPointF (0.f, 0.40f * g));
	}

	float rot = 0;
	if (_value == 1)
		rot = 360.f / Notches / 4.f * 1.f;
	else if (_value == 3)
		rot = 360.f / Notches / 4.f * 2.f;
	else if (_value == 2)
		rot = 360.f / Notches / 4.f * 3.f;

	QColor bg = palette().color (QPalette::Window);
	if (parentWidget())
		bg = parentWidget()->palette().color (QPalette::Window);
	painter.fillRect (rect(), bg);
	painter.translate (width() / 2.f, height() / 2.f);
	painter.rotate (rot);

	painter.translate (1.f, 1.f);
	painter.setBrush (Qt::NoBrush);
	painter.setPen (QPen (palette().color (QPalette::Button).darker (150), 3.5f));
	painter.drawPolygon (polygon);

	painter.translate (-1.f, -1.f);
	painter.setBrush (Qt::NoBrush);
	painter.setPen (QPen (palette().color (QPalette::Button).lighter (400), 2.5f));
	painter.drawPolygon (polygon);

	painter.setBrush (palette().color (QPalette::Button).lighter (200));
	painter.setPen (QPen (palette().color (QPalette::Window).darker (100), 2.f));
	painter.scale (0.94f, 0.94f);
	painter.drawPolygon (polygon);

	QFontMetricsF metrics (font());
	QPainterPath text_path;
	QPointF text_pos (-metrics.width (_knob_label) / 2.f, metrics.height() / 3.5f);
	text_path.addText (text_pos, font(), _knob_label);
	painter.setPen (QPen (Qt::black, 2.25f));
	painter.setBrush (Qt::white);
	painter.resetTransform();
	painter.translate (width() / 2.f, height() / 2.f);
	painter.rotate (_angle.deg());
	painter.drawPath (text_path);
	painter.setPen (QPen (Qt::white, 0.5f));
	painter.drawPath (text_path);

	QPainter (this).drawImage (0, 0, image);
}


void
PanelRotaryEncoder::mousePressEvent (QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		event->accept();
		_mouse_last_position = event->pos();
		_mouse_pressed = true;
	}
	else
		event->ignore();
}


void
PanelRotaryEncoder::mouseReleaseEvent (QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		event->accept();
		_mouse_pressed = false;
	}
	else
		event->ignore();
}


void
PanelRotaryEncoder::mouseMoveEvent (QMouseEvent* event)
{
	if (_mouse_pressed)
	{
		event->accept();
		int pixels = _mouse_last_position.y() - event->pos().y();

		for (int i = 0; i < std::abs (pixels); ++i)
		{
			_value = apply_steps (_value, Xefis::sgn (pixels));
			_angle += Xefis::sgn (pixels) * 360_deg / Notches / 4.f;
			rotate (pixels);
			write();
		}

		_mouse_last_position = event->pos();
	}
}


void
PanelRotaryEncoder::wheelEvent (QWheelEvent* event)
{
	event->accept();

	if (_mouse_pressed)
		return;

	_value = apply_steps (_value, Xefis::sgn (event->delta()));
	_angle += Xefis::sgn (event->delta()) * 360_deg / Notches / 4.f;
	rotate (event->delta());

	write();
}


void
PanelRotaryEncoder::mouseDoubleClickEvent (QMouseEvent*)
{
	if (_click_property.configured())
	{
		_click_property.write (true);
		_click_timer->start();
	}
}


void
PanelRotaryEncoder::write()
{
	_rotate_a.write (_value & 2);
	_rotate_b.write (_value & 1);

	update();
}


void
PanelRotaryEncoder::rotate (int delta)
{
	if (delta > 0)
	{
		_rotate_up.write (true);
		_rotate_up_timer->start();
	}
	else if (delta < 0)
	{
		_rotate_down.write (true);
		_rotate_down_timer->start();
	}
}


uint8_t
PanelRotaryEncoder::apply_steps (uint8_t value, int steps)
{
	bool a = value & 2;
	bool b = value & 1;

	if (steps == 1)
	{
		if (!a && !b)		b = true;
		else if (!a && b)	a = true;
		else if (a && b)	b = false;
		else				a = false;

		value = static_cast<uint8_t> (a) << 1 | static_cast<uint8_t> (b);
	}
	else if (steps == -1)
	{
		if (!a && !b)		a = true;
		else if (!a && b)	b = false;
		else if (a && b)	a = false;
		else				b = true;

		value = static_cast<uint8_t> (a) << 1 | static_cast<uint8_t> (b);
	}

	return value;
}

} // namespace Xefis


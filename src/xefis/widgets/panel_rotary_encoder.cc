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
#include <QtGui/QMouseEvent>
#include <QtGui/QWheelEvent>
#include <QtWidgets/QLayout>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/numeric.h>

// Local:
#include "panel_rotary_encoder.h"


namespace xf {

constexpr QSize	PanelRotaryEncoder::KnobSize;
constexpr int	PanelRotaryEncoder::Notches;


PanelRotaryEncoder::PanelRotaryEncoder (QWidget* parent, Panel* panel, QString const& knob_label,
										PropertyInteger value_property, PropertyBoolean click_property):
	PanelWidget (parent, panel),
	_knob_label (knob_label),
	_value_property (value_property),
	_click_property (click_property)
{
	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (0);
	layout->addItem (new QSpacerItem (KnobSize.width(), KnobSize.height(), QSizePolicy::Fixed, QSizePolicy::Fixed));

	_click_timer = std::make_unique<QTimer>();
	_click_timer->setSingleShot (true);
	_click_timer->setInterval (20);
	QObject::connect (_click_timer.get(), &QTimer::timeout, [&] {
		if (_click_property.configured())
			_click_property.write (false);
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
	PropertyInteger::Type value_mod = floored_mod<PropertyInteger::Type> (_value, 0, 4);
	if (value_mod == 1)
		rot = 360.f / Notches / 4.f * 1.f;
	else if (value_mod == 2)
		rot = 360.f / Notches / 4.f * 2.f;
	else if (value_mod == 3)
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
	painter.rotate (_angle.quantity<Degree>());
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

		_angle += pixels * 360_deg / Notches / 4.0;
		_value += pixels;
		_mouse_last_position = event->pos();

		write();
		update();
	}
}


void
PanelRotaryEncoder::wheelEvent (QWheelEvent* event)
{
	event->accept();

	if (_mouse_pressed)
		return;

	_angle += sgn (event->delta()) * 360_deg / Notches / 4.0;
	_value += sgn (event->delta());

	write();
	update();
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
	if (_value_property.configured())
		_value_property = _value;
}

} // namespace xf


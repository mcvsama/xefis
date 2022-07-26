/* vim:ts=4
 *
 * Copyleft 2022  Michał Gawron
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
#include "widget.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/ui/paint_helper.h>

// Qt:
#include <QGridLayout>
#include <QLabel>
#include <QWidget>

// Standard:
#include <cstddef>


namespace xf {

Widget::Widget (QWidget* parent, Qt::WindowFlags flags):
	QWidget (parent, flags)
{
	setStyleSheet ("QTabWidget::pane > QWidget { margin: 0.2em; }");
}


QWidget*
Widget::create_color_widget (QColor color, QWidget* parent)
{
	auto widget = new QWidget (parent);
	auto palette = widget->palette();

	widget->setAutoFillBackground (true);
	palette.setColor (QPalette::Window, color);
	widget->setPalette (palette);
	widget->update();

	return widget;
}


QWidget*
Widget::create_colored_strip_label (QString const& text, QColor color, Qt::Alignment strip_position, QWidget* parent) const
{
	auto* widget = new QWidget (parent);
	auto const ph = PaintHelper (*widget, widget->palette(), widget->font());

	auto* strip = create_color_widget (color, widget);
	strip->setFixedHeight (ph.em_pixels (0.3f));

	auto* label = new QLabel (text, widget);
	label->setStyleSheet ("margin: 0.15em;");
	label->setAlignment (Qt::AlignLeft);

	QFont font = label->font();
	font.setPixelSize (ph.em_pixels (1.4f));
	label->setFont (font);

	auto* layout = new QGridLayout (widget);
	layout->setMargin (0);
	layout->setSpacing (0);

	if (strip_position & Qt::AlignLeft)
	{
		layout->addWidget (strip, 0, 0);
		layout->addWidget (label, 0, 1);
	}
	else if (strip_position & Qt::AlignRight)
	{
		layout->addWidget (strip, 0, 1);
		layout->addWidget (label, 0, 0);
	}
	else if (strip_position & Qt::AlignTop)
	{
		layout->addWidget (strip, 0, 0);
		layout->addWidget (label, 1, 0);
	}
	else if (strip_position & Qt::AlignBottom)
	{
		layout->addWidget (strip, 1, 0);
		layout->addWidget (label, 0, 0);
	}

	return widget;
}

} // namespace xf


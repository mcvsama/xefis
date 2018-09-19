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
#include <QWidget>

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "widget.h"


namespace xf {

QWidget*
Widget::create_color_widget (QColor color, QWidget* parent)
{
	auto widget = new QWidget (parent);
	auto palette = widget->palette();

	widget->setAutoFillBackground (true);
	palette.setColor (QPalette::Background, color);
	widget->setPalette (palette);
	widget->update();

	return widget;
}

} // namespace xf


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
#include <QtWidgets/QLayout>

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "data_recorder.h"


namespace xf {

DataRecorder::DataRecorder (QWidget* parent):
	QWidget (parent)
{
	setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);

	_graphs_stack = new GraphsStack (this);

	_scroll_area = new QScrollArea (this);
	_scroll_area->setWidgetResizable (true);
	_scroll_area->setWidget (_graphs_stack);

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setSpacing (WidgetSpacing);
	layout->setMargin (0);
	layout->addWidget (_scroll_area);
}

} // namespace xf


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
#include "graphs_stack.h"


namespace xf {

GraphsStack::GraphsStack (QWidget* parent):
	QWidget (parent)
{
	setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);

	_layout = new QVBoxLayout (this);
	_layout->setSpacing (WidgetSpacing);
	_layout->setMargin (0);
}


void
GraphsStack::add_graph (GraphWidget* graph_widget)
{
	_layout->addWidget (graph_widget);
	graph_widget->show();
}

} // namespace xf


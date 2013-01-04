/* vim:ts=4
 *
 * Copyleft 2012…2013  Michał Gawron
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
#include <QtCore/QTimer>
#include <QtWidgets/QLayout>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/property_node.h>

// Local:
#include "property_storage_widget.h"


namespace Xefis {

PropertyStorageWidget::PropertyStorageWidget (PropertyNode* property_node, QWidget* parent):
	QWidget (parent)
{
	_property_tree = new PropertyTreeWidget (property_node, this);

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->addWidget (_property_tree);

	_refresh_timer = new QTimer (this);
	_refresh_timer->setInterval (66); // 15 fps
	QObject::connect (_refresh_timer, SIGNAL (timeout()), _property_tree, SLOT (read()));
	_refresh_timer->start();
}


void
PropertyStorageWidget::read()
{
	_property_tree->read();
}

} // namespace Xefis


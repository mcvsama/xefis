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
#include <xefis/core/property_storage.h>

// Local:
#include "configurator_widget.h"


namespace Xefis {

ConfiguratorWidget::ConfiguratorWidget (QWidget* parent):
	QWidget (parent)
{
	_property_tree_widget = new PropertyTreeWidget (PropertyStorage::default_storage()->root(), this);

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (0);
	layout->addWidget (_property_tree_widget);

	QTimer* timer = new QTimer (this);
	timer->setInterval (1000.0 / 15.0);
	QObject::connect (timer, SIGNAL (timeout()), this, SLOT (read_properties()));
	timer->start();
}


void
ConfiguratorWidget::read_properties()
{
	_property_tree_widget->read();
}

} // namespace Xefis


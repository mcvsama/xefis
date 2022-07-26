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
#include "configurable_items_list.h"
#include "processing_loop_item.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/processing_loop.h>

// Neutrino:
#include <neutrino/qt/qutils.h>

// Qt:
#include <QtWidgets/QTreeWidgetItem>

// Standard:
#include <cstddef>
#include <stdexcept>


namespace xf::configurator {

ProcessingLoopItem::ProcessingLoopItem (ProcessingLoop& processing_loop, QTreeWidget& parent):
	QTreeWidgetItem (&parent, { "", "", "" }),
	_processing_loop (processing_loop)
{
	setup_appereance (*this);
	setText (ConfigurableItemsList::NameColumn, QString::fromStdString (_processing_loop.instance()));
}

} // namespace xf::configurator


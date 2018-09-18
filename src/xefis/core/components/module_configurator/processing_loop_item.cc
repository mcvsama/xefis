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
#include <stdexcept>

// Qt:
#include <QtWidgets/QApplication>
#include <QtWidgets/QTreeWidgetItem>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/processing_loop.h>
#include <xefis/utility/qutils.h>

// Local:
#include "modules_list.h"
#include "processing_loop_item.h"


namespace xf {

ProcessingLoopItem::ProcessingLoopItem (ProcessingLoop& processing_loop, QTreeWidget& parent):
	QTreeWidgetItem (&parent, { "", "", "" }),
	_processing_loop (processing_loop)
{
	setup_appereance (*this);
	setText (ModulesList::NameColumn, QString::fromStdString (_processing_loop.instance()));
}

} // namespace xf


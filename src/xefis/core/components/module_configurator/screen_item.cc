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
#include <QtWidgets/QTreeWidgetItem>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/screen.h>
#include <xefis/utility/qutils.h>

// Local:
#include "configurable_items_list.h"
#include "screen_item.h"


namespace xf::configurator {

ScreenItem::ScreenItem (Screen& screen, QTreeWidget& parent):
	QTreeWidgetItem (&parent, { "", "", "" }),
	_screen (screen)
{
	setup_appereance (*this);
	setText (ConfigurableItemsList::NameColumn, QString::fromStdString (_screen.instance()));
}

} // namespace xf::configurator


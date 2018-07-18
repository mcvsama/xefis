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
#include <QtWidgets/QTreeWidgetItem>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/utility/qutils.h>

// Local:
#include "modules_list.h"
#include "module_item.h"


namespace xf {

ModuleItem::ModuleItem (BasicModule& module, ProcessingLoopItem& parent):
	QTreeWidgetItem (&parent, { "", "", "" }),
	_module (module)
{
	setup_appereance (*this);
	setText (ModulesList::NameColumn, QString::fromStdString (identifier (_module)));
}


void
ModuleItem::update_stats()
{
	try {
		setText (ModulesList::StatsAvgColumn, QString ("%1 s").arg ("TODO"));
		setText (ModulesList::StatsMaxColumn, QString ("%1 s").arg ("TODO"));
	}
	catch (...)
	{
		setText (ModulesList::StatsAvgColumn, "error");
		setText (ModulesList::StatsMaxColumn, "error");
	}
}

} // namespace xf


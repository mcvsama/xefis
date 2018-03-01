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
#include <vector>
#include <stdexcept>

// Qt:
#include <QtWidgets/QApplication>
#include <QtWidgets/QTreeWidgetItem>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/services.h>
#include <xefis/core/v2/module.h>

// Local:
#include "modules_list.h"
#include "modules_list_item.h"


namespace xf {

ModulesListItem::ModulesListItem (v2::BasicModule& module, QTreeWidget* parent):
	QTreeWidgetItem (parent, { "", "", "" }),
	_module (module)
{
	setup_appereance();
	setText (ModulesList::ModuleColumn, QString::fromStdString (identifier (_module)));
}


void
ModulesListItem::update_stats()
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


void
ModulesListItem::setup_appereance()
{
	QSize s = sizeHint (0);
	s.setHeight (Services::default_font_size (treeWidget()->physicalDpiY()));
	setSizeHint (0, s);
}

} // namespace xf


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
#include <xefis/core/accounting.h>
#include <xefis/core/services.h>
#include <xefis/core/v1/module_manager.h>

// Local:
#include "modules_list.h"
#include "modules_list_item.h"


namespace xf {

ModulesListItem::ModulesListItem (v1::Module::Pointer const& module_pointer, v1::ModuleManager* module_manager, QTreeWidget* parent):
	QTreeWidgetItem (parent, { "", "", "" }),
	_module_pointer (module_pointer),
	_module_manager (module_manager)
{
	setup_appereance();

	QString name = QString::fromStdString (module_pointer.name());
	QString instance = QString::fromStdString (module_pointer.instance());
	setText (ModulesList::ModuleColumn, name + (instance.isEmpty() ? QString() : (" • " + instance)));
}


void
ModulesListItem::reload()
{
	v1::Module* module = _module_manager->find (_module_pointer);
	if (module)
	{
		try {
			Accounting::Stats const& ms = _module_manager->xefis()->accounting()->module_stats (_module_pointer, Accounting::Timespan::Last100Samples);
			setText (ModulesList::StatsAvgColumn, QString ("%1 s").arg (ms.average().quantity<Second>(), 0, 'f', 6));
			setText (ModulesList::StatsMaxColumn, QString ("%1 s").arg (ms.maximum().quantity<Second>(), 0, 'f', 6));
		}
		catch (...)
		{
			setText (ModulesList::StatsAvgColumn, "?");
			setText (ModulesList::StatsMaxColumn, "?");
		}
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


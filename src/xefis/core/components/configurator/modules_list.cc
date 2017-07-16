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
#include <QtWidgets/QHeaderView>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/services.h>
#include <xefis/core/v1/module_manager.h>

// Local:
#include "modules_list.h"
#include "modules_list_item.h"


namespace xf {

ModulesList::ModulesList (ModuleManager* module_manager, QWidget* parent):
	QWidget (parent),
	_module_manager (module_manager)
{
	_list = new QTreeWidget (this);
	_list->header()->setSectionsClickable (true);
	_list->sortByColumn (ModuleColumn, Qt::AscendingOrder);
	_list->setSortingEnabled (true);
	_list->setSelectionMode (QTreeWidget::SingleSelection);
	_list->setRootIsDecorated (true);
	_list->setAllColumnsShowFocus (true);
	_list->setAcceptDrops (false);
	_list->setAutoScroll (true);
	_list->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	_list->setVerticalScrollMode (QAbstractItemView::ScrollPerPixel);
	_list->setContextMenuPolicy (Qt::CustomContextMenu);
	_list->setHeaderLabels ({ "Module", "Avg latency", "Max latency" });
	QObject::connect (_list, SIGNAL (currentItemChanged (QTreeWidgetItem*, QTreeWidgetItem*)), this, SLOT (item_selected (QTreeWidgetItem*, QTreeWidgetItem*)));

	QHBoxLayout* layout = new QHBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (WidgetSpacing);
	layout->addWidget (_list);

	setMinimumWidth (25.f * Services::default_font_size (physicalDpiY()));

	_refresh_timer = new QTimer (this);
	_refresh_timer->setInterval (100);
	_refresh_timer->setSingleShot (false);
	QObject::connect (_refresh_timer, SIGNAL (timeout()), this, SLOT (read()));
	_refresh_timer->start();

	read();

	_list->header()->resizeSection (ModuleColumn, 14.f * Services::default_font_size (physicalDpiY()));
	_list->header()->resizeSection (StatsAvgColumn, 5.f * Services::default_font_size (physicalDpiY()));
	_list->header()->resizeSection (StatsMaxColumn, 5.f * Services::default_font_size (physicalDpiY()));
}


void
ModulesList::deselect()
{
	_list->clearSelection();
	_list->setCurrentItem (nullptr);
}


void
ModulesList::read()
{
	std::set<Module::Pointer> module_ptrs;
	for (auto m: _module_manager->modules())
		module_ptrs.insert (m.first);

	// Find items that are no longer listed in @modules:
	for (int ci = 0; ci < _list->invisibleRootItem()->childCount(); ++ci)
	{
		ModulesListItem* mli = dynamic_cast<ModulesListItem*> (_list->invisibleRootItem()->child (ci));
		if (!mli)
			continue;

		auto p = module_ptrs.find (mli->module_pointer());

		if (p != module_ptrs.end())
		{
			// Update module/remove from @module_ptrs:
			mli->reload();
			module_ptrs.erase (p);
		}
		else
			delete _list->invisibleRootItem()->takeChild (ci--);
	}

	for (auto p: module_ptrs)
		_list->addTopLevelItem (new ModulesListItem (p, _module_manager, _list));
}


void
ModulesList::item_selected (QTreeWidgetItem* current, QTreeWidgetItem*)
{
	if (current)
	{
		ModulesListItem* mli = dynamic_cast<ModulesListItem*> (current);
		if (mli)
			emit module_selected (mli->module_pointer());
	}
	else
		emit none_selected();
}

} // namespace xf


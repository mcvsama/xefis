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

// Local:
#include "modules_list.h"
#include "modules_list_item.h"


namespace xf {

ModulesList::ModulesList (v2::Machine& machine, QWidget* parent):
	QWidget (parent),
	_machine (machine)
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
	std::set<v2::BasicModule*> module_ptrs;

	// TODO add new top-level item representing a ProcessingLoop
	// TODO fill module_ptrs

	// Find tree widget items that are no longer listed in module_ptrs, to remove them from the tree:
	for (int ci = 0; ci < _list->invisibleRootItem()->childCount(); ++ci)
	{
		if (ModulesListItem* mli = dynamic_cast<ModulesListItem*> (_list->invisibleRootItem()->child (ci)))
		{
			if (auto p = module_ptrs.find (&mli->module()); p != module_ptrs.end())
			{
				// Remove from the list existing module, so that at the end module_ptrs will only contain new modules:
				module_ptrs.erase (p);
			}
			else
				delete _list->invisibleRootItem()->takeChild (ci--);
		}
	}

	// Add new modules:
	for (auto p: module_ptrs)
		_list->addTopLevelItem (new ModulesListItem (*p, _list));
}


void
ModulesList::item_selected (QTreeWidgetItem* current, QTreeWidgetItem*)
{
	if (current)
	{
		ModulesListItem* mli = dynamic_cast<ModulesListItem*> (current);

		if (mli)
			emit module_selected (mli->module());
	}
	else
		emit none_selected();
}

} // namespace xf


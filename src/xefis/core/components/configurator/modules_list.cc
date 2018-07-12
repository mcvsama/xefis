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
#include <set>

// Qt:
#include <QtWidgets/QLayout>
#include <QtWidgets/QHeaderView>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/services.h>

// Local:
#include "modules_list.h"
#include "module_item.h"
#include "processing_loop_item.h"


namespace xf {

ModulesList::ModulesList (xf::Machine& machine, QWidget* parent):
	QWidget (parent),
	_machine (machine)
{
	_list = new QTreeWidget (this);
	_list->header()->setSectionsClickable (true);
	_list->sortByColumn (NameColumn, Qt::AscendingOrder);
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

	_list->header()->resizeSection (NameColumn, 14.f * Services::default_font_size (physicalDpiY()));
	_list->header()->resizeSection (StatsAvgColumn, 5.f * Services::default_font_size (physicalDpiY()));
	_list->header()->resizeSection (StatsMaxColumn, 5.f * Services::default_font_size (physicalDpiY()));

	_processing_loop_ptrs.reserve (100);
	_module_ptrs.reserve (1000);
}


void
ModulesList::deselect()
{
	_list->clearSelection();
	_list->setCurrentItem (nullptr);
}


template<class TempContainer, class ItemToPointerMapper>
	inline void
	populate_subtree (QTreeWidgetItem& tree, TempContainer& container, ItemToPointerMapper&& item_to_pointer)
	{
		for (int ci = 0; ci < tree.childCount(); ++ci)
		{
			if (auto ptr = item_to_pointer (tree.child (ci)))
			{
				auto new_end = std::remove (container.begin(), container.end(), ptr);

				if (new_end != container.end())
					container.resize (std::distance (container.begin(), new_end));
				else
					delete tree.takeChild (ci--);
			}
		}
	}


void
ModulesList::read()
{
	_processing_loop_ptrs.clear();

	for (auto& processing_loop: _machine.processing_loops())
		_processing_loop_ptrs.push_back (processing_loop.get());

	populate_subtree (*_list->invisibleRootItem(), _processing_loop_ptrs, [](auto* item) -> ProcessingLoop* {
		if (auto* pli = dynamic_cast<ProcessingLoopItem*> (item))
			return &pli->processing_loop();
		else
			return nullptr;
	});

	// Add new processing loops:
	for (auto p: _processing_loop_ptrs)
		new ProcessingLoopItem (*p, *_list);

	for (int ci = 0; ci < _list->invisibleRootItem()->childCount(); ++ci)
	{
		if (auto* pli = dynamic_cast<ProcessingLoopItem*> (_list->invisibleRootItem()->child (ci)))
		{
			_module_ptrs.clear();
			ProcessingLoop& processing_loop = pli->processing_loop();

			for (auto& module_details: processing_loop.module_details_list())
				_module_ptrs.push_back (&module_details.module());

			populate_subtree (*pli, _module_ptrs, [](auto* item) -> BasicModule* {
				if (auto* mi = dynamic_cast<ModuleItem*> (item))
					return &mi->module();
				else
					return nullptr;
			});

			// Add new modules:
			for (auto p: _module_ptrs)
				new ModuleItem (*p, *pli);
		}
	}
}


void
ModulesList::item_selected (QTreeWidgetItem* current, QTreeWidgetItem*)
{
	if (current)
	{
		if (ModuleItem* mi = dynamic_cast<ModuleItem*> (current))
			emit module_selected (mi->module());
	}
	else
		emit none_selected();
}

} // namespace xf


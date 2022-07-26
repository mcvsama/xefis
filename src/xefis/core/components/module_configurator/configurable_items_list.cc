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
#include "module_item.h"
#include "processing_loop_item.h"
#include "screen_item.h"

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/qt/qutils.h>

// Qt:
#include <QtWidgets/QLayout>
#include <QtWidgets/QHeaderView>

// Standard:
#include <cstddef>
#include <set>


namespace xf::configurator {

ConfigurableItemsList::ConfigurableItemsList (Machine& machine, QWidget* parent):
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
	_list->setHeaderLabels ({ "Module" });
	QObject::connect (_list, SIGNAL (currentItemChanged (QTreeWidgetItem*, QTreeWidgetItem*)), this, SLOT (item_selected (QTreeWidgetItem*, QTreeWidgetItem*)));

	QHBoxLayout* layout = new QHBoxLayout (this);
	layout->setMargin (0);
	layout->addWidget (_list);

	auto lh = default_line_height (this);
	setMinimumWidth (25.0f * lh);

	_refresh_timer = new QTimer (this);
	_refresh_timer->setInterval (100);
	_refresh_timer->setSingleShot (false);
	QObject::connect (_refresh_timer, SIGNAL (timeout()), this, SLOT (read()));
	_refresh_timer->start();

	read();

	_tmp_processing_loop_ptrs.reserve (100);
	_tmp_module_ptrs.reserve (1000);
}


void
ConfigurableItemsList::deselect()
{
	_list->clearSelection();
	_list->setCurrentItem (nullptr);
}


template<class TempContainer, class ItemToPointerMapper>
	inline void
	ConfigurableItemsList::populate_subtree (QTreeWidgetItem& tree, TempContainer& container, ItemToPointerMapper&& item_to_pointer)
	{
		for (int ci = 0; ci < tree.childCount(); ++ci)
		{
			if (auto ptr = item_to_pointer (tree.child (ci)))
			{
				auto new_end = std::remove (container.begin(), container.end(), ptr);

				if (new_end != container.end())
					container.resize (neutrino::to_unsigned (std::distance (container.begin(), new_end)));
				else
					delete tree.takeChild (ci--);
			}
		}
	}


void
ConfigurableItemsList::read()
{
	// Processing loops:
	for (auto* p: _tmp_processing_loop_ptrs)
		new ProcessingLoopItem (*p, *_list);

	_tmp_processing_loop_ptrs.clear();

	for (auto& processing_loop: _machine.processing_loops())
		_tmp_processing_loop_ptrs.push_back (&processing_loop.value());

	populate_subtree (*_list->invisibleRootItem(), _tmp_processing_loop_ptrs, [](auto* item) -> ProcessingLoop* {
		if (auto* pli = dynamic_cast<ProcessingLoopItem*> (item))
			return &pli->processing_loop();
		else
			return nullptr;
	});

	// Add new processing loops:
	for (auto* p: _tmp_processing_loop_ptrs)
		new ProcessingLoopItem (*p, *_list);

	// Screens:
	_tmp_screen_ptrs.clear();

	for (auto& screen: _machine.screens())
		_tmp_screen_ptrs.push_back (&screen.value());

	populate_subtree (*_list->invisibleRootItem(), _tmp_screen_ptrs, [](auto* item) -> Screen* {
		if (auto* si = dynamic_cast<ScreenItem*> (item))
			return &si->screen();
		else
			return nullptr;
	});

	// Add new processing loops:
	for (auto s: _tmp_screen_ptrs)
		new ScreenItem (*s, *_list);

	for (int ci = 0; ci < _list->invisibleRootItem()->childCount(); ++ci)
	{
		// Add modules registered in a ProcessingLoop:
		if (auto* pli = dynamic_cast<ProcessingLoopItem*> (_list->invisibleRootItem()->child (ci)))
		{
			_tmp_module_ptrs.clear();
			ProcessingLoop& processing_loop = pli->processing_loop();

			for (auto& module_details: processing_loop.module_details_list())
			{
				auto& module = module_details.module();

				// Don't add Instruments. They will be children of Screen items.
				if (!dynamic_cast<Instrument*> (&module))
					_tmp_module_ptrs.push_back (&module);
			}

			populate_subtree (*pli, _tmp_module_ptrs, [](auto* item) -> Module* {
				if (auto* mi = dynamic_cast<ModuleItem*> (item))
					return &mi->module();
				else
					return nullptr;
			});

			// Add new modules:
			for (auto p: _tmp_module_ptrs)
				new ModuleItem (*p, *pli);
		}

		// Add instruments registered in a Screen:
		if (auto* si = dynamic_cast<ScreenItem*> (_list->invisibleRootItem()->child (ci)))
		{
			_tmp_module_ptrs.clear();
			Screen& screen = si->screen();

			for (auto& disclosure: screen.instrument_tracker())
				_tmp_module_ptrs.push_back (&*disclosure.registrant());

			populate_subtree (*si, _tmp_module_ptrs, [](auto* item) -> Module* {
				if (auto* mi = dynamic_cast<ModuleItem*> (item))
					return &mi->module();
				else
					return nullptr;
			});

			// Add new instruments:
			for (auto p: _tmp_module_ptrs)
				new ModuleItem (*p, *si);
		}
	}
}


void
ConfigurableItemsList::item_selected (QTreeWidgetItem* current, QTreeWidgetItem*)
{
	if (current)
	{
		if (ModuleItem* mi = dynamic_cast<ModuleItem*> (current))
			emit module_selected (mi->module());
		else if (ProcessingLoopItem* pli = dynamic_cast<ProcessingLoopItem*> (current))
			emit processing_loop_selected (pli->processing_loop());
		else if (ScreenItem* si = dynamic_cast<ScreenItem*> (current))
			emit screen_selected (si->screen());
	}
	else
		emit none_selected();
}

} // namespace xf::configurator


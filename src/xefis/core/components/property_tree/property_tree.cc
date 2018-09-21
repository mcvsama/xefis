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
#include <QBoxLayout>
#include <QHeaderView>
#include <QTreeWidgetItem>
#include <QTreeWidgetItemIterator>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module_io.h>
#include <xefis/core/property.h>

// Local:
#include "property_tree.h"
#include "property_item.h"


namespace xf {

PropertyTree::PropertyTree (QWidget* parent):
	QWidget (parent)
{
	_tree = new QTreeWidget (this);
	_tree->header()->setSectionsClickable (true);
	_tree->header()->resizeSections (QHeaderView::ResizeToContents);
	_tree->sortByColumn (NameColumn, Qt::AscendingOrder);
	_tree->setSortingEnabled (true);
	_tree->setSelectionMode (QTreeWidget::SingleSelection);
	_tree->setRootIsDecorated (true);
	_tree->setAllColumnsShowFocus (true);
	_tree->setAcceptDrops (false);
	_tree->setAutoScroll (true);
	_tree->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	_tree->setVerticalScrollMode (QAbstractItemView::ScrollPerPixel);
	_tree->setContextMenuPolicy (Qt::CustomContextMenu);
	_tree->setHeaderLabels ({ "Property", "Use count", "Actual value",  "Set value", "Fallback value" });

	QHBoxLayout* layout = new QHBoxLayout (this);
	layout->setMargin (0);
	layout->addWidget (_tree);

	_refresh_timer = new QTimer (this);
	_refresh_timer->setInterval ((100_ms).in<si::Millisecond>());
	QObject::connect (_refresh_timer, &QTimer::timeout, this, &PropertyTree::read_values);

	// TODO QObject::connect (this, &QTreeWidget::customContextMenuRequested, this, &PropertyTreeWidget::handle_context_menu_request);
}


void
PropertyTree::setup_icons()
{
	for (QTreeWidgetItemIterator item (_tree); *item; ++item)
		if (auto* property_item = dynamic_cast<PropertyItem*> (*item))
			property_item->setup_appereance();
}


void
PropertyTree::read_values()
{
	for (QTreeWidgetItemIterator item (_tree); *item; ++item)
		if (auto* property_item = dynamic_cast<PropertyItem*> (*item))
			property_item->read();
}


void
PropertyTree::showEvent (QShowEvent*)
{
	_refresh_timer->start();
}


void
PropertyTree::hideEvent (QHideEvent*)
{
	_refresh_timer->stop();
}

} // namespace xf


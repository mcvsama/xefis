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
#include <xefis/core/sockets/socket.h>

// Local:
#include "socket_tree.h"
#include "socket_item.h"


namespace xf {

SocketTree::SocketTree (QWidget* parent):
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
	_tree->setHeaderLabels ({ "Socket", "Use count", "Actual value",  "Set value", "Fallback value" });

	QHBoxLayout* layout = new QHBoxLayout (this);
	layout->setMargin (0);
	layout->addWidget (_tree);

	_refresh_timer = new QTimer (this);
	_refresh_timer->setInterval ((100_ms).in<si::Millisecond>());
	QObject::connect (_refresh_timer, &QTimer::timeout, this, &SocketTree::read_values);

	// TODO QObject::connect (this, &QTreeWidget::customContextMenuRequested, this, &SocketTreeWidget::handle_context_menu_request);
}


void
SocketTree::setup_icons()
{
	for (QTreeWidgetItemIterator item (_tree); *item; ++item)
		if (auto* socket_item = dynamic_cast<SocketItem*> (*item))
			socket_item->setup_appereance();
}


void
SocketTree::read_values()
{
	for (QTreeWidgetItemIterator item (_tree); *item; ++item)
		if (auto* socket_item = dynamic_cast<SocketItem*> (*item))
			socket_item->read();
}


void
SocketTree::showEvent (QShowEvent*)
{
	_refresh_timer->start();
}


void
SocketTree::hideEvent (QHideEvent*)
{
	_refresh_timer->stop();
}

} // namespace xf


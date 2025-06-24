/* vim:ts=4
 *
 * Copyleft 2025  Michał Gawron
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
#include "group_item.h"

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/qt/qstring.h>

// Qt:
#include <QSignalBlocker>

// Standard:
#include <cstddef>


namespace xf {

GroupItem::GroupItem (QTreeWidget& parent, rigid_body::Group& group):
	QTreeWidgetItem (&parent),
	_group (group)
{
	setFlags (flags() | Qt::ItemIsEditable);
	refresh();
}


GroupItem::GroupItem (QTreeWidgetItem& parent, rigid_body::Group& group):
	QTreeWidgetItem (&parent),
	_group (group)
{
	setFlags (flags() | Qt::ItemIsEditable);
	refresh();
}


void
GroupItem::refresh()
{
	auto const signals_blocker = QSignalBlocker (treeWidget());
	setText (0, neutrino::to_qstring (_group.label()));
}


void
GroupItem::backpropagate()
{
	_group.set_label (text (0).toStdString());
}

} // namespace xf


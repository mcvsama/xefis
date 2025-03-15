/* vim:ts=4
 *
 * Copyleft 2024  Michał Gawron
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
#include "body_item.h"

// Xefis:
#include <xefis/config/all.h>

// Qt:
#include <QSignalBlocker>

// Standard:
#include <cstddef>


namespace xf {

BodyItem::BodyItem (QTreeWidget& parent, rigid_body::Body& body):
	QTreeWidgetItem (&parent),
	_body (body)
{
	setFlags (flags() | Qt::ItemIsEditable);
	refresh();
}


BodyItem::BodyItem (QTreeWidgetItem& parent, rigid_body::Body& body):
	QTreeWidgetItem (&parent),
	_body (body)
{
	setFlags (flags() | Qt::ItemIsEditable);
	refresh();
}


void
BodyItem::refresh()
{
	auto const signals_blocker = QSignalBlocker (treeWidget());

	setText (0, QString::fromStdString (_body.label()));

	if (_body.broken())
		setForeground (0 , QBrush (Qt::gray));
}


void
BodyItem::backpropagate()
{
	_body.set_label (text (0).toStdString());
}

} // namespace xf


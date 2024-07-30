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
#include "constraint_item.h"

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>


namespace xf {

ConstraintItem::ConstraintItem (QTreeWidget& parent, rigid_body::Constraint& constraint):
	QTreeWidgetItem (&parent),
	_constraint (constraint)
{
	setFlags (flags() | Qt::ItemIsEditable);
	refresh();
}


ConstraintItem::ConstraintItem (QTreeWidgetItem& parent, rigid_body::Constraint& constraint):
	QTreeWidgetItem (&parent),
	_constraint (constraint)
{
	setFlags (flags() | Qt::ItemIsEditable);
	refresh();
}


void
ConstraintItem::refresh()
{
	bool was_blocked = false;

	if (auto* tree = treeWidget())
		was_blocked = tree->blockSignals (true);

	setText (0, QString::fromStdString (_constraint.label()));

	if (_constraint.broken())
		setForeground (0 , QBrush (Qt::gray));

	if (auto* tree = treeWidget())
		tree->blockSignals (was_blocked);
}


void
ConstraintItem::backpropagate()
{
	_constraint.set_label (text (0).toStdString());
}

} // namespace xf


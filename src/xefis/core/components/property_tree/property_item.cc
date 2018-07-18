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
#include <QtWidgets/QTreeWidgetItem>

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "property_item.h"
#include "property_tree.h"


namespace xf {

PropertyItem::PropertyItem (BasicProperty* property, QTreeWidgetItem& parent):
	QTreeWidgetItem (&parent),
	_property (property)
{
	if (_property)
		setText (PropertyTree::UseCountColumn, QString::number (_property->use_count()));
}


void
PropertyItem::setup_appereance()
{
	setIcon (0, childCount() > 0
		? resources::icons16::property_dir()
		: resources::icons16::property_value());
}


void
PropertyItem::read()
{
	// TODO call it periodically
	setText (PropertyTree::ActualValueColumn, "x");
	setText (PropertyTree::ForcedValueColumn, "x");
	setText (PropertyTree::SetValueColumn, "x");
	setText (PropertyTree::FallbackValueColumn, "x");
}

} // namespace xf


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
#include <vector>
#include <stdexcept>

// Qt:
#include <QtWidgets/QApplication>
#include <QtWidgets/QTreeWidgetItem>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/services.h>
#include <xefis/core/v1/property_node.h>

// Local:
#include "property_tree_widget.h"
#include "property_tree_widget_item.h"


namespace xf {

PropertyTreeWidgetItem::PropertyTreeWidgetItem (PropertyNode* node, QTreeWidget* parent):
	QTreeWidgetItem (parent, { node->name().c_str() }),
	_node (node)
{
	setup_appereance();
}


PropertyTreeWidgetItem::PropertyTreeWidgetItem (PropertyNode* node, QTreeWidgetItem* parent):
	QTreeWidgetItem (parent, { node->name().c_str() }),
	_node (node)
{
	setup_appereance();
}


void
PropertyTreeWidgetItem::reload()
{
	static_cast<PropertyTreeWidget*> (treeWidget())->read (this, _node);

	// TODO http://qt-project.org/doc/qt-5.0/qtcore/qt.html#ItemFlag-enum
	if (!_node)
		return;

	PropertyValueNode<bool>* node_bool = dynamic_cast<PropertyValueNode<bool>*> (_node);
	if (node_bool)
	{
		setFlags (flags() | Qt::ItemIsUserCheckable);
		setCheckState (1, node_bool->read (false) ? Qt::Checked : Qt::Unchecked);
	}
	else
		setFlags (flags() & ~Qt::ItemIsUserCheckable);
}


bool
PropertyTreeWidgetItem::operator< (QTreeWidgetItem const& that_item) const
{
	PropertyTreeWidgetItem const* that = dynamic_cast<PropertyTreeWidgetItem const*> (&that_item);

	if (that)
	{
		bool const this_is_dir = !!dynamic_cast<PropertyDirectoryNode*> (this->_node);
		bool const that_is_dir = !!dynamic_cast<PropertyDirectoryNode*> (that->_node);

		if (this_is_dir != that_is_dir)
			return this_is_dir;
	}

	return QTreeWidgetItem::operator< (that_item);
}


PropertyTreeWidgetItem*
PropertyTreeWidgetItem::convert_item (QTreeWidgetItem* item)
{
	PropertyTreeWidgetItem* ret = dynamic_cast<PropertyTreeWidgetItem*> (item);
	if (ret)
		return ret;
	throw std::logic_error ("generic QTreeWidgetItem in PropertyTreeWidgetItem");
}


void
PropertyTreeWidgetItem::setup_appereance()
{
	if (dynamic_cast<PropertyDirectoryNode*> (_node))
		setIcon (0, resources::icons16::property_dir());
	else
		setIcon (0, resources::icons16::property_value());
	// Note: setFirstColumnSpanned is extremely slow in Qt, so it's currently disabled.
	// Once it's fixed in future, the following line can be uncommented:
	// setFirstColumnSpanned (!!dynamic_cast<PropertyDirectoryNode*> (_node));
	QSize s = sizeHint (0);
	s.setHeight (Services::default_font_size (treeWidget()->physicalDpiY()));
	setSizeHint (0, s);
}

} // namespace xf


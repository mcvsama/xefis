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

#ifndef XEFIS__CORE__COMPONENTS__PROPERTY_EDITOR__PROPERTY_TREE_WIDGET_ITEM_H__INCLUDED
#define XEFIS__CORE__COMPONENTS__PROPERTY_EDITOR__PROPERTY_TREE_WIDGET_ITEM_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtWidgets/QTreeWidgetItem>

// Xefis:
#include <xefis/config/all.h>


namespace xf {

class PropertyNode;
class PropertyTreeWidget;

class PropertyTreeWidgetItem: public QTreeWidgetItem
{
  public:
	// Ctor
	explicit
	PropertyTreeWidgetItem (PropertyNode* node, QTreeWidget* parent);

	// Ctor
	explicit
	PropertyTreeWidgetItem (PropertyNode* node, QTreeWidgetItem* parent);

	/**
	 * Reads the node and update itself.
	 */
	void
	reload();

	/**
	 * Return PropertyNode* associated with this item.
	 */
	PropertyNode*
	node() const;

	bool
	operator< (QTreeWidgetItem const& other) const override;

  protected:
	/**
	 * Dynamically casts item to PropertyTreeWidgetItem.
	 * If not possible, throws std::logic_error(), as
	 * PropertyTreeWidget should only take PropertyTreeWidgetItems
	 * as children.
	 */
	static PropertyTreeWidgetItem*
	convert_item (QTreeWidgetItem* item);

  private:
	/**
	 * Sets up widget appereance.
	 */
	void
	setup_appereance();

  private:
	PropertyNode* _node;
};


inline PropertyNode*
PropertyTreeWidgetItem::node() const
{
	return _node;
}

} // namespace xf

#endif


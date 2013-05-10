/* vim:ts=4
 *
 * Copyleft 2012…2013  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__COMPONENTS__PROPERTY_EDITOR__PROPERTY_TREE_WIDGET_H__INCLUDED
#define XEFIS__COMPONENTS__PROPERTY_EDITOR__PROPERTY_TREE_WIDGET_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtWidgets/QTreeWidget>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/property_node.h>


namespace Xefis {

class PropertyNode;
class PropertyTreeWidgetItem;

class PropertyTreeWidget: public QTreeWidget
{
	friend class PropertyTreeWidgetItem;

	Q_OBJECT

  public:
	constexpr static int NameColumn		= 0;
	constexpr static int ValueColumn	= 1;

  public:
	PropertyTreeWidget (PropertyNode* root_node, QWidget* parent);

	/**
	 * Return PropertyNode for the selected item.
	 * May return nullptr, if nothing is selected.
	 */
	PropertyNode*
	selected_property_node() const;

  public slots:
	/**
	 * Reads the nodes structure and updates
	 * tree widget.
	 */
	void
	read();

  protected:
	/**
	 * Read data for the given item.
	 */
	void
	read (QTreeWidgetItem*, PropertyNode*);

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
	PropertyNode* _root_node;
};

} // namespace Xefis

#endif


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

#ifndef XEFIS__CORE__COMPONENTS__PROPERTY_EDITOR__PROPERTY_TREE_WIDGET_H__INCLUDED
#define XEFIS__CORE__COMPONENTS__PROPERTY_EDITOR__PROPERTY_TREE_WIDGET_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtCore/QTimer>
#include <QtWidgets/QTreeWidget>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/property_node.h>


namespace xf {

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
	// Ctor
	explicit
	PropertyTreeWidget (PropertyNode* root_node, QWidget* parent);

	/**
	 * Return PropertyNode for the selected item.
	 * May return nullptr, if nothing is selected.
	 */
	PropertyNode*
	selected_property_node() const;

	/**
	 * Return true if given string contains "binary data".
	 */
	static bool
	contains_binary_data (TypedPropertyValueNode const*);

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

  signals:
	/**
	 * Emitted when context menu is requested.
	 */
	void
	context_menu (QTreeWidgetItem*, QPoint const&);

  private slots:
	/**
	 * Right clicked.
	 */
	void
	handle_context_menu_request (const QPoint& pos);

  private:
	/**
	 * Sets up widget appereance.
	 */
	void
	setup_appereance();

	/**
	 * Convert string to colon-delimited hexadecimal form.
	 */
	std::string
	to_binary_form (std::string const& blob);

	// QWidget
	void
	showEvent (QShowEvent*) override;

	// QWidget
	void
	hideEvent (QHideEvent*) override;

  private:
	QTimer*			_refresh_timer;
	PropertyNode*	_root_node;
};

} // namespace xf

#endif


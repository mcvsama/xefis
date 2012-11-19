/* vim:ts=4
 *
 * Copyleft 2008…2012  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__COMPONENTS__PROPERTY_TREE__PROPERTY_STORAGE_WIDGET_H__INCLUDED
#define XEFIS__COMPONENTS__PROPERTY_TREE__PROPERTY_STORAGE_WIDGET_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QWidget>
#include <QtGui/QTreeWidget>

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "property_tree_widget.h"


namespace Xefis {

class PropertyStorageWidget: public QWidget
{
  public:
	PropertyStorageWidget (PropertyNode* root_node, QWidget* parent);

	/**
	 * Reads the nodes structure and updates
	 * tree widget.
	 */
	void
	read();

  private:
	PropertyTreeWidget*	_property_tree;
	QTimer*				_refresh_timer;
};

} // namespace Xefis

#endif


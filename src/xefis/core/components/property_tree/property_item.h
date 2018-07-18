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

#ifndef XEFIS__CORE__COMPONENTS__PROPERTY_TREE__PROPERTY_ITEM_H__INCLUDED
#define XEFIS__CORE__COMPONENTS__PROPERTY_TREE__PROPERTY_ITEM_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtWidgets/QTreeWidgetItem>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/property.h>


namespace xf {

class PropertyItem: public QTreeWidgetItem
{
  public:
	// Ctor
	explicit
	PropertyItem (BasicProperty*, QTreeWidgetItem& parent);

	/**
	 * Should be called after populating the tree with all properties.
	 */
	void
	setup_appereance();

	void
	read();

  private:
	BasicProperty* _property;
 };

} // namespace xf

#endif


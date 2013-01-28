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

#ifndef XEFIS__MODULES__GENERIC__PROPERTY_TREE_H__INCLUDED
#define XEFIS__MODULES__GENERIC__PROPERTY_TREE_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtWidgets/QWidget>
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/instrument.h>
#include <xefis/components/property_tree/property_tree_widget.h>


class PropertyTree: public Xefis::Instrument
{
  public:
	// Ctor
	PropertyTree (Xefis::ModuleManager*, QDomElement const& config, QWidget* parent);

  protected:
	void
	data_updated() override;

  private:
	Xefis::PropertyTreeWidget* _widget = nullptr;
};


inline void
PropertyTree::data_updated()
{
	_widget->read();
}

#endif

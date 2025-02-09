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

#ifndef XEFIS__CORE__COMPONENTS__MODULE_CONFIGURATOR__SCREEN_ITEM_H__INCLUDED
#define XEFIS__CORE__COMPONENTS__MODULE_CONFIGURATOR__SCREEN_ITEM_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/screen.h>

// Qt:
#include <QtWidgets/QTreeWidgetItem>

// Standard:
#include <cstddef>


namespace xf::configurator {

class ScreenItem: public QTreeWidgetItem
{
  public:
	// Ctor
	explicit
	ScreenItem (Screen&, QTreeWidget& parent);

	/**
	 * Return Screen* associated with this item.
	 */
	Screen&
	screen() const noexcept;

  private:
	Screen& _screen;
};


inline Screen&
ScreenItem::screen() const noexcept
{
	return _screen;
}

} // namespace xf::configurator

#endif


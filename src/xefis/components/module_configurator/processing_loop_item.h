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

#ifndef XEFIS__CORE__COMPONENTS__MODULE_CONFIGURATOR__PROCESSING_LOOP_ITEM_H__INCLUDED
#define XEFIS__CORE__COMPONENTS__MODULE_CONFIGURATOR__PROCESSING_LOOP_ITEM_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/processing_loop.h>

// Qt:
#include <QtWidgets/QTreeWidgetItem>

// Standard:
#include <cstddef>


namespace xf::configurator {

class ProcessingLoopItem: public QTreeWidgetItem
{
  public:
	// Ctor
	explicit
	ProcessingLoopItem (ProcessingLoop&, QTreeWidget& parent);

	/**
	 * Return ProcessingLoop* associated with this item.
	 */
	ProcessingLoop&
	processing_loop() const noexcept;

  private:
	ProcessingLoop& _processing_loop;
};


inline ProcessingLoop&
ProcessingLoopItem::processing_loop() const noexcept
{
	return _processing_loop;
}

} // namespace xf::configurator

#endif


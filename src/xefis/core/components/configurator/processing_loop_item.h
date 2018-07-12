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

#ifndef XEFIS__CORE__COMPONENTS__CONFIGURATOR__PROCESSING_LOOP_ITEM_H__INCLUDED
#define XEFIS__CORE__COMPONENTS__CONFIGURATOR__PROCESSING_LOOP_ITEM_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtWidgets/QTreeWidgetItem>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/processing_loop.h>


namespace xf {

class ProcessingLoopItem: public QTreeWidgetItem
{
  public:
	// Ctor
	explicit
	ProcessingLoopItem (xf::ProcessingLoop&, QTreeWidget& parent);

	/**
	 * Return ProcessingLoop* associated with this item.
	 */
	xf::ProcessingLoop&
	processing_loop() const noexcept;

  private:
	xf::ProcessingLoop& _processing_loop;
};


inline xf::ProcessingLoop&
ProcessingLoopItem::processing_loop() const noexcept
{
	return _processing_loop;
}

} // namespace xf

#endif


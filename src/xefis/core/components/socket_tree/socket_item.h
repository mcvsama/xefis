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

#ifndef XEFIS__CORE__COMPONENTS__SOCKET_TREE__SOCKET_ITEM_H__INCLUDED
#define XEFIS__CORE__COMPONENTS__SOCKET_TREE__SOCKET_ITEM_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtWidgets/QTreeWidgetItem>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/sockets/socket.h>


namespace xf {

class SocketItem: public QTreeWidgetItem
{
  public:
	// Ctor
	explicit
	SocketItem (BasicSocket*, QTreeWidgetItem& parent);

	/**
	 * Should be called after populating the tree with all sockets.
	 */
	void
	setup_appereance();

	void
	read();

	bool
	is_dir() const
		{ return childCount() > 0; }

	// API of QTreeWidgetItem
	bool
	operator< (QTreeWidgetItem const&) const override;

  private:
	BasicSocket* _socket;
 };

} // namespace xf

#endif


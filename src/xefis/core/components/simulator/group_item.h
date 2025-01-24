/* vim:ts=4
 *
 * Copyleft 2025  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__CORE__COMPONENTS__SIMULATOR__GROUP_ITEM_H__INCLUDED
#define XEFIS__CORE__COMPONENTS__SIMULATOR__GROUP_ITEM_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/simulation/rigid_body/group.h>

// Qt:
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QTreeWidgetItem>

// Standard:
#include <cstddef>


namespace xf {

class GroupItem: public QTreeWidgetItem
{
  public:
	// Ctor
	explicit
	GroupItem (QTreeWidget& parent, rigid_body::Group&);

	// Ctor
	explicit
	GroupItem (QTreeWidgetItem& parent, rigid_body::Group&);

	[[nodiscard]]
	rigid_body::Group&
	group() noexcept
		{ return _group; }

	[[nodiscard]]
	rigid_body::Group const&
	group() const noexcept
		{ return _group; }

	void
	refresh();

	void
	backpropagate();

  private:
	rigid_body::Group& _group;
};

} // namespace xf

#endif


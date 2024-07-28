/* vim:ts=4
 *
 * Copyleft 2024  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__CORE__COMPONENTS__SIMULATOR__BODIES_TREE_H__INCLUDED
#define XEFIS__CORE__COMPONENTS__SIMULATOR__BODIES_TREE_H__INCLUDED

// Local:
#include "body_item.h"
#include "constraint_item.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/simulation/rigid_body/system.h>
#include <xefis/support/ui/rigid_body_viewer.h>

// Qt:
#include <QIcon>
#include <QTreeWidget>

// Standard:
#include <cstddef>
#include <map>
#include <set>


namespace xf {

class BodiesTree: public QTreeWidget
{
  public:
	// Ctor
	explicit
	BodiesTree (QWidget* parent, rigid_body::System&, RigidBodyViewer&);

	/**
	 * Refreshes the list of bodies from the system.
	 */
	void
	refresh();

  protected:
	/**
	 * Remove items for deleted bodies.
	 * Modify parameters to leave only new bodies and new constraints that need
	 * new items in the tree.
	 */
	void
	remove_deleted (std::set<rigid_body::Body*>& existing_bodies,
					std::set<rigid_body::Constraint*>& existing_constraints,
					std::set<BodyItem*>& body_items_to_update,
					std::set<ConstraintItem*>& constraint_items_to_update,
					std::map<rigid_body::Body*, BodyItem*>& body_to_item);

	void
	recalculate_gravitating_bodies();

	void
	insert_new (std::set<rigid_body::Body*> const& new_bodies,
				std::set<rigid_body::Constraint*> const& new_constraints,
				std::map<rigid_body::Body*, BodyItem*> const& body_to_item);

	void
	update_existing (std::set<BodyItem*> const& body_items,
					 std::set<ConstraintItem*> const& constraint_items);

	void
	set_icon (rigid_body::Body const&, QTreeWidgetItem&);

	void
	set_icon (ConstraintItem&);

	void
	add_constraint_item_to (BodyItem&, rigid_body::Constraint&);

	void
	contextMenuEvent (QContextMenuEvent*);

  private:
	rigid_body::System&		_rigid_body_system;
	RigidBodyViewer&		_rigid_body_viewer;
	rigid_body::Body const*	_followed_body					{ nullptr };
	std::set<rigid_body::Body const*> // TODO flat_set when compiler supports it
							_gravitating_bodies;
	// TODO make an common icon repository with loading directly from SVGs
	QIcon					_body_icon						{ resources::icons16::body() };
	QIcon					_gravitating_body_icon			{ resources::icons16::gravitating_body() };
	QIcon					_followed_body_icon				{ resources::icons16::followed_body() };
	QIcon					_followed_gravitating_body_icon	{ resources::icons16::followed_gravitating_body() };
	QIcon					_constraint_icon				{ resources::icons16::constraint() };
};

} // namespace xf

#endif


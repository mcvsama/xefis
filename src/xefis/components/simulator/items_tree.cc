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

// Local:
#include "items_tree.h"
#include "body_item.h"
#include "constraint_item.h"
#include "group_item.h"

// Xefis:
#include <xefis/config/all.h>

// Qt:
#include <QCursor>
#include <QMenu>
#include <QSignalBlocker>
#include <QTreeWidgetItemIterator>

// Standard:
#include <cstddef>


namespace xf {

ItemsTree::ItemsTree (QWidget* parent, rigid_body::System& system, RigidBodyViewer& viewer):
	QTreeWidget (parent),
	_rigid_body_system (system),
	_rigid_body_viewer (viewer)
{
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	sortByColumn (0, Qt::AscendingOrder);
	setSortingEnabled (true);
	setSelectionMode (QTreeWidget::SingleSelection);
	setRootIsDecorated (true);
	setAllColumnsShowFocus (true);
	setAcceptDrops (false);
	setAutoScroll (true);
	setVerticalScrollMode (QAbstractItemView::ScrollPerPixel);
	setHeaderLabels ({ "Body" });
}


void
ItemsTree::refresh()
{
	// Prevent sending itemChanged() signals when creating new items:
	auto const signals_blocker = QSignalBlocker (this);

	auto groups = std::set<rigid_body::Group*>();

	for (auto const& group: _rigid_body_system.groups())
		groups.insert (group.get());

	auto bodies = std::set<rigid_body::Body*>();

	for (auto const& body: _rigid_body_system.bodies())
		bodies.insert (body.get());

	auto constraints = std::set<rigid_body::Constraint*>();

	for (auto const& constraint: _rigid_body_system.constraints())
		constraints.insert (constraint.get());

	auto group_items_to_update = std::set<GroupItem*>();
	auto body_items_to_update = std::set<BodyItem*>();
	auto constraint_items_to_update = std::set<ConstraintItem*>();
	auto body_to_item = std::map<rigid_body::Body*, BodyItem*>();

	remove_deleted (groups, group_items_to_update,
					bodies, body_items_to_update,
					constraints, constraint_items_to_update,
					body_to_item);
	recalculate_gravitating_bodies();
	insert_new (groups, bodies, constraints, body_to_item);
	update_existing (group_items_to_update, body_items_to_update, constraint_items_to_update);

	// Select first element by default:
	if (selectedItems().empty() && topLevelItemCount() > 0)
		setCurrentItem (topLevelItem (0));

	// Make sure to redraw the viewer after potential changes in the config or system:
	_rigid_body_viewer.update();
}


void
ItemsTree::remove_deleted (std::set<rigid_body::Group*>& existing_groups,
						   std::set<GroupItem*>& group_items_to_update,
						   std::set<rigid_body::Body*>& existing_bodies,
						   std::set<BodyItem*>& body_items_to_update,
						   std::set<rigid_body::Constraint*>& existing_constraints,
						   std::set<ConstraintItem*>& constraint_items_to_update,
						   std::map<rigid_body::Body*, BodyItem*>& body_to_item)
{
	auto groups_to_erase = std::set<rigid_body::Group*>();
	auto bodies_to_erase = std::set<rigid_body::Body*>();
	auto constraints_to_erase = std::set<rigid_body::Constraint*>();
	auto items_to_delete = std::vector<QTreeWidgetItem*>();

	for (QTreeWidgetItemIterator iter (this); *iter; ++iter)
	{
		if (GroupItem* group_item = dynamic_cast<GroupItem*> (*iter))
		{
			auto* group = &group_item->group();

			if (existing_groups.contains (group))
			{
				groups_to_erase.insert (group);
				group_items_to_update.insert (group_item);
			}
			else
			{
				items_to_delete.push_back (group_item);

				if (_followed_group == group)
					_followed_group = nullptr;
			}
		}
		else if (BodyItem* body_item = dynamic_cast<BodyItem*> (*iter))
		{
			auto* body = &body_item->body();

			if (existing_bodies.contains (body))
			{
				bodies_to_erase.insert (body);
				body_items_to_update.insert (body_item);
				body_to_item[body] = body_item;
			}
			else
			{
				items_to_delete.push_back (body_item);

				if (_followed_body == body)
					_followed_body = nullptr;
			}
		}
		else if (ConstraintItem* constraint_item = dynamic_cast<ConstraintItem*> (*iter))
		{
			auto* constraint = &constraint_item->constraint();

			if (existing_constraints.contains (constraint))
			{
				constraints_to_erase.insert (constraint);
				constraint_items_to_update.insert (constraint_item);
			}
			else
				items_to_delete.push_back (constraint_item);
		}
	}

	for (auto* group: groups_to_erase)
		existing_groups.erase (group);

	for (auto* body: bodies_to_erase)
		existing_bodies.erase (body);

	for (auto* constraint: constraints_to_erase)
		existing_constraints.erase (constraint);

	// Deleting parent deletes also children, so first remove children
	// from parents, then do the deleting operation to avoid use-after-free:
	for (auto* item: items_to_delete)
		if (item->parent())
			item->parent()->removeChild (item);

	for (auto* item: items_to_delete)
		delete item;
}


void
ItemsTree::recalculate_gravitating_bodies()
{
	_gravitating_bodies.clear();

	for (auto const* body: _rigid_body_system.gravitating_bodies())
		_gravitating_bodies.insert (body);
}


void
ItemsTree::insert_new (std::set<rigid_body::Group*> const& new_groups,
					   std::set<rigid_body::Body*> new_bodies,
					   std::set<rigid_body::Constraint*> const& new_constraints,
					   std::map<rigid_body::Body*, BodyItem*> const& body_to_item)
{
	// Collect body constraints info:
	auto body_constraints = std::map<rigid_body::Body*, std::set<rigid_body::Constraint*>>();

	for (auto const& constraint: _rigid_body_system.constraints())
	{
		body_constraints[&constraint->body_1()].insert (constraint.get());
		body_constraints[&constraint->body_2()].insert (constraint.get());
	}

	auto const add_body_item_to = [&] (rigid_body::Body& body, auto& parent) {
		BodyItem* new_body_item = new BodyItem (parent, body);
		set_icon (*new_body_item);

		for (auto* constraint: body_constraints[&body])
			add_constraint_item_to (*constraint, *new_body_item);
	};

	// GroupItem items:
	for (auto* group: new_groups)
	{
		GroupItem* new_group_item = new GroupItem (*this, *group);
		set_icon (*new_group_item);

		for (auto* body: group->bodies())
		{
			if (new_bodies.contains (body))
			{
				new_bodies.erase (body);
				add_body_item_to (*body, *new_group_item);
			}
		}
	}

	// Ungrouped (remaining) body items:
	for (auto* body: new_bodies)
		add_body_item_to (*body, *this);

	// Body children (constraints):
	for (auto* constraint: new_constraints)
	{
		auto body_1_iter = body_to_item.find (&constraint->body_1());
		auto body_2_iter = body_to_item.find (&constraint->body_2());

		if (body_1_iter != body_to_item.end() &&
			body_2_iter != body_to_item.end())
		{
			auto* body_1_item = body_1_iter->second;
			auto* body_2_item = body_1_iter->second;

			add_constraint_item_to (*constraint, *body_1_item);
			add_constraint_item_to (*constraint, *body_2_item);
		}
	}
}


void
ItemsTree::update_existing (std::set<GroupItem*> const& group_items,
							std::set<BodyItem*> const& body_items,
							std::set<ConstraintItem*> const& constraint_items)
{
	for (auto* group_item: group_items)
		group_item->refresh();

	for (auto* body_item: body_items)
		body_item->refresh();

	for (auto* constraint_item: constraint_items)
		constraint_item->refresh();

	// Update followed-group icons:
	{
		auto const* old_followed_group = _followed_group;
		auto const* new_followed_group = _rigid_body_viewer.followed_group();

		if (old_followed_group != new_followed_group)
		{
			for (auto* group_item: group_items)
			{
				auto& group = group_item->group();

				if (&group == old_followed_group || &group == new_followed_group)
					set_icon (*group_item);
			}

			_followed_group = _rigid_body_viewer.followed_group();
		}
	}

	// Update followed-body icons:
	{
		auto const* old_followed_body = _followed_body;
		auto const* new_followed_body = _rigid_body_viewer.followed_body();

		if (old_followed_body != new_followed_body)
		{
			for (auto* body_item: body_items)
			{
				auto& body = body_item->body();

				if (&body == old_followed_body || &body == new_followed_body)
					set_icon (*body_item);
			}

			_followed_body = _rigid_body_viewer.followed_body();
		}
	}
}


void
ItemsTree::set_icon (GroupItem& item)
{
	auto const followed = _rigid_body_viewer.followed_group() == &item.group();
	item.setIcon (0, followed ? _followed_group_icon : _group_icon);
}


void
ItemsTree::set_icon (BodyItem& item)
{
	auto const& body = item.body();
	auto const gravitating = _gravitating_bodies.contains (&body);
	auto const followed = _rigid_body_viewer.followed_body() == &body;
	auto const icon = gravitating
		? followed
			? _followed_gravitating_body_icon
			: _gravitating_body_icon
		: followed
			? _followed_body_icon
			: _body_icon;
	item.setIcon (0, icon);
}


void
ItemsTree::set_icon (ConstraintItem& item)
{
	item.setIcon (0, _constraint_icon);
}


void
ItemsTree::add_constraint_item_to (rigid_body::Constraint& constraint, BodyItem& body_item)
{
	auto* constraint_item = new ConstraintItem (body_item, constraint);
	set_icon (*constraint_item);

	auto& connected_body = &body_item.body() == &constraint.body_1()
		? constraint.body_2()
		: constraint.body_1();

	auto* connected_body_item = new BodyItem (*constraint_item, connected_body);
	set_icon (*connected_body_item);
}


void
ItemsTree::contextMenuEvent (QContextMenuEvent* event)
{
	QMenu menu;
	QTreeWidgetItem* item = itemAt (event->pos());

	if (auto* group_item = dynamic_cast<GroupItem*> (item))
	{
		auto& rendering = _rigid_body_viewer.get_rendering_config (group_item->group());

		{
			auto* action = menu.addAction ("&Follow this group", [this, group_item] {
				_rigid_body_viewer.set_followed (group_item->group());
				refresh();
			});
			action->setIcon (_followed_group_icon);
		}

		{
			auto* action = menu.addAction ("Center of mass always visible", [this, group_item, &rendering] {
				rendering.center_of_mass_visible = !rendering.center_of_mass_visible;
				_rigid_body_viewer.update();
			});
			action->setCheckable (true);
			action->setChecked (rendering.center_of_mass_visible);
		}
	}
	else if (auto* body_item = dynamic_cast<BodyItem*> (item))
	{
		auto& rendering = _rigid_body_viewer.get_rendering_config (body_item->body());

		{
			auto* action = menu.addAction ("&Follow this body", [this, body_item] {
				_rigid_body_viewer.set_followed (body_item->body());
				refresh();
			});
			action->setIcon (_followed_body_icon);
		}

		menu.addAction ("&Edit name", [this, body_item] {
			editItem (body_item, 0);
		});

		{
			auto* action = menu.addAction ("Break this body", [this, body_item] {
				body_item->body().set_broken();
				_rigid_body_viewer.update();
				refresh();
			});

			if (body_item->body().broken())
				action->setEnabled (false);
		}

		menu.addSeparator();

		{
			auto* action = menu.addAction ("Body visible", [this, body_item, &rendering] {
				rendering.body_visible = !rendering.body_visible;
				_rigid_body_viewer.update();
			});
			action->setCheckable (true);
			action->setChecked (rendering.body_visible);
		}

		{
			auto* action = menu.addAction ("Origin always visible", [this, body_item, &rendering] {
				rendering.origin_visible = !rendering.origin_visible;
				_rigid_body_viewer.update();
			});
			action->setCheckable (true);
			action->setChecked (rendering.origin_visible);
		}

		{
			auto* action = menu.addAction ("Center of mass always visible", [this, body_item, &rendering] {
				rendering.center_of_mass_visible = !rendering.center_of_mass_visible;
				_rigid_body_viewer.update();
			});
			action->setCheckable (true);
			action->setChecked (rendering.center_of_mass_visible);
		}

		{
			auto* action = menu.addAction ("Moments of inertia cuboid visible", [this, body_item, &rendering] {
				rendering.moments_of_inertia_visible = !rendering.moments_of_inertia_visible;
				_rigid_body_viewer.update();
			});
			action->setCheckable (true);
			action->setChecked (rendering.moments_of_inertia_visible);
		}

		// TODO For airfoils - "Center of pressure visible" (green or light blue?)
	}
	else if (auto* constraint_item = dynamic_cast<ConstraintItem*> (item))
	{
		menu.addAction ("&Edit name", [this, constraint_item] {
			editItem (constraint_item, 0);
		});

		menu.addAction ("Break this constraint", [this, constraint_item] {
			constraint_item->constraint().set_broken();
			_rigid_body_viewer.update();
			refresh();
		});
	}

	menu.exec (event->globalPos());
}


void
ItemsTree::leaveEvent (QEvent*)
{
	itemEntered (nullptr, 0);
}

} // namespace xf


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
#include "bodies_tree.h"
#include "body_item.h"
#include "constraint_item.h"

// Xefis:
#include <xefis/config/all.h>

// Qt:
#include <QCursor>
#include <QMenu>
#include <QTreeWidgetItemIterator>

// Standard:
#include <cstddef>


namespace xf {

BodiesTree::BodiesTree (QWidget* parent, rigid_body::System& system, RigidBodyViewer& viewer):
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
BodiesTree::refresh()
{
	// Prevent sending itemChanged() signals when creating new items:
	bool was_blocked = blockSignals (true);

	auto bodies = std::set<rigid_body::Body*>();

	for (auto const& body: _rigid_body_system.bodies())
		bodies.insert (body.get());

	auto constraints = std::set<rigid_body::Constraint*>();

	for (auto const& constraint: _rigid_body_system.constraints())
		constraints.insert (constraint.get());

	auto body_items_to_update = std::set<BodyItem*>();
	auto constraint_items_to_update = std::set<ConstraintItem*>();
	auto body_to_item = std::map<rigid_body::Body*, BodyItem*>();

	remove_deleted (bodies, constraints, body_items_to_update, constraint_items_to_update, body_to_item);
	recalculate_gravitating_bodies();
	insert_new (bodies, constraints, body_to_item);
	update_existing (body_items_to_update, constraint_items_to_update);

	blockSignals (was_blocked);

	// Select first element by default:
	if (selectedItems().empty() && topLevelItemCount() > 0)
		setCurrentItem (topLevelItem (0));
}


void
BodiesTree::remove_deleted (std::set<rigid_body::Body*>& existing_bodies,
							std::set<rigid_body::Constraint*>& existing_constraints,
							std::set<BodyItem*>& body_items_to_update,
							std::set<ConstraintItem*>& constraint_items_to_update,
							std::map<rigid_body::Body*, BodyItem*>& body_to_item)
{
	auto bodies_to_erase = std::set<rigid_body::Body*>();
	auto constraints_to_erase = std::set<rigid_body::Constraint*>();
	auto items_to_delete = std::vector<QTreeWidgetItem*>();

	for (QTreeWidgetItemIterator iter (this); *iter; ++iter)
	{
		if (BodyItem* body_item = dynamic_cast<BodyItem*> (*iter))
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

	for (auto* body: bodies_to_erase)
		existing_bodies.erase (body);

	for (auto* constraint: constraints_to_erase)
		existing_constraints.erase (constraint);

	// Deleting parent deletes also children, so split removing children
	// from parenst from deleting operation to avoid use-after-free:
	for (auto* item: items_to_delete)
		if (item->parent())
			item->parent()->removeChild (item);

	for (auto* item: items_to_delete)
		delete item;
}


void
BodiesTree::recalculate_gravitating_bodies()
{
	_gravitating_bodies.clear();

	for (auto const* body: _rigid_body_system.gravitating_bodies())
		_gravitating_bodies.insert (body);
}


void
BodiesTree::insert_new (std::set<rigid_body::Body*> const& new_bodies,
						std::set<rigid_body::Constraint*> const& new_constraints,
						std::map<rigid_body::Body*, BodyItem*> const& body_to_item)
{
	// Update body items:
	{
		auto body_constraints = std::map<rigid_body::Body*, std::set<rigid_body::Constraint*>>();

		for (auto const& constraint: _rigid_body_system.constraints())
		{
			body_constraints[&constraint->body_1()].insert (constraint.get());
			body_constraints[&constraint->body_2()].insert (constraint.get());
		}

		for (auto* body: new_bodies)
		{
			BodyItem* new_body_item = new BodyItem (*this, *body);
			set_icon (*body, *new_body_item);

			for (auto* constraint: body_constraints[body])
				add_constraint_item_to (*new_body_item, *constraint);
		}
	}

	// Update body children (constraints):
	{
		for (auto* constraint: new_constraints)
		{
			auto body_1_iter = body_to_item.find (&constraint->body_1());
			auto body_2_iter = body_to_item.find (&constraint->body_2());

			if (body_1_iter != body_to_item.end() &&
				body_2_iter != body_to_item.end())
			{
				auto* body_1_item = body_1_iter->second;
				auto* body_2_item = body_1_iter->second;

				add_constraint_item_to (*body_1_item, *constraint);
				add_constraint_item_to (*body_2_item, *constraint);
			}
		}
	}
}


void
BodiesTree::update_existing (std::set<BodyItem*> const& body_items,
							 std::set<ConstraintItem*> const& constraint_items)
{
	for (auto* body_item: body_items)
		body_item->refresh();

	for (auto* constraint_item: constraint_items)
		constraint_item->refresh();

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
					set_icon (body, *body_item);
			}

			_followed_body = _rigid_body_viewer.followed_body();
		}
	}
}


void
BodiesTree::set_icon (rigid_body::Body const& body, QTreeWidgetItem& item)
{
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
BodiesTree::set_icon (ConstraintItem& item)
{
	item.setIcon (0, _constraint_icon);
}


void
BodiesTree::add_constraint_item_to (BodyItem& body_item, rigid_body::Constraint& constraint)
{
	auto* constraint_item = new ConstraintItem (body_item, constraint);
	set_icon (*constraint_item);

	auto& connected_body = &body_item.body() == &constraint.body_1()
		? constraint.body_2()
		: constraint.body_1();

	auto* connected_body_item = new BodyItem (*constraint_item, connected_body);
	set_icon (connected_body, *connected_body_item);
}


void
BodiesTree::contextMenuEvent (QContextMenuEvent* event)
{
	QMenu menu;

	{
		QTreeWidgetItem* item = itemAt (event->pos());

		if (auto* body_item = dynamic_cast<BodyItem*> (item))
		{
			auto& rendering = _rigid_body_viewer.get_body_rendering_config (body_item->body());

			{
				auto* action = menu.addAction ("&Follow this body", [this, body_item] {
					_rigid_body_viewer.set_followed_body (&body_item->body());
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
					refresh();
				});

				if (body_item->body().broken())
					action->setEnabled (false);
			}

			menu.addSeparator();

			{
				auto* action = menu.addAction ("Body visible", [this, body_item, &rendering] {
					rendering.body_visible = !rendering.body_visible;
				});
				action->setCheckable (true);
				action->setChecked (rendering.body_visible);
			}

			{
				auto* action = menu.addAction ("Origin always visible", [this, body_item, &rendering] {
					rendering.origin_visible = !rendering.origin_visible;
				});
				action->setCheckable (true);
				action->setChecked (rendering.origin_visible);
			}

			{
				auto* action = menu.addAction ("Center of mass always visible", [this, body_item, &rendering] {
					rendering.center_of_mass_visible = !rendering.center_of_mass_visible;
				});
				action->setCheckable (true);
				action->setChecked (rendering.center_of_mass_visible);
			}

			{
				auto* action = menu.addAction ("Moments of inertia cuboid visible", [this, body_item, &rendering] {
					rendering.moments_of_inertia_visible = !rendering.moments_of_inertia_visible;
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
				refresh();
			});
		}
	}

	menu.exec (event->globalPos());
}


void
BodiesTree::leaveEvent (QEvent*)
{
	itemEntered (nullptr, 0);
}

} // namespace xf


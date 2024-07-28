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

#ifndef XEFIS__CORE__COMPONENTS__SIMULATOR__CONSTRAINT_ITEM_H__INCLUDED
#define XEFIS__CORE__COMPONENTS__SIMULATOR__CONSTRAINT_ITEM_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/simulation/rigid_body/constraint.h>

// Qt:
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QTreeWidgetItem>

// Standard:
#include <cstddef>


namespace xf {

class ConstraintItem: public QTreeWidgetItem
{
  public:
	// Ctor
	explicit
	ConstraintItem (QTreeWidget& parent, rigid_body::Constraint& constraint):
		QTreeWidgetItem (&parent, QStringList (QString::fromStdString (constraint.label()))),
		_constraint (constraint)
	{ }

	// Ctor
	explicit
	ConstraintItem (QTreeWidgetItem& parent, rigid_body::Constraint& constraint):
		QTreeWidgetItem (&parent, QStringList (QString::fromStdString (constraint.label()))),
		_constraint (constraint)
	{ }

	[[nodiscard]]
	rigid_body::Constraint&
	constraint() noexcept
		{ return _constraint; }

	[[nodiscard]]
	rigid_body::Constraint const&
	constraint() const noexcept
		{ return _constraint; }

	void
	refresh()
		{ setText (0, QString::fromStdString (_constraint.label())); }

  private:
	rigid_body::Constraint& _constraint;
};

} // namespace xf

#endif


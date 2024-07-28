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

#ifndef XEFIS__CORE__COMPONENTS__SIMULATOR__BODY_ITEM_H__INCLUDED
#define XEFIS__CORE__COMPONENTS__SIMULATOR__BODY_ITEM_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/simulation/rigid_body/body.h>

// Qt:
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QTreeWidgetItem>

// Standard:
#include <cstddef>


namespace xf {

class BodyItem: public QTreeWidgetItem
{
  public:
	// Ctor
	explicit
	BodyItem (QTreeWidget& parent, rigid_body::Body& body):
		QTreeWidgetItem (&parent, QStringList (QString::fromStdString (body.label()))),
		_body (body)
	{ }

	// Ctor
	explicit
	BodyItem (QTreeWidgetItem& parent, rigid_body::Body& body):
		QTreeWidgetItem (&parent, QStringList (QString::fromStdString (body.label()))),
		_body (body)
	{ }

	[[nodiscard]]
	rigid_body::Body&
	body() noexcept
		{ return _body; }

	[[nodiscard]]
	rigid_body::Body const&
	body() const noexcept
		{ return _body; }

	void
	refresh()
		{ setText (0, QString::fromStdString (_body.label())); }

  private:
	rigid_body::Body& _body;
};

} // namespace xf

#endif


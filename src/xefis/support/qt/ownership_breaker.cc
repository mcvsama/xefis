/* vim:ts=4
 *
 * Copyleft 2008…2018  Michał Gawron
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
#include "ownership_breaker.h"

// Qt:
#include <QtWidgets/QLayout>

// Standard:
#include <cstddef>


namespace xf {

OwnershipBreaker::OwnershipBreaker (QWidget* child, QWidget* parent):
	QWidget (parent),
	_child (child)
{
	QHBoxLayout* layout = new QHBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (0);
	layout->addWidget (child, 0, Qt::AlignTop | Qt::AlignLeft);
	layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
}


OwnershipBreaker::~OwnershipBreaker()
{
	_child->hide();
	_child->setParent (nullptr);
}

} // namespace xf


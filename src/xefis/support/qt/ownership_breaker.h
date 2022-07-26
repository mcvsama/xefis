/* vim:ts=4
 *
 * Copyleft 2012…2018  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__QT__OWNERSHIP_BREAKER_H__INCLUDED
#define XEFIS__SUPPORT__QT__OWNERSHIP_BREAKER_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Qt:
#include <QtWidgets/QWidget>

// Standard:
#include <cstddef>


namespace xf {

/**
 * This widget ensures that its child widget is NOT deleted when this widget is deleted. Used to break Qt's parent-child relationship when it comes to pointer
 * ownership (since Qt doesn't have its own mechanism for this).
 *
 * Also - lays out the child widget.
 */
class OwnershipBreaker: public QWidget
{
  public:
	// Ctor
	explicit
	OwnershipBreaker (QWidget* child, QWidget* parent);

	// Dtor
	~OwnershipBreaker();

  private:
	QWidget* _child;
};

} // namespace xf

#endif


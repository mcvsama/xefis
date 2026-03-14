/* vim:ts=4
 *
 * Copyleft 2026  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__PROPERTIES__HAS_CONFIGURATOR_WIDGET_H__INCLUDED
#define XEFIS__SUPPORT__PROPERTIES__HAS_CONFIGURATOR_WIDGET_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>


class QWidget;

namespace xf {

/**
 * Defines method for accessing configuration widget if an object decides to implement one.
 * The returned widget is meant to be embedded into some host UI, which then owns it
 * through normal Qt parent-child lifetime rules.
 */
class HasConfiguratorWidget
{
  public:
	// Dtor
	virtual
	~HasConfiguratorWidget() = default;

	virtual QWidget*
	configurator_widget() = 0;
};

} // namespace xf

#endif

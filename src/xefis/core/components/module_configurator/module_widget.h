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

#ifndef XEFIS__CORE__COMPONENTS__MODULE_CONFIGURATOR__MODULE_WIDGET_H__INCLUDED
#define XEFIS__CORE__COMPONENTS__MODULE_CONFIGURATOR__MODULE_WIDGET_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtWidgets/QWidget>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QLabel>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/machine.h>
#include <xefis/core/components/property_tree/property_tree.h>

// Local:
#include "module_widget.h"


namespace xf {

/**
 * Configuration widget for module.
 * Contains generic config widgets, module's configurator widget,
 * and other stuff.
 */
class ModuleWidget: public QWidget
{
  public:
	// Ctor
	explicit
	ModuleWidget (BasicModule&, QWidget* parent);

	/**
	 * Return module.
	 */
	BasicModule&
	module() const noexcept;

  private:
	BasicModule&	_module;
	PropertyTree*	_inputs_property_tree;
	PropertyTree*	_outputs_property_tree;
};


inline BasicModule&
ModuleWidget::module() const noexcept
{
	return _module;
}

} // namespace xf

#endif


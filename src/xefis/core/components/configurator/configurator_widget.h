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

#ifndef XEFIS__CORE__COMPONENTS__CONFIGURATOR__CONFIGURATOR_WIDGET_H__INCLUDED
#define XEFIS__CORE__COMPONENTS__CONFIGURATOR__CONFIGURATOR_WIDGET_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/components/data_recorder/data_recorder.h>
#include <xefis/core/components/module_configurator/module_configurator.h>
#include <xefis/core/module.h>
#include <xefis/support/ui/widget.h>

// Qt:
#include <QtWidgets/QTabWidget>

// Standard:
#include <cstddef>


namespace xf {

class ConfiguratorWidget: public Widget
{
  public:
	// Ctor
	explicit
	ConfiguratorWidget (Machine& machine, QWidget* parent);

  private:
	Machine&			_machine;
	ModuleConfigurator*	_module_configurator	= nullptr;
	DataRecorder*		_data_recorder			= nullptr;
	QTabWidget*			_tabs					= nullptr;
};

} // namespace xf

#endif


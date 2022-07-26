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

#ifndef XEFIS__CORE__COMPONENTS__MODULE_CONFIGURATOR__MODULE_CONFIGURATOR_H__INCLUDED
#define XEFIS__CORE__COMPONENTS__MODULE_CONFIGURATOR__MODULE_CONFIGURATOR_H__INCLUDED

// Local:
#include "configurable_items_list.h"
#include "module_widget.h"
#include "processing_loop_widget.h"
#include "screen_widget.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/machine.h>

// Qt:
#include <QtWidgets/QWidget>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QLabel>

// Standard:
#include <cstddef>


namespace xf {

class ModuleConfigurator: public QWidget
{
  public:
	// Ctor
	explicit
	ModuleConfigurator (Machine&, QWidget* parent);

  private:
	void
	processing_loop_selected (ProcessingLoop&);

	void
	screen_selected (Screen&);

	void
	module_selected (Module&);

	void
	none_selected();

  private:
	Machine&														_machine;
	configurator::ConfigurableItemsList*							_configurable_items_list	{ nullptr };
	QStackedWidget*													_stack						{ nullptr };
	QLabel*															_no_module_selected			{ nullptr };
	std::map<ProcessingLoop*, configurator::ProcessingLoopWidget*>	_processing_loop_widgets;
	std::map<Screen*, configurator::ScreenWidget*>					_screen_widgets;
	std::map<Module*, configurator::ModuleWidget*>					_module_widgets;
};

} // namespace xf

#endif


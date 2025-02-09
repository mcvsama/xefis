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

// Local:
#include "module_configurator.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/qt/ownership_breaker.h>

// Qt:
#include <QBoxLayout>

// Standard:
#include <cstddef>


namespace xf {

using namespace configurator;


ModuleConfigurator::ModuleConfigurator (Machine& machine, QWidget* parent):
	QWidget (parent),
	_machine (machine)
{
	_no_module_selected = new QLabel ("No module selected", this);
	_no_module_selected->setAlignment (Qt::AlignCenter);

	_configurable_items_list = new ConfigurableItemsList (_machine, this);
	_configurable_items_list->setSizePolicy (QSizePolicy::Maximum, QSizePolicy::Minimum);
	QObject::connect (_configurable_items_list, &ConfigurableItemsList::processing_loop_selected, this, &ModuleConfigurator::processing_loop_selected);
	QObject::connect (_configurable_items_list, &ConfigurableItemsList::screen_selected, this, &ModuleConfigurator::screen_selected);
	QObject::connect (_configurable_items_list, &ConfigurableItemsList::module_selected, this, &ModuleConfigurator::module_selected);
	QObject::connect (_configurable_items_list, &ConfigurableItemsList::none_selected, this, &ModuleConfigurator::none_selected);

	_stack = new QStackedWidget (this);
	_stack->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	_stack->addWidget (_no_module_selected);

	QHBoxLayout* layout = new QHBoxLayout (this);
	layout->setMargin (0);
	layout->addWidget (_configurable_items_list);
	layout->addWidget (_stack);
}


void
ModuleConfigurator::processing_loop_selected (ProcessingLoop& processing_loop)
{
	auto plw = _processing_loop_widgets.find (&processing_loop);

	if (plw == _processing_loop_widgets.end())
		plw = _processing_loop_widgets.insert ({ &processing_loop, new ProcessingLoopWidget (processing_loop, this) }).first;

	if (_stack->indexOf (plw->second) == -1)
		_stack->addWidget (plw->second);

	_stack->setCurrentWidget (plw->second);
}


void
ModuleConfigurator::screen_selected (Screen& screen)
{
	auto sw = _screen_widgets.find (&screen);

	if (sw == _screen_widgets.end())
		sw = _screen_widgets.insert ({ &screen, new ScreenWidget (screen, this) }).first;

	if (_stack->indexOf (sw->second) == -1)
		_stack->addWidget (sw->second);

	_stack->setCurrentWidget (sw->second);
}


void
ModuleConfigurator::module_selected (Module& module)
{
	auto gmw = _module_widgets.find (&module);

	if (gmw == _module_widgets.end())
		gmw = _module_widgets.insert ({ &module, new ModuleWidget (module, this) }).first;

	if (_stack->indexOf (gmw->second) == -1)
		_stack->addWidget (gmw->second);

	_stack->setCurrentWidget (gmw->second);
}


void
ModuleConfigurator::none_selected()
{
	_stack->setCurrentWidget (_no_module_selected);
}

} // namespace xf


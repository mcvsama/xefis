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

// Standard:
#include <cstddef>

// Qt:
#include <QBoxLayout>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/qt/ownership_breaker.h>

// Local:
#include "module_configurator.h"


namespace xf {

ModuleConfigurator::ModuleConfigurator (Machine& machine, QWidget* parent):
	QWidget (parent),
	_machine (machine)
{
	_no_module_selected = new QLabel ("No module selected", this);
	_no_module_selected->setAlignment (Qt::AlignCenter);

	_modules_list = new ModulesList (_machine, this);
	_modules_list->setSizePolicy (QSizePolicy::Maximum, QSizePolicy::Minimum);
	QObject::connect (_modules_list, &ModulesList::module_selected, this, &ModuleConfigurator::module_selected);
	QObject::connect (_modules_list, &ModulesList::none_selected, this, &ModuleConfigurator::none_selected);

	_modules_stack = new QStackedWidget (this);
	_modules_stack->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	_modules_stack->addWidget (_no_module_selected);

	QHBoxLayout* layout = new QHBoxLayout (this);
	layout->setMargin (0);
	layout->addWidget (_modules_list);
	layout->addWidget (_modules_stack);
}


void
ModuleConfigurator::module_selected (BasicModule& module)
{
	auto gmw = _module_widgets.find (&module);

	if (gmw == _module_widgets.end())
	{
		auto new_gmw = std::make_shared<ModuleWidget> (module, this);
		gmw = _module_widgets.insert ({ &module, new_gmw }).first;
	}

	if (_modules_stack->indexOf (gmw->second.get()) == -1)
		_modules_stack->addWidget (gmw->second.get());

	_modules_stack->setCurrentWidget (gmw->second.get());
}


void
ModuleConfigurator::none_selected()
{
	_modules_stack->setCurrentWidget (_no_module_selected);
}


// TODO remove or add button for this function
void
ModuleConfigurator::reload_module_widget (ModuleWidget* module_widget)
{
	_modules_list->deselect();
	_module_widgets.erase (&module_widget->module());
}

} // namespace xf


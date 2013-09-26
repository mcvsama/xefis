/* vim:ts=4
 *
 * Copyleft 2012…2013  Michał Gawron
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
#include <QtWidgets/QLayout>
#include <QtWidgets/QGridLayout>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module_manager.h>

// Local:
#include "configurator_widget.h"


namespace Xefis {

ConfiguratorWidget::ConfiguratorWidget (ModuleManager* module_manager, QWidget* parent):
	QWidget (parent),
	_module_manager (module_manager)
{
	_no_config_placeholder = new QLabel ("This module doesn't have configuration UI", this);
	_no_config_placeholder->setAlignment (Qt::AlignCenter);

	_property_editor = new PropertyEditor (PropertyStorage::default_storage()->root(), this);

	_modules_list = new ModulesList (_module_manager, this);
	_modules_list->setSizePolicy (QSizePolicy::Maximum, QSizePolicy::Minimum);
	QObject::connect (_modules_list, SIGNAL (module_selected (Module::Pointer const&)), this, SLOT (module_selected (Module::Pointer const&)));

	_modules_stack = new QStackedWidget (this);
	_modules_stack->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	_modules_stack->addWidget (_no_config_placeholder);

	QWidget* _module_configurator = new QWidget (this);

	QHBoxLayout* module_configurator_layout = new QHBoxLayout (_module_configurator);
	module_configurator_layout->setMargin (WidgetMargin);
	module_configurator_layout->setSpacing (WidgetSpacing);
	module_configurator_layout->addWidget (_modules_list);
	module_configurator_layout->addWidget (_modules_stack);

	_tabs = new QTabWidget (this);
	_tabs->addTab (_property_editor, "Property database");
	_tabs->addTab (_module_configurator, "Module configuration");

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (WidgetSpacing);
	layout->addWidget (_tabs);
}


void
ConfiguratorWidget::module_selected (Module::Pointer const& module_pointer)
{
	Module* module = _module_manager->find (module_pointer);
	QWidget* configurator_widget = module->configurator_widget();

	if (configurator_widget)
	{
		if (_modules_stack->indexOf (configurator_widget) == -1)
		{
			decorate_widget (configurator_widget);
			_modules_stack->addWidget (_config_decorators[configurator_widget]);
		}
		_modules_stack->setCurrentWidget (_config_decorators[configurator_widget]);
	}
	else
		_modules_stack->setCurrentWidget (_no_config_placeholder);
}


void
ConfiguratorWidget::decorate_widget (QWidget* configurator_widget)
{
	QWidget* decorator = new QWidget (this);
	_config_decorators[configurator_widget] = decorator;

	QGridLayout* layout = new QGridLayout (decorator);
	layout->setMargin (0);
	layout->setSpacing (0);
	layout->addWidget (configurator_widget, 0, 0);
	layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding), 0, 1);
	layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding), 1, 0);
}

} // namespace Xefis


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
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QMessageBox>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/machine.h>
#include <xefis/core/v2/module.h>
#include <xefis/support/qt/ownership_breaker.h>

// Local:
#include "configurator_widget.h"


namespace xf {

ConfiguratorWidget::GeneralModuleWidget::GeneralModuleWidget (v2::BasicModule& module, ConfiguratorWidget& configurator_widget, QWidget* parent):
	QWidget (parent),
	_module (module),
	_configurator_widget (configurator_widget)
{
	auto full_name_str = QString::fromStdString (xf::identifier (module));

	QLabel* name_label = new QLabel (full_name_str.toHtmlEscaped());
	name_label->setAlignment (Qt::AlignLeft);
	QFont font = name_label->font();
	font.setPointSize (2.0 * font.pointSize());
	name_label->setFont (font);

	QTabWidget* tabs = new QTabWidget (this);

	if (auto module_with_config_widget = dynamic_cast<v2::BasicModule::HasConfiguratorWidget*> (&module))
	{
		auto module_config_widget = module_with_config_widget->configurator_widget();
		tabs->addTab (new OwnershipBreaker (module_config_widget, this), "Module config");
	}

	tabs->addTab (new QWidget (this), "I/O");

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (WidgetSpacing);
	layout->addWidget (name_label);
	layout->addWidget (tabs);
}


ConfiguratorWidget::ConfiguratorWidget (v2::Machine& machine, QWidget* parent):
	QWidget (parent),
	_machine (machine)
{
	_no_module_selected = new QLabel ("No module selected", this);
	_no_module_selected->setAlignment (Qt::AlignCenter);

	_modules_list = new ModulesList (_machine, this);
	_modules_list->setSizePolicy (QSizePolicy::Maximum, QSizePolicy::Minimum);
	QObject::connect (_modules_list, &ModulesList::module_selected, this, &ConfiguratorWidget::module_selected);
	QObject::connect (_modules_list, &ModulesList::none_selected, this, &ConfiguratorWidget::none_selected);

	_modules_stack = new QStackedWidget (this);
	_modules_stack->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	_modules_stack->addWidget (_no_module_selected);

	QWidget* module_configurator = new QWidget (this);

	_data_recorder = new DataRecorder (this);

	QHBoxLayout* module_configurator_layout = new QHBoxLayout (module_configurator);
	module_configurator_layout->setMargin (WidgetMargin);
	module_configurator_layout->setSpacing (WidgetSpacing);
	module_configurator_layout->addWidget (_modules_list);
	module_configurator_layout->addWidget (_modules_stack);

	_tabs = new QTabWidget (this);
	_tabs->addTab (new QLabel ("To be implemented: property editor"), "&Property database");
	_tabs->addTab (module_configurator, "Module &configuration");
	_tabs->addTab (_data_recorder, "&Data recorder");

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (WidgetSpacing);
	layout->addWidget (_tabs);
}


void
ConfiguratorWidget::module_selected (v2::BasicModule& module)
{
	auto gmw = _general_module_widgets.find (&module);

	if (gmw == _general_module_widgets.end())
	{
		auto new_gmw = std::make_shared<GeneralModuleWidget> (module, *this, this);
		gmw = _general_module_widgets.insert ({ &module, new_gmw }).first;
	}

	if (_modules_stack->indexOf (gmw->second.get()) == -1)
		_modules_stack->addWidget (gmw->second.get());

	_modules_stack->setCurrentWidget (gmw->second.get());
}


void
ConfiguratorWidget::none_selected()
{
	_modules_stack->setCurrentWidget (_no_module_selected);
}


void
ConfiguratorWidget::reload_module_widget (GeneralModuleWidget* module_widget)
{
	_modules_list->deselect();
	_general_module_widgets.erase (&module_widget->module());
}

} // namespace xf


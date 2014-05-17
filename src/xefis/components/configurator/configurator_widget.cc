/* vim:ts=4
 *
 * Copyleft 2012…2014  Michał Gawron
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
#include <xefis/core/module_manager.h>

// Local:
#include "configurator_widget.h"


namespace Xefis {

ConfiguratorWidget::OwnershipBreakingDecorator::OwnershipBreakingDecorator (QWidget* child, QWidget* parent):
	QWidget (parent),
	_child (child)
{
	QHBoxLayout* layout = new QHBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (0);
	layout->addWidget (child, 0, Qt::AlignTop | Qt::AlignLeft);
	layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
}


ConfiguratorWidget::OwnershipBreakingDecorator::~OwnershipBreakingDecorator()
{
	_child->hide();
	_child->setParent (nullptr);
}


ConfiguratorWidget::GeneralModuleWidget::GeneralModuleWidget (Module* module, QWidget* parent):
	QWidget (parent)
{
	QPushButton* reload_button = new QPushButton ("Force module restart", this);
	QObject::connect (reload_button, &QPushButton::clicked, [module,this]() {
		QString instance_html = module->instance().empty()
			? "<i>default</i>"
			: "<b>" + QString::fromStdString (module->instance()).toHtmlEscaped() + "</b>";
		QString message = QString ("<p>Confirm module restart:</p>")
			+ "<table style='margin: 1em 0'>"
			+ "<tr><td>Module name: </td><td><b>" + QString::fromStdString (module->name()).toHtmlEscaped() + "</b></td></tr>"
			+ "<tr><td>Instance: </td><td>" + instance_html + "</td></tr>"
			+ "</table>";
		if (QMessageBox::question (this, "Module restart", message) == QMessageBox::Ok)
			module->module_manager()->post_module_reload_request (module);
	});

	QHBoxLayout* buttons_layout = new QHBoxLayout();
	buttons_layout->addWidget (reload_button);
	buttons_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));

	QTabWidget* tabs = new QTabWidget (this);
	QWidget* module_config_widget = module->configurator_widget();
	if (module_config_widget)
		tabs->addTab (new OwnershipBreakingDecorator (module_config_widget, this), "Module config");
	tabs->addTab (new QWidget (this), "I/O");

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (WidgetSpacing);
	layout->addLayout (buttons_layout);
	layout->addWidget (tabs);
}


ConfiguratorWidget::ConfiguratorWidget (ModuleManager* module_manager, QWidget* parent):
	QWidget (parent),
	_module_manager (module_manager)
{
	_no_module_selected = new QLabel ("No module selected", this);
	_no_module_selected->setAlignment (Qt::AlignCenter);

	_property_editor = new PropertyEditor (PropertyStorage::default_storage()->root(), this);

	_modules_list = new ModulesList (_module_manager, this);
	_modules_list->setSizePolicy (QSizePolicy::Maximum, QSizePolicy::Minimum);
	QObject::connect (_modules_list, SIGNAL (module_selected (Module::Pointer const&)), this, SLOT (module_selected (Module::Pointer const&)));

	_modules_stack = new QStackedWidget (this);
	_modules_stack->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	_modules_stack->addWidget (_no_module_selected);

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

	auto gmw = _general_module_widgets.find (module);
	if (gmw == _general_module_widgets.end())
	{
		auto new_gmw = std::make_unique<GeneralModuleWidget> (module, this);
		gmw = _general_module_widgets.insert ({ module, new_gmw.get() }).first;
		new_gmw.release();
	}

	if (_modules_stack->indexOf (gmw->second) == -1)
		_modules_stack->addWidget (gmw->second);

	_modules_stack->setCurrentWidget (gmw->second);
}

} // namespace Xefis


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
#include <xefis/core/v1/module_manager.h>

// Local:
#include "configurator_widget.h"


namespace xf {

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


ConfiguratorWidget::GeneralModuleWidget::GeneralModuleWidget (Xefis* xefis, Module* module, ConfiguratorWidget* configurator_widget, QWidget* parent):
	QWidget (parent),
	_xefis (xefis),
	_module (module),
	_configurator_widget (configurator_widget)
{
	_module_ptr = module->get_pointer();

	QPushButton* reload_button = new QPushButton ("Force module restart", this);
	QObject::connect (reload_button, &QPushButton::clicked, this, &GeneralModuleWidget::reload_module);

	QString full_name_str = QString::fromStdString (module->name());
	if (!module->instance().empty())
		full_name_str += " • " + QString::fromStdString (module->instance());
	QLabel* name_label = new QLabel (full_name_str.toHtmlEscaped());
	name_label->setAlignment (Qt::AlignLeft);
	QFont font = name_label->font();
	font.setPointSize (2.0 * font.pointSize());
	name_label->setFont (font);

	QTabWidget* tabs = new QTabWidget (this);
	QWidget* module_config_widget = module->configurator_widget();
	if (module_config_widget)
		tabs->addTab (new OwnershipBreakingDecorator (module_config_widget, this), "Module config");
	tabs->addTab (new QWidget (this), "I/O");

	QHBoxLayout* buttons_layout = new QHBoxLayout();
	buttons_layout->addWidget (name_label);
	buttons_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	buttons_layout->addWidget (reload_button);

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (WidgetSpacing);
	layout->addLayout (buttons_layout);
	layout->addWidget (tabs);
}


void
ConfiguratorWidget::GeneralModuleWidget::reload_module()
{
	auto module_manager = _xefis->module_manager();
	if (module_manager)
	{
		QString instance_html = _module_ptr.instance().empty()
			? "<i>default</i>"
			: "<b>" + QString::fromStdString (_module_ptr.instance()).toHtmlEscaped() + "</b>";
		QString message = QString ("<p>Confirm module restart:</p>")
			+ "<table style='margin: 1em 0'>"
			+ "<tr><td>Module name: </td><td><b>" + QString::fromStdString (_module_ptr.name()).toHtmlEscaped() + "</b></td></tr>"
			+ "<tr><td>Instance: </td><td>" + instance_html + "</td></tr>"
			+ "</table>";

		if (QMessageBox::question (this, "Module restart", message) == QMessageBox::Yes)
		{
			module_manager->post_module_reload_request (_module_ptr);
			// Must be called last, since @this will be deleted by this call:
			_configurator_widget->reload_module_widget (this);
			// Note! @this is dangling now.
		}
	}
}


ConfiguratorWidget::ConfiguratorWidget (Xefis* xefis, QWidget* parent):
	QWidget (parent),
	_xefis (xefis)
{
	_no_module_selected = new QLabel ("No module selected", this);
	_no_module_selected->setAlignment (Qt::AlignCenter);

	_property_editor = new PropertyEditor (PropertyStorage::default_storage()->root(), this);

	_modules_list = new ModulesList (_xefis->module_manager(), this);
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
	_tabs->addTab (_property_editor, "&Property database");
	_tabs->addTab (module_configurator, "Module &configuration");
	_tabs->addTab (_data_recorder, "&Data recorder");

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (WidgetSpacing);
	layout->addWidget (_tabs);
}


void
ConfiguratorWidget::module_selected (Module::Pointer const& module_pointer)
{
	Module* module = _xefis->module_manager()->find (module_pointer);
	if (module)
	{
		auto gmw = _general_module_widgets.find (module);
		if (gmw == _general_module_widgets.end())
		{
			auto new_gmw = std::make_shared<GeneralModuleWidget> (_xefis, module, this, this);
			gmw = _general_module_widgets.insert ({ module, new_gmw }).first;
		}

		if (_modules_stack->indexOf (gmw->second.get()) == -1)
			_modules_stack->addWidget (gmw->second.get());

		_modules_stack->setCurrentWidget (gmw->second.get());
	}
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
	if (_general_module_widgets.find (module_widget->module()) != _general_module_widgets.end())
		_general_module_widgets.erase (module_widget->module());
}

} // namespace xf


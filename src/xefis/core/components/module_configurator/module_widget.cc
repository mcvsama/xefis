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
#include <QTabWidget>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/qt/ownership_breaker.h>

// Local:
#include "module_widget.h"


namespace xf {

ModuleWidget::ModuleWidget (BasicModule& module, QWidget* parent):
	QWidget (parent),
	_module (module)
{
	auto full_name_str = QString::fromStdString (identifier (module));

	QLabel* name_label = new QLabel (full_name_str.toHtmlEscaped());
	name_label->setAlignment (Qt::AlignLeft);
	QFont font = name_label->font();
	font.setPointSize (2.0 * font.pointSize());
	name_label->setFont (font);

	auto tabs = new QTabWidget (this);

	if (auto* io_base = _module.io_base())
	{
		_inputs_property_tree = new PropertyTree (this);
		_inputs_property_tree->populate (ModuleIO::ProcessingLoopAPI (*io_base).input_properties());

		_outputs_property_tree = new PropertyTree (this);
		_outputs_property_tree->populate (ModuleIO::ProcessingLoopAPI (*io_base).output_properties());

		tabs->addTab (_inputs_property_tree, "Data inputs");
		tabs->addTab (_outputs_property_tree, "Data outputs");
		tabs->addTab (new QLabel ("TODO", this), "Settings"); // TODO
	}

	if (auto module_with_config_widget = dynamic_cast<BasicModule::HasConfiguratorWidget*> (&_module))
	{
		auto module_config_widget = module_with_config_widget->configurator_widget();
		tabs->addTab (new OwnershipBreaker (module_config_widget, this), "Module config");
	}

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (WidgetSpacing);
	layout->addWidget (name_label);
	layout->addWidget (tabs);
}

} // namespace xf


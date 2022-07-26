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
#include "configurator_widget.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/machine.h>
#include <xefis/core/module.h>
#include <xefis/support/ui/paint_helper.h>

// Qt:
#include <QBoxLayout>
#include <QShortcut>

// Standard:
#include <cstddef>


namespace xf {

ConfiguratorWidget::ConfiguratorWidget (Machine& machine, QWidget* parent):
	Widget (parent),
	_machine (machine)
{
	auto const ph = PaintHelper (*this, palette(), font());

	_module_configurator = new ModuleConfigurator (_machine, this);
	_data_recorder = new DataRecorder (this);

	_tabs = new QTabWidget (this);
	_tabs->addTab (_module_configurator, "Module &configuration");
	_tabs->addTab (_data_recorder, "&Data recorder");

	auto* layout = new QVBoxLayout (this);
	layout->setMargin (ph.em_pixels (0.15f));
	layout->addWidget (_tabs);

	auto* esc = new QShortcut (this);
	esc->setKey (Qt::Key_Escape);
	QObject::connect (esc, &QShortcut::activated, this, &Screen::close);
}

} // namespace xf


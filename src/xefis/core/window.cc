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
#include <QtWidgets/QShortcut>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/config_reader.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/numeric.h>
#include <xefis/components/configurator/configurator_widget.h>

// Local:
#include "window.h"


namespace Xefis {

Window::Window (Application* application, ConfigReader* config_reader, QDomElement const& element):
	_application (application),
	_config_reader (config_reader)
{
	setWindowTitle ("XEFIS");
	resize (limit (element.attribute ("width").toInt(), 40, 10000),
			limit (element.attribute ("height").toInt(), 30, 10000));
	setMouseTracking (true);
	setAttribute (Qt::WA_TransparentForMouseEvents);

	if (element.attribute ("full-screen") == "true")
		setWindowState (windowState() | Qt::WindowFullScreen);

	_stack = new QStackedWidget (this);

	_instruments_panel = new QWidget (_stack);
	_instruments_panel->setBackgroundRole (QPalette::Shadow);
	_instruments_panel->setAutoFillBackground (true);
	// Black background:
	QPalette p = palette();
	p.setColor (QPalette::Shadow, Qt::black);
	p.setColor (QPalette::Dark, Qt::gray);
	_instruments_panel->setPalette (p);

	_configurator_panel = new QWidget (this);

	QLayout* configurator_layout = new QVBoxLayout (_configurator_panel);
	configurator_layout->setMargin (WidgetMargin);
	configurator_layout->setSpacing (0);

	QLayout* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (0);
	layout->addWidget (_stack);

	_stack->addWidget (_instruments_panel);
	_stack->addWidget (_configurator_panel);
	_stack->setCurrentWidget (_instruments_panel);

	for (QDomElement& e: element)
	{
		if (e == "layout")
			process_layout_element (e, nullptr, _instruments_panel);
		else
			throw Exception (QString ("unsupported child of <window>: <%1>").arg (e.tagName()).toStdString());
	}

	new QShortcut (Qt::Key_Escape, this, SLOT (show_configurator()));
}


void
Window::process_layout_element (QDomElement const& layout_element, QBoxLayout* layout, QWidget* instruments_panel, int stretch)
{
	QBoxLayout* new_layout = nullptr;

	if (layout_element.attribute ("type") == "horizontal")
		new_layout = new QHBoxLayout();
	else if (layout_element.attribute ("type") == "vertical")
		new_layout = new QVBoxLayout();
	else
		throw Exception ("layout type must be 'vertical' or 'horizontal'");

	new_layout->setSpacing (0);
	new_layout->setMargin (0);

	// If adding layout to instruments_panel:
	if (!layout)
	{
		if (instruments_panel->layout())
			throw Exception ("an window can only have one layout");

		instruments_panel->setLayout (new_layout);
	}
	// Adding layout to parent layout:
	else
	{
		layout->addLayout (new_layout);
		layout->setStretchFactor (new_layout, stretch);
	}

	for (QDomElement& e: layout_element)
	{
		if (e == "item")
			process_item_element (e, new_layout, instruments_panel);
		else if (e == "separator")
		{
			QWidget* separator = new QWidget (instruments_panel);
			separator->setMinimumSize (2, 2);
			separator->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
			separator->setBackgroundRole (QPalette::Dark);
			separator->setAutoFillBackground (true);
			separator->setCursor (QCursor (Qt::CrossCursor));
			new_layout->addWidget (separator);
		}
		else
			throw Exception (QString ("unsupported child of <layout>: <%1>").arg (e.tagName()).toStdString());
	}
}


void
Window::process_item_element (QDomElement const& item_element, QBoxLayout* layout, QWidget* instruments_panel)
{
	bool has_child = false;
	int stretch = limit (item_element.attribute ("stretch-factor").toInt(), 1, std::numeric_limits<int>::max());

	for (QDomElement& e: item_element)
	{
		if (has_child)
			throw Exception ("only one child element per <item> allowed");

		has_child = true;

		if (e == "layout")
			process_layout_element (e, layout, instruments_panel, stretch);
		else if (e == "module")
			_config_reader->process_module_element (e, layout, instruments_panel, stretch);
		else
			throw Exception (QString ("unsupported child of <item>: <%1>").arg (e.tagName()).toStdString());
	}

	if (!has_child)
	{
		layout->addStretch (stretch);
		QWidget* p = layout->parentWidget();
		if (p)
			p->setCursor (QCursor (Qt::CrossCursor));
	}
}


void
Window::show_configurator()
{
	if (_stack->currentWidget() == _instruments_panel)
	{
		ConfiguratorWidget* configurator_widget = _application->configurator_widget();
		if (!configurator_widget)
			return;
		if (configurator_widget->owning_window())
			configurator_widget->owning_window()->configurator_taken();
		_configurator_panel->layout()->addWidget (configurator_widget);
		_stack->setCurrentWidget (_configurator_panel);
		configurator_widget->set_owning_window (this);
	}
	else
		_stack->setCurrentWidget (_instruments_panel);
}


void
Window::configurator_taken()
{
	_stack->setCurrentWidget (_instruments_panel);
}

} // namespace Xefis


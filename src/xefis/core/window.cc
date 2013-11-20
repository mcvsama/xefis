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
#include <QtWidgets/QStackedLayout>

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
		{
			QLayout* layout = process_layout_element (e, _instruments_panel);
			if (_instruments_panel->layout())
				throw Exception ("a window can only have one layout");
			_instruments_panel->setLayout (layout);
		}
		else
			throw Exception (QString ("unsupported child of <window>: <%1>").arg (e.tagName()).toStdString());
	}

	new QShortcut (Qt::Key_Escape, this, SLOT (show_configurator()));
}


void
Window::data_updated (Time const&)
{
	for (auto stack: _stacks)
	{
		if (stack->property.fresh() && stack->property.valid())
			stack->layout->setCurrentIndex (*stack->property);
	}
}


float
Window::pen_scale() const
{
	return _application->config_reader()->pen_scale();
}


float
Window::font_scale() const
{
	return _application->config_reader()->font_scale();
}


QLayout*
Window::process_layout_element (QDomElement const& layout_element, QWidget* instruments_panel)
{
	QLayout* new_layout = nullptr;
	Shared<Stack> stack;

	QString type = layout_element.attribute ("type");

	if (type == "horizontal")
		new_layout = new QHBoxLayout();
	else if (type == "vertical")
		new_layout = new QVBoxLayout();
	else if (type == "stack")
	{
		if (!layout_element.hasAttribute ("path"))
			throw Exception ("missing @path attribute on <layout type='stack'>");

		stack = std::make_shared<Stack>();
		stack->property.set_path (layout_element.attribute ("path").toStdString());
		stack->property.set_default (0);
		new_layout = stack->layout = new QStackedLayout();
		_stacks.insert (stack);
	}
	else
		throw Exception ("layout type must be 'vertical', 'horizontal' or 'stack'");

	new_layout->setSpacing (0);
	new_layout->setMargin (0);

	for (QDomElement& e: layout_element)
	{
		if (e == "item")
			process_item_element (e, new_layout, instruments_panel, stack);
		else if (e == "separator")
		{
			if (type == "stack")
				throw Exception ("<separator> not allowed in stack-type layout");

			QWidget* separator = new QWidget (instruments_panel);
			separator->setMinimumSize (2, 2);
			separator->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
			separator->setBackgroundRole (QPalette::Dark);
			separator->setAutoFillBackground (true);
			separator->setCursor (QCursor (Qt::CrossCursor));
			new_layout->addWidget (separator);
		}
		else
			throw Exception (QString ("unsupported child of <layout>: <%1>").arg (e.tagName()));
	}

	return new_layout;
}


void
Window::process_item_element (QDomElement const& item_element, QLayout* layout, QWidget* instruments_panel, Shared<Stack> stack)
{
	QBoxLayout* box_layout = dynamic_cast<QBoxLayout*> (layout);
	QStackedLayout* stacked_layout = dynamic_cast<QStackedLayout*> (layout);

	assert (stacked_layout && stack);

	if (stacked_layout && item_element.hasAttribute ("stretch-factor"))
		throw Exception ("attribute @stretch-factor not allowed on <item> of stack-type layout");

	if (box_layout && item_element.hasAttribute ("id"))
		throw Exception ("attribute @id not allowed on <item> of non-stack-type layout");

	bool has_child = false;
	int stretch = limit (item_element.attribute ("stretch-factor").toInt(), 1, std::numeric_limits<int>::max());

	// <item>'s children:
	for (QDomElement& e: item_element)
	{
		if (has_child)
			throw Exception ("only one child element per <item> allowed");

		has_child = true;

		if (e == "layout")
		{
			if (box_layout)
			{
				QLayout* sub_layout = process_layout_element (e, instruments_panel);
				box_layout->addLayout (sub_layout);
				box_layout->setStretchFactor (sub_layout, stretch);
			}
			else if (stacked_layout)
			{
				QLayout* sub_layout = process_layout_element (e, instruments_panel);
				QWidget* proxy_widget = new QWidget (instruments_panel);
				proxy_widget->setLayout (sub_layout);
				stacked_layout->addWidget (proxy_widget);
				stacked_layout->setCurrentWidget (proxy_widget);
			}
		}
		else if (e == "module")
		{
			Module* module = _config_reader->process_module_element (e, instruments_panel);
			if (module)
			{
				QWidget* module_widget = dynamic_cast<QWidget*> (module);

				if (module_widget)
				{
					if (box_layout)
					{
						box_layout->addWidget (module_widget);
						box_layout->setStretchFactor (module_widget, stretch);
					}
					else if (stacked_layout)
					{
						stacked_layout->addWidget (module_widget);
						stacked_layout->setCurrentWidget (module_widget);
					}
				}
			}
		}
		else
			throw Exception (QString ("unsupported child of <item>: <%1>").arg (e.tagName()).toStdString());
	}

	if (!has_child)
	{
		if (box_layout)
		{
			box_layout->addStretch (stretch);
			QWidget* p = box_layout->parentWidget();
			if (p)
				p->setCursor (QCursor (Qt::CrossCursor));
		}
		else if (stacked_layout)
		{
			// Empty widget:
			QWidget* empty_widget = new QWidget (instruments_panel);
			empty_widget->setCursor (QCursor (Qt::CrossCursor));
			stacked_layout->addWidget (empty_widget);
			stacked_layout->setCurrentWidget (empty_widget);
		}
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


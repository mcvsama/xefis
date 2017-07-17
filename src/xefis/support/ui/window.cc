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
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QShortcut>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/module.h>

// Local:
#include "window.h"


namespace v2 {

Window::Window()
{
	try {
		setWindowTitle ("XEFIS");
		setMouseTracking (true);
		setAttribute (Qt::WA_TransparentForMouseEvents);

		_stack = new QStackedWidget (this);

		_instruments_panel = new QWidget (_stack);
		_instruments_panel->setBackgroundRole (QPalette::Shadow);
		_instruments_panel->setAutoFillBackground (true);
		// Black background:
		QPalette p = palette();
		p.setColor (QPalette::Shadow, Qt::black);
		p.setColor (QPalette::Dark, Qt::gray);
		_instruments_panel->setPalette (p);

		_configurator_panel = new QWidget (_stack);

		auto* configurator_layout = new QVBoxLayout (_configurator_panel);
		configurator_layout->setMargin (WidgetMargin);
		configurator_layout->setSpacing (0);

		auto* layout = new QVBoxLayout (this);
		layout->setMargin (0);
		layout->setSpacing (0);
		layout->addWidget (_stack);

		_stack->addWidget (_instruments_panel);
		_stack->addWidget (_configurator_panel);
		_stack->setCurrentWidget (_instruments_panel);

		new QShortcut (Qt::Key_Escape, this, SLOT (show_configurator()));
	}
	catch (...)
	{
		unparent_modules();
		throw;
	}
}


Window::~Window()
{
	unparent_modules();
}


void
Window::show_configurator()
{
	// TODO
}


void
Window::unparent_modules()
{
	// Since children of type Module are managed by the ModuleManager, we must not let parent widget delete them.
	// Find them and reparent to 0.
	for (auto child: findChildren<QWidget*>())
		if (dynamic_cast<Module*> (child))
			child->setParent (nullptr);
}


void
Window::set_fullscreen()
{
	setWindowState (windowState() | Qt::WindowFullScreen);
}

} // namespace v2


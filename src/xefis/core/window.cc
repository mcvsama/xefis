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

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/config_reader.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/numeric.h>

// Local:
#include "window.h"


namespace Xefis {

Window::Window (ConfigReader* config_reader, QDomElement const& element):
	_config_reader (config_reader)
{
	setBackgroundRole (QPalette::Shadow);
	setAutoFillBackground (true);
	setWindowTitle ("XEFIS");
	resize (limit (element.attribute ("width").toInt(), 40, 10000),
			limit (element.attribute ("height").toInt(), 30, 10000));
	setMouseTracking (true);
	setAttribute (Qt::WA_TransparentForMouseEvents);

	if (element.attribute ("full-screen") == "true")
		setWindowState (windowState() | Qt::WindowFullScreen);

	// Black background:
	QPalette p = palette();
	p.setColor (QPalette::Shadow, Qt::black);
	setPalette (p);

	for (QDomElement& e: element)
	{
		if (e == "layout")
			process_layout_element (e, nullptr, this);
		else
			throw Exception (QString ("unsupported child of <window>: <%1>").arg (e.tagName()).toStdString());
	}
}


void
Window::process_layout_element (QDomElement const& layout_element, QBoxLayout* layout, QWidget* window, int stretch)
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

	// If adding layout to window:
	if (!layout)
	{
		if (window->layout())
			throw Exception ("a window can only have one layout");

		window->setLayout (new_layout);
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
			process_item_element (e, new_layout, window);
		else if (e == "separator")
		{
			QWidget* separator = new QWidget (window);
			separator->setMinimumSize (2, 2);
			separator->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
			separator->setBackgroundRole (QPalette::Dark);
			separator->setAutoFillBackground (true);
			separator->setCursor (QCursor (QPixmap (XEFIS_SHARED_DIRECTORY "/images/cursors/crosshair.png")));
			new_layout->addWidget (separator);
		}
		else
			throw Exception (QString ("unsupported child of <layout>: <%1>").arg (e.tagName()).toStdString());
	}
}


void
Window::process_item_element (QDomElement const& item_element, QBoxLayout* layout, QWidget* window)
{
	bool has_child = false;
	int stretch = limit (item_element.attribute ("stretch-factor").toInt(), 1, std::numeric_limits<int>::max());

	for (QDomElement& e: item_element)
	{
		if (has_child)
			throw Exception ("only one child element per <item> allowed");

		has_child = true;

		if (e == "layout")
			process_layout_element (e, layout, window, stretch);
		else if (e == "module")
			_config_reader->process_module_element (e, layout, window, stretch);
		else
			throw Exception (QString ("unsupported child of <item>: <%1>").arg (e.tagName()).toStdString());
	}

	if (!has_child)
	{
		layout->addStretch (stretch);
		QWidget* p = layout->parentWidget();
		if (p)
			p->setCursor (QCursor (QPixmap (XEFIS_SHARED_DIRECTORY "/images/cursors/crosshair.png")));
	}
}

} // namespace Xefis


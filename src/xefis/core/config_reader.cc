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
#include <memory>
#include <limits>

// System:
#include <unistd.h>

// Qt:
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QTextStream>
#include <QtWidgets/QWidget>
#include <QtWidgets/QLayout>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/application/application.h>
#include <xefis/core/module_manager.h>
#include <xefis/core/module.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/numeric.h>

// Local:
#include "config_reader.h"


namespace Xefis {

ConfigReader::ConfigReader (Application* application, ModuleManager* module_manager):
	_application (application),
	_module_manager (module_manager)
{ }


void
ConfigReader::load (QString const& path)
{
	_config_document = parse_file (path);
	process();
}


QDomDocument
ConfigReader::parse_file (QString const& path)
{
	QFile file (path);
	QDomDocument doc;

	if (!file.exists())
		throw ConfigException (("file not found: " + path).toStdString());

	if (!file.open (QFile::ReadOnly))
		throw ConfigException (("file access error: " + path).toStdString());

	if (!doc.setContent (&file, true))
		throw ConfigException (("config parse error: " + path).toStdString());

	return doc;
}


void
ConfigReader::process()
{
	QDomElement root = _config_document.documentElement();

	process_includes (root);

	if (root != "xefis-config")
		throw ConfigException (("config process error: unsupported root tag: " + root.tagName()).toStdString());

	for (QDomElement& e: root)
	{
		if (e == "windows")
			process_windows_element (e);
		else if (e == "modules")
			process_modules_element (e);
		else if (e == "properties")
			process_properties_element (e);
		else
			throw ConfigException (QString ("unsupported child of <xefis-config>: <%1>").arg (e.tagName()).toStdString());
	}
}


void
ConfigReader::process_includes (QDomElement parent)
{
	std::list<QDomElement> to_remove;

	for (QDomElement& e: parent)
	{
		if (e == "include")
		{
			to_remove.push_back (e);

			QString filename = e.attribute ("name");
			int last_slash = filename.lastIndexOf ('/');
			QString dirname = filename.left (last_slash + 1);
			QString basename = filename.mid (last_slash + 1);

			QDir cwd;
			::chdir (dirname.toStdString().c_str());

			QDomDocument sub_doc = parse_file (basename);
			process_includes (sub_doc.documentElement());

			for (QDomElement& x: sub_doc.documentElement())
			{
				QDomNode node = e.ownerDocument().importNode (x, true);
				parent.insertBefore (node, e);
			}

			::chdir (cwd.absolutePath().toStdString().c_str());
		}
	}

	for (QDomElement& e: to_remove)
		parent.removeChild (e);
}


void
ConfigReader::process_properties_element (QDomElement const& properties_element)
{
	for (QDomElement& e: properties_element)
	{
		if (e == "property")
			process_property_element (e);
		else
			throw ConfigException (QString ("unsupported child of <properties>: <%1>").arg (e.tagName()).toStdString());
	}
}


void
ConfigReader::process_windows_element (QDomElement const& windows_element)
{
	for (QDomElement& e: windows_element)
	{
		if (e == "window")
			process_window_element (e);
		else
			throw ConfigException (QString ("unsupported child of <windows>: <%1>").arg (e.tagName()).toStdString());
	}
}


void
ConfigReader::process_modules_element (QDomElement const& modules_element)
{
	for (QDomElement& e: modules_element)
	{
		if (e == "module")
			process_module_element (e);
		else
			throw ConfigException (QString ("unsupported child of <modules>: <%1>").arg (e.tagName()).toStdString());
	}
}


void
ConfigReader::process_property_element (QDomElement const& /*property_element*/)
{
	// TODO
}


void
ConfigReader::process_window_element (QDomElement const& window_element)
{
	std::unique_ptr<QWidget> window (new QWidget (nullptr));
	window->setBackgroundRole (QPalette::Shadow);
	window->setAutoFillBackground (true);
	window->setWindowTitle ("XEFIS");
	window->resize (bound (window_element.attribute ("width").toInt(), 40, 10000),
					bound (window_element.attribute ("height").toInt(), 30, 10000));
	if (window_element.attribute ("full-screen") == "true")
		window->setWindowState (window->windowState() | Qt::WindowFullScreen);

	// Black background:
	QPalette palette = window->palette();
	palette.setColor (QPalette::Shadow, Qt::black);
	window->setPalette (palette);

	for (QDomElement& e: window_element)
	{
		if (e == "layout")
			process_layout_element (e, nullptr, window.get());
		else
			throw ConfigException (QString ("unsupported child of <window>: <%1>").arg (e.tagName()).toStdString());
	}

	window->show();
	window.release();
}


void
ConfigReader::process_layout_element (QDomElement const& layout_element, QBoxLayout* layout, QWidget* window, int stretch)
{
	QBoxLayout* new_layout = nullptr;

	if (layout_element.attribute ("type") == "horizontal")
		new_layout = new QHBoxLayout();
	else if (layout_element.attribute ("type") == "vertical")
		new_layout = new QVBoxLayout();
	else
		throw ConfigException ("layout type must be 'vertical' or 'horizontal'");

	new_layout->setSpacing (0);
	new_layout->setMargin (0);

	// If adding layout to window:
	if (!layout)
	{
		if (window->layout())
			throw ConfigException ("a window can only have one layout");

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
			new_layout->addWidget (separator);
		}
		else
			throw ConfigException (QString ("unsupported child of <layout>: <%1>").arg (e.tagName()).toStdString());
	}
}


void
ConfigReader::process_item_element (QDomElement const& item_element, QBoxLayout* layout, QWidget* window)
{
	bool has_child = false;
	int stretch = bound (item_element.attribute ("stretch-factor").toInt(), 1, std::numeric_limits<int>::max());

	for (QDomElement& e: item_element)
	{
		if (has_child)
			throw ConfigException ("only one child element per <item> allowed");

		has_child = true;

		if (e == "layout")
			process_layout_element (e, layout, window, stretch);
		else if (e == "module")
			process_module_element (e, layout, window, stretch);
		else
			throw ConfigException (QString ("unsupported child of <item>: <%1>").arg (e.tagName()).toStdString());
	}

	if (!has_child)
		layout->addStretch (stretch);
}


void
ConfigReader::process_module_element (QDomElement const& module_element, QBoxLayout* layout, QWidget* window, int stretch)
{
	Module* module = _module_manager->load_module (module_element.attribute ("name"), module_element, window);

	if (layout)
	{
		QWidget* widget = dynamic_cast<QWidget*> (module);
		if (widget)
		{
			layout->addWidget (widget);
			layout->setStretchFactor (widget, stretch);
		}
	}
}

} // namespace Xefis


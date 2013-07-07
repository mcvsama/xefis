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
#include <QtCore/QTextStream>
#include <QtWidgets/QWidget>
#include <QtWidgets/QLayout>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/application/application.h>
#include <xefis/core/module_manager.h>
#include <xefis/core/module.h>
#include <xefis/core/window.h>
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
	int last_slash = path.lastIndexOf ('/');
	QString dirname = path.left (last_slash + 1);
	QString basename = path.mid (last_slash + 1);

	QDir cwd;
	_current_dir = cwd.absolutePath() + '/' + dirname;

	_config_document = parse_file (basename);
	process();

	_current_dir = cwd;
}


QDomDocument
ConfigReader::parse_file (QString const& path)
{
	QFile file (_current_dir.absolutePath() + '/' + path);
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
		if (e == "system")
			process_system_element (e);
		else if (e == "windows")
			process_windows_element (e);
		else if (e == "modules")
			process_modules_element (e);
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

			QDir cwd = _current_dir;
			_current_dir = _current_dir.absolutePath() + '/' + dirname;

			QDomDocument sub_doc = parse_file (basename);
			process_includes (sub_doc.documentElement());

			for (QDomElement& x: sub_doc.documentElement())
			{
				QDomNode node = e.ownerDocument().importNode (x, true);
				parent.insertBefore (node, e);
			}

			_current_dir = cwd;
		}
		else
			process_includes (e);
	}

	for (QDomElement& e: to_remove)
		parent.removeChild (e);
}


void
ConfigReader::process_system_element (QDomElement const& system_element)
{
	for (QDomElement& e: system_element)
	{
		if (e == "disable-navaids")
			_load_navaids = false;
		else
			throw ConfigException (QString ("unsupported child of <system>: <%1>").arg (e.tagName()).toStdString());
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
ConfigReader::process_window_element (QDomElement const& window_element)
{
	if (window_element.attribute ("disabled") == "true")
		return;

	// Auto delete if Window throws an exception:
	std::unique_ptr<Window> window (new Window (_application, this, window_element));
	window->show();
	window.release();
	_has_windows = true;
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
ConfigReader::process_module_element (QDomElement const& module_element, QBoxLayout* layout, QWidget* window, int stretch)
{
	if (module_element.attribute ("disabled") == "true")
		return;

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


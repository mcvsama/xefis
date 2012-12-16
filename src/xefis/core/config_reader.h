/* vim:ts=4
 *
 * Copyleft 2008…2012  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__COMPONENTS__CONFIG_READER_H__INCLUDED
#define XEFIS__COMPONENTS__CONFIG_READER_H__INCLUDED

// Standard:
#include <cstddef>
#include <stdexcept>

// Qt:
#include <QtXml/QDomElement>
#include <QtXml/QDomDocument>
#include <QtGui/QLayout>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/config/exception.h>


namespace Xefis {

class ModuleManager;

class ConfigReader
{
  public:
	ConfigReader (ModuleManager*);

	/**
	 * Read config, create windows and loads modules.
	 */
	void
	load (QString const& path);

  private:
	QDomDocument
	parse_file (QString const& path);

	void
	process();

	void
	process_includes (QDomElement parent);

	void
	process_properties_element (QDomElement const& properties_element);

	void
	process_property_element (QDomElement const& property_element);

	void
	process_windows_element (QDomElement const& windows_element);

	void
	process_modules_element (QDomElement const& modules_element);

	void
	process_window_element (QDomElement const& window_element);

	void
	process_layout_element (QDomElement const& layout_element, QBoxLayout* layout, QWidget* window, int stretch = 0);

	void
	process_item_element (QDomElement const& item_element, QBoxLayout* layout, QWidget* window);

	void
	process_module_element (QDomElement const& module_element, QBoxLayout* layout = nullptr, QWidget* window = nullptr, int stretch = 0);

  private:
	QDomDocument	_config_document;
	ModuleManager*	_module_manager		= nullptr;
};


class ConfigException: public Exception
{
  public:
	ConfigException (std::string const& message):
		Exception (message)
	{ }
};

} // namespace Xefis

#endif


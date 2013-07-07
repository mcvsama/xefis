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

#ifndef XEFIS__CORE__CONFIG_READER_H__INCLUDED
#define XEFIS__CORE__CONFIG_READER_H__INCLUDED

// Standard:
#include <cstddef>
#include <stdexcept>

// Qt:
#include <QtCore/QDir>
#include <QtXml/QDomElement>
#include <QtXml/QDomDocument>
#include <QtWidgets/QLayout>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/config/exception.h>


namespace Xefis {

class Application;
class ModuleManager;

class ConfigReader
{
	friend class Window;

  public:
	ConfigReader (Application*, ModuleManager*);

	/**
	 * Read config, create windows and loads modules.
	 */
	void
	load (QString const& path);

	/**
	 * Determines if there are any windows (and instruments)
	 * configured.
	 */
	bool
	has_windows() const;

	/**
	 * Return true if navaids are supposed to be loaded.
	 */
	bool
	load_navaids() const;

  private:
	QDomDocument
	parse_file (QString const& path);

	void
	process();

	void
	process_includes (QDomElement parent);

	void
	process_system_element (QDomElement const& system_element);

	void
	process_windows_element (QDomElement const& windows_element);

	void
	process_window_element (QDomElement const& window_element);

	void
	process_modules_element (QDomElement const& modules_element);

	void
	process_module_element (QDomElement const& module_element, QBoxLayout* layout = nullptr, QWidget* window = nullptr, int stretch = 0);

  private:
	Application*	_application		= nullptr;
	ModuleManager*	_module_manager		= nullptr;
	QDomDocument	_config_document;
	QDir			_current_dir;
	bool			_has_windows		= false;
	bool			_load_navaids		= true;
};


class ConfigException: public Exception
{
  public:
	ConfigException (std::string const& message):
		Exception (message)
	{ }
};


inline bool
ConfigReader::has_windows() const
{
	return _has_windows;
}


inline bool
ConfigReader::load_navaids() const
{
	return _load_navaids;
}

} // namespace Xefis

#endif


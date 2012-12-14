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
	 * Reads config and loads modules.
	 */
	void
	read_config (QString const& path);

  protected:
	/**
	 * Process XML file and load modules.
	 */
	void
	process();

  private:
	QDomDocument	_config_document;
	ModuleManager*	_module_manager;
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


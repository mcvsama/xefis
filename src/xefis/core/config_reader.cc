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

// Standard:
#include <cstddef>

// Qt:
#include <QtCore/QFile>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module_manager.h>
#include <xefis/utility/qdom.h>

// Local:
#include "config_reader.h"


namespace Xefis {

ConfigReader::ConfigReader (ModuleManager* module_manager):
	_module_manager (module_manager)
{ }


void
ConfigReader::read_config (QString const& path)
{
	QFile file (path);

	if (!file.exists())
		throw ConfigException ("file not found");

	if (!file.open (QIODevice::ReadOnly))
		throw ConfigException ("file access error");

	if (!_config_document.setContent (&file, true))
		throw ConfigException ("config parse error");

	file.close();
	process();
}


void
ConfigReader::process()
{
	QDomElement root = _config_document.documentElement();

	if (root != "xefis-config")
		throw ConfigException (("config process error: unsupported root tag: " + root.tagName()).toStdString());

	for (QDomElement& e: root)
	{
		if (e == "")
		{
			// TODO
		}
	}
}

} // namespace Xefis


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
#include <map>
#include <set>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/qdom.h>

// Local:
#include "module.h"
#include "module_manager.h"


namespace Xefis {

Module::Module (ModuleManager* module_manager):
	_module_manager (module_manager)
{ }


void
Module::parse_properties (QDomElement& properties_element, PropertiesList list)
{
	std::map<QString, PropertyUnion*> map;
	std::set<QString> unconfigured_values;

	for (NameAndProperty& pair: list)
	{
		map[pair.name] = &pair.property;
		if (pair.required)
			unconfigured_values.insert (pair.name);
	}

	QString root = properties_element.attribute ("path");

	for (QDomElement& e: properties_element)
	{
		if (e == "property")
		{
			if (!e.hasAttribute ("name"))
				throw Exception ("missing attribute @name for property");

			QString name = e.attribute ("name");

			unconfigured_values.erase (name);

			auto it = map.find (name);
			// Found config for nonexistent property:
			if (it == map.end())
				continue;

			if (e.hasAttribute ("path"))
			{
				std::string path = (root + e.attribute ("path")).toStdString();
				switch (it->second->type())
				{
					case PropBoolean:
						it->second->access_bool() = Xefis::PropertyBoolean (path);
						break;
					case PropInteger:
						it->second->access_int() = Xefis::PropertyInteger (path);
						break;
					case PropFloat:
						it->second->access_float() = Xefis::PropertyFloat (path);
						break;
					case PropString:
						it->second->access_string() = Xefis::PropertyString (path);
						break;
					case PropDirectory:
						break;
				}
			}
			else
				throw Exception (QString ("missing parameter @path for property: %1").arg (name).toStdString());

			if (!e.hasAttribute ("default"))
				it->second->access().set_nil();
			else
			{
				QString value = e.attribute ("default");
				switch (it->second->type())
				{
					case PropBoolean:
						it->second->access_bool().write (value == "true");
						break;
					case PropInteger:
						it->second->access_int().write (value.toInt());
						break;
					case PropFloat:
						it->second->access_float().write (value.toFloat());
						break;
					case PropString:
						it->second->access_string().write (value.toStdString());
						break;
					case PropDirectory:
						break;
				}
			}
		}
	}

	if (!unconfigured_values.empty())
	{
		QStringList list;
		for (auto s: unconfigured_values)
			list << s;
		throw Exception (QString ("missing configuration for the following properties: %1").arg (list.join (", ")).toStdString());
	}
}


void
Module::data_updated()
{
	_module_manager->application()->data_update();
}

} // namespace Xefis


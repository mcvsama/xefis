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
Module::parse_properties (QDomElement const& properties_element, PropertiesList list)
{
	try {
		std::map<QString, BaseProperty*> map;
		std::set<QString> unconfigured_values;
		std::set<QString> known_values;

		for (NameAndProperty& pair: list)
		{
			map[pair.name] = &pair.property;
			if (pair.required)
				unconfigured_values.insert (pair.name);
			known_values.insert (pair.name);
		}

		QString root = properties_element.attribute ("path");
		QString root_2;

		auto parse_property = [&] (QDomElement& e) -> void
		{
			if (!e.hasAttribute ("name"))
				throw Exception ("missing attribute @name for property");

			QString name = e.attribute ("name");

			if (known_values.find (name) == known_values.end())
				throw Exception (QString ("configuration for unknown property: %1").arg (name).toStdString());

			unconfigured_values.erase (name);

			auto it = map.find (name);
			// Found config for nonexistent property:
			if (it == map.end())
				return;

			if (e.hasAttribute ("path"))
			{
				std::string path = (root + root_2 + e.attribute ("path")).toStdString();
				it->second->set_path (path);
			}
			else
				throw Exception (QString ("missing parameter @path for property: %1").arg (name).toStdString());

			if (!e.hasAttribute ("default"))
				it->second->ensure_existence();
			else
			{
				QString value = e.attribute ("default");
				it->second->parse (value.toStdString());
			}
		};

		for (QDomElement& d: properties_element)
		{
			if (d == "property")
				parse_property (d);
			else if (d == "directory")
			{
				root_2 = d.attribute ("path");
				for (QDomElement& e: d)
					if (e == "property")
						parse_property (e);
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
	catch (Exception& e)
	{
		throw Exception ("error when parsing <properties>", &e);
	}
}


void
Module::parse_i2c (QDomElement const& i2c_element, I2C::Bus& bus, I2C::Address& address)
{
	bool has_bus = false;
	bool has_address = false;

	for (QDomElement& e: i2c_element)
	{
		if (e == "bus")
		{
			bus.set_bus_number (e.text().trimmed().toUInt());
			has_bus = true;
		}
		else if (e == "address")
		{
			QString addr = e.text().trimmed();
			if (addr.startsWith ("0x"))
				address = I2C::Address (addr.mid (2).toUInt (nullptr, 16));
			else
				address = I2C::Address (addr.toUInt());
			has_address = true;
		}
	}

	if (!has_bus)
		throw Xefis::Exception ("configure I2C bus number <i2c>/<bus>");
	if (!has_address)
		throw Xefis::Exception ("configure I2C BMP085 address <i2c>/<address>");
}


void
Module::signal_data_updated()
{
	_module_manager->application()->data_updated();
}


NavaidStorage*
Module::navaid_storage() const
{
	return _module_manager->application()->navaid_storage();
}


WorkPerformer*
Module::work_performer() const
{
	return _module_manager->application()->work_performer();
}


void
Module::register_factory (std::string const& name, FactoryFunction factory_function)
{
	factories()[name] = factory_function;
}


Module::FactoryFunction
Module::find_factory (std::string const& name)
{
	FactoriesMap::iterator f = factories().find (name);
	if (f == factories().end())
		return FactoryFunction();
	return f->second;
}


inline Module::FactoriesMap&
Module::factories()
{
	static FactoriesMap factories;
	return factories;
}

} // namespace Xefis


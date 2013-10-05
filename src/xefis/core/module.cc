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

// Qt:
#include <QtXml/QDomElement>

// Lib:
#include <boost/format.hpp>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/qdom.h>

// Local:
#include "module.h"
#include "module_manager.h"


namespace Xefis {

Module::Module (ModuleManager* module_manager, QDomElement const& config):
	_module_manager (module_manager),
	_name (config.attribute ("name").toStdString()),
	_instance (config.attribute ("instance", "").toStdString())
{
	_logger.set_prefix ((boost::format ("[%-30s#%-20s]") % _name % _instance).str());
}


void
Module::parse_properties (QDomElement const& properties_element, PropertiesList list)
{
	try {
		std::map<QString, TypedProperty*> map;
		std::set<QString> unconfigured_values;
		std::set<QString> configured_values;
		std::set<QString> known_values;

		for (NameAndProperty& element: list)
		{
			if (map.insert ({ element.name, &element.property }).second == false)
				throw Exception ("duplicated entry name in property list");
			if (element.required)
				unconfigured_values.insert (element.name);
			known_values.insert (element.name);
		}

		QString root = properties_element.attribute ("path");
		QString root_2;

		auto parse_property = [&] (QDomElement& e) -> void
		{
			if (!e.hasAttribute ("name"))
				throw Exception ("missing attribute @name for property");

			QString name = e.attribute ("name");

			if (known_values.find (name) == known_values.end())
				throw Exception (QString ("configuration for unknown property: %1").arg (name));

			if (configured_values.find (name) != configured_values.end())
				throw Exception (QString ("duplicated entry <properties>: %1").arg (name));

			unconfigured_values.erase (name);
			configured_values.insert (name);

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
				throw Exception (QString ("missing parameter @path for property: %1").arg (name));

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
			throw Exception (QString ("missing configuration for the following properties: %1").arg (list.join (", ")));
		}
	}
	catch (Exception& e)
	{
		throw Exception ("error when parsing <properties>", &e);
	}
}


void
Module::parse_settings (QDomElement const& settings_element, SettingsList list)
{
	try {
		std::map<QString, NameAndSetting*> map;
		std::set<QString> unconfigured_values;
		std::set<QString> configured_values;
		std::set<QString> known_values;

		for (NameAndSetting& element: list)
		{
			if (map.insert ({ element.name, &element }).second == false)
				throw Exception ("duplicated entry name in settings list");
			if (element.required)
				unconfigured_values.insert (element.name);
			known_values.insert (element.name);
		}

		for (QDomElement& d: settings_element)
		{
			if (d == "setting")
			{
				if (!d.hasAttribute ("name"))
					throw Exception ("missing attribute @name for setting");

				QString name = d.attribute ("name");

				if (known_values.find (name) == known_values.end())
					throw Exception (QString ("configuration for unknown setting: %1").arg (name));

			if (configured_values.find (name) != configured_values.end())
				throw Exception (QString ("duplicated entry <properties>: %1").arg (name));

				if (!d.hasAttribute ("value"))
					throw Exception (QString ("missing attribute @value for setting: %1").arg (name));

				QString value = d.attribute ("value");

				unconfigured_values.erase (name);
				configured_values.insert (name);

				auto it = map.find (name);
				if (it == map.end())
					return;

				NameAndSetting* nas = it->second;
				if (nas->value_bool)
					*nas->value_bool = value == "true";
				else if (nas->value_int)
					*nas->value_int = boost::lexical_cast<int> (value.toStdString());
				else if (nas->value_int64)
					*nas->value_int64 = boost::lexical_cast<int64_t> (value.toStdString());
				else if (nas->value_float)
					*nas->value_float = boost::lexical_cast<float> (value.toStdString());
				else if (nas->value_double)
					*nas->value_double = boost::lexical_cast<double> (value.toStdString());
				else if (nas->value_string)
					*nas->value_string = value.toStdString();
				else if (nas->value_qstring)
					*nas->value_qstring = value;
				else if (nas->value_si_value)
					nas->value_si_value->parse (value.toStdString());

				_settings_set.insert (name);
			}
		}

		if (!unconfigured_values.empty())
		{
			QStringList list;
			for (auto s: unconfigured_values)
				list << s;
			throw Exception (QString ("missing configuration for the following settings: %1").arg (list.join (", ")));
		}
	}
	catch (Exception& e)
	{
		throw Exception ("error when parsing <settings>", &e);
	}
}


bool
Module::has_setting (QString const& name)
{
	SettingsSet::iterator s = _settings_set.find (name);
	return s != _settings_set.end();
}


void
Module::parse_i2c (QDomElement const& i2c_element, I2C::Device& device)
{
	bool has_bus = false;
	bool has_address = false;

	for (QDomElement& e: i2c_element)
	{
		if (e == "bus")
		{
			device.bus().set_bus_number (e.text().trimmed().toUInt());
			has_bus = true;
		}
		else if (e == "address")
		{
			QString addr = e.text().trimmed();
			if (addr.startsWith ("0x"))
				device.set_address (I2C::Address (addr.mid (2).toUInt (nullptr, 16)));
			else
				device.set_address (I2C::Address (addr.toUInt()));
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


Time
Module::update_time() const
{
	return _module_manager->update_time();
}


Time
Module::update_dt() const
{
	return _module_manager->update_dt();
}


QWidget*
Module::configurator_widget() const
{
	return nullptr;
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


Accounting*
Module::accounting() const
{
	return _module_manager->application()->accounting();
}


Xefis::Logger const&
Module::log() const
{
	return _logger;
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


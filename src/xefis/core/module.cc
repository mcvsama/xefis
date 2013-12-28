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
	_settings_parser = std::make_unique<ConfigReader::SettingsParser>();
	_properties_parser = std::make_unique<ConfigReader::PropertiesParser>();
}


void
Module::dump_debug_log()
{
	for (auto const& s: _settings_parser->registered_names())
		log() << "* setting: " << s.toStdString() << std::endl;

	for (auto const& s: _properties_parser->registered_names())
		log() << "* property: " << s.toStdString() << std::endl;
}


void
Module::parse_properties (QDomElement const& element, ConfigReader::PropertiesParser::PropertiesList list)
{
	QDomElement properties_element;
	if (element == "properties")
		properties_element = element;
	else
	{
		for (QDomElement& e: element)
		{
			if (e == "properties")
			{
				if (properties_element.isNull())
					properties_element = e;
				else
					throw Exception ("multiple <properties> elements");
			}
		}
	}

	if (properties_element.isNull())
		throw Exception ("missing <properties> element");

	_properties_parser = std::make_unique<ConfigReader::PropertiesParser> (list);
	_properties_parser->parse (properties_element);
}


void
Module::parse_settings (QDomElement const& element, ConfigReader::SettingsParser::SettingsList list)
{
	QDomElement settings_element;
	if (element == "settings")
		settings_element = element;
	else
	{
		for (QDomElement& e: element)
		{
			if (e == "settings")
			{
				if (settings_element.isNull())
					settings_element = e;
				else
					throw Exception ("multiple <settings> elements");
			}
		}
	}

	if (settings_element.isNull())
		throw Exception ("missing <settings> element");

	_settings_parser = std::make_unique<ConfigReader::SettingsParser> (list);
	_settings_parser->parse (settings_element);
}


bool
Module::has_setting (QString const& name)
{
	return _settings_parser->has_setting (name);
}


void
Module::signal_data_updated()
{
	_signalled_data_update = true;
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


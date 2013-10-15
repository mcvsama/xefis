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
#include <xefis/core/application.h>
#include <xefis/core/module_manager.h>
#include <xefis/core/window_manager.h>
#include <xefis/core/module.h>
#include <xefis/core/window.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/numeric.h>

// Local:
#include "config_reader.h"


namespace Xefis {

ConfigReader::SettingsParser::SettingsParser (SettingsList const& list):
	_list (list)
{ }


void
ConfigReader::SettingsParser::parse (QDomElement const& settings_element)
{
	try {
		std::map<QString, NameAndSetting*> map;
		std::set<QString> unconfigured_values;
		std::set<QString> configured_values;
		std::set<QString> known_values;

		for (NameAndSetting& element: _list)
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
				else if (nas->value_int8)
					*nas->value_int8 = parse_int<int8_t> (value);
				else if (nas->value_int16)
					*nas->value_int16 = parse_int<int16_t> (value);
				else if (nas->value_int32)
					*nas->value_int32 = parse_int<int32_t> (value);
				else if (nas->value_int64)
					*nas->value_int64 = parse_int<int64_t> (value);
				else if (nas->value_uint8)
					*nas->value_uint8 = parse_int<uint8_t> (value);
				else if (nas->value_uint16)
					*nas->value_uint16 = parse_int<uint16_t> (value);
				else if (nas->value_uint32)
					*nas->value_uint32 = parse_int<uint32_t> (value);
				else if (nas->value_uint64)
					*nas->value_uint64 = parse_int<uint64_t> (value);
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

				_set.insert (name);
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
ConfigReader::SettingsParser::has_setting (QString const& name)
{
	SettingsSet::iterator s = _set.find (name);
	return s != _set.end();
}


template<class TargetInt>
	inline TargetInt
	ConfigReader::SettingsParser::parse_int (QString const& s)
	{
		static_assert (std::numeric_limits<TargetInt>::is_integer, "TargetInt must be integer type");

		if (std::numeric_limits<TargetInt>::is_signed)
		{
			if (s.startsWith ("0x"))
				return s.mid (2).toInt (nullptr, 16);
			return s.toInt();
		}
		else
		{
			if (s.startsWith ("0x"))
				return s.mid (2).toInt (nullptr, 16);
			return s.toInt();
		}
	}


ConfigReader::PropertiesParser::PropertiesParser (PropertiesList const& list):
	_list (list)
{ }


void
ConfigReader::PropertiesParser::parse (QDomElement const& properties_element)
{
	try {
		std::map<QString, TypedProperty*> map;
		std::set<QString> unconfigured_values;
		std::set<QString> configured_values;
		std::set<QString> known_values;

		for (NameAndProperty& element: _list)
		{
			if (map.insert ({ element.name, element.property }).second == false)
				throw Exception ("duplicated entry name in property list");
			if (element.required)
				unconfigured_values.insert (element.name);
			known_values.insert (element.name);
		}

		QString root = properties_element.attribute ("path");

		auto parse_property = [&] (QDomElement const& e) -> void
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
				std::string path = (root + e.attribute ("path")).toStdString();
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

		std::function<void (QDomElement const& e)> parse_element;

		auto parse_directory = [&] (QDomElement const& e) -> void
		{
			QString orig_root = root;
			QString subdir = e.attribute ("path");
			root += subdir;
			for (QDomElement const& sub: e)
				parse_element (sub);
			root = orig_root;
		};

		parse_element = [&] (QDomElement const& e) -> void
		{
			if (e == "property")
				parse_property (e);
			else if (e == "directory")
				parse_directory (e);
		};

		for (QDomElement& d: properties_element)
			parse_element (d);

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
		if (e == "settings")
			process_settings_element (e);
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
ConfigReader::process_settings_element (QDomElement const& settings_element)
{
	SettingsParser sp ({
		{ "navaids.enable", _navaids_enable, false },
		{ "scale.pen", _scale_pen, false },
		{ "scale.font", _scale_font, false },
		// TODO use these settings in Instrument modules
	});
	sp.parse (settings_element);
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
	_application->window_manager()->add_window (window.get());
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


Module*
ConfigReader::process_module_element (QDomElement const& module_element, QWidget* window)
{
	if (module_element.attribute ("disabled") == "true")
		return nullptr;

	QString name = module_element.attribute ("name");
	QString instance = module_element.attribute ("instance");

	return _module_manager->load_module (name, instance, module_element, window);
}

} // namespace Xefis


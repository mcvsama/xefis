/* vim:ts=4
 *
 * Copyleft 2012…2016  Michał Gawron
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
#include <limits>
#include <type_traits>

// System:
#include <unistd.h>

// Boost:
#include <boost/lexical_cast.hpp>

// Qt:
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtWidgets/QWidget>
#include <QtWidgets/QLayout>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v1/module_manager.h>
#include <xefis/core/v1/window_manager.h>
#include <xefis/core/v1/module.h>
#include <xefis/core/v1/window.h>
#include <xefis/core/stdexcept.h>
#include <xefis/core/xefis.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/qdom_iterator.h>
#include <xefis/utility/numeric.h>

// Local:
#include "config_reader.h"


namespace v1 {
using namespace xf;

ConfigReader::SettingsParser::NameAndSetting::NameAndSetting (NameAndSetting const& other):
	name (other.name),
	required (other.required),
	_holder (other._holder)
{ }


ConfigReader::SettingsParser::NameAndSetting&
ConfigReader::SettingsParser::NameAndSetting::operator= (NameAndSetting const& other)
{
	name = other.name;
	required = other.required;
	_holder = other._holder;
	return *this;
}


void
ConfigReader::SettingsParser::NameAndSetting::assign_setting_value (QString const& value_str)
{
	_holder->assign_setting_value (value_str);
}


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
				throw BadConfiguration ("duplicated entry name in settings list");
			if (element.required)
				unconfigured_values.insert (element.name);
			known_values.insert (element.name);
		}

		for (QDomElement const& d: xf::iterate_sub_elements (settings_element))
		{
			if (d == "setting")
			{
				if (!d.hasAttribute ("name"))
					throw MissingDomAttribute (d, "name");

				QString name = d.attribute ("name");

				if (known_values.find (name) == known_values.end())
					throw BadConfiguration (QString ("configuration for unknown setting: %1").arg (name));

				if (configured_values.find (name) != configured_values.end())
					throw BadConfiguration (QString ("duplicated entry <properties>: %1").arg (name));

				if (!d.hasAttribute ("value"))
					throw MissingDomAttribute (d, "value");

				QString value = d.attribute ("value");

				unconfigured_values.erase (name);
				configured_values.insert (name);

				auto it = map.find (name);
				if (it == map.end())
					return;

				NameAndSetting* nas = it->second;
				nas->assign_setting_value (value);

				_set.insert (name);
			}
		}

		if (!unconfigured_values.empty())
		{
			QStringList list;
			for (auto s: unconfigured_values)
				list << s;
			throw BadConfiguration (QString ("missing configuration for the following settings: %1").arg (list.join (", ")));
		}
	}
	catch (Exception& e)
	{
		throw BadConfiguration ("error when parsing <settings>", &e);
	}
}


std::vector<QString>
ConfigReader::SettingsParser::registered_names() const
{
	std::vector<QString> result;
	for (auto const& p: _list)
		result.push_back (p.name);
	return result;
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
				return s.mid (2).toUInt (nullptr, 16);
			return s.toUInt();
		}
	}


ConfigReader::PropertiesParser::PropertiesParser (PropertiesList const& list):
	_list (list)
{ }


void
ConfigReader::PropertiesParser::parse (QDomElement const& properties_element)
{
	try {
		std::map<QString, GenericProperty*> map;
		std::set<QString> unconfigured_values;
		std::set<QString> configured_values;
		std::set<QString> known_values;

		for (NameAndProperty& element: _list)
		{
			if (map.insert ({ element.name, element.property }).second == false)
				throw BadConfiguration ("duplicated entry name in property list");
			if (element.required)
				unconfigured_values.insert (element.name);
			known_values.insert (element.name);
		}

		QString root = properties_element.attribute ("path");

		auto parse_property = [&] (QDomElement const& e) -> void
		{
			if (!e.hasAttribute ("name"))
				throw MissingDomAttribute (e, "name");

			QString name = e.attribute ("name");

			if (known_values.find (name) == known_values.end())
				throw BadConfiguration (QString ("configuration for unknown property: %1").arg (name));

			if (configured_values.find (name) != configured_values.end())
				throw BadConfiguration (QString ("duplicated entry <properties>: %1").arg (name));

			unconfigured_values.erase (name);
			configured_values.insert (name);

			auto it = map.find (name);
			// Found config for nonexistent property:
			if (it == map.end())
				return;

			if (e.hasAttribute ("path"))
				it->second->set_path (PropertyPath (root + e.attribute ("path")));
			else
				throw MissingDomAttribute (e, "path");

			it->second->ensure_existence();

			//if (!e.hasAttribute ("default"))
			//{
			//	// If default value is not given, require attribute 'type' to be present:
			//	if (!e.hasAttribute ("type"))
			//		throw MissingDomAttribute (e, "type");
			//	else
			//		it->second->ensure_existence (PropertyType (e.attribute ("type")));
			//}
			//else
			//{
			//	QString value = e.attribute ("default");
			//	it->second->create_and_parse (value.toStdString());
			//}
		};

		std::function<void (QDomElement const& e)> parse_element;

		auto parse_directory = [&] (QDomElement const& e) -> void
		{
			QString orig_root = root;
			QString subdir = e.attribute ("path");
			root += subdir;
			for (QDomElement const& sub: xf::iterate_sub_elements (e))
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

		for (QDomElement const& d: xf::iterate_sub_elements (properties_element))
			parse_element (d);

		if (!unconfigured_values.empty())
		{
			QStringList list;
			for (auto s: unconfigured_values)
				list << s;
			throw BadConfiguration (QString ("missing configuration for the following properties: %1").arg (list.join (", ")));
		}
	}
	catch (Exception& e)
	{
		throw BadConfiguration ("error when parsing <properties>", &e);
	}
}


std::vector<QString>
ConfigReader::PropertiesParser::registered_names() const
{
	std::vector<QString> result;
	for (auto const& p: _list)
		result.push_back (p.name);
	return result;
}


ConfigReader::ConfigReader (Xefis* xefis, ModuleManager* module_manager):
	_xefis (xefis),
	_module_manager (module_manager)
{
	_logger.set_prefix ("<config reader>");
	_logger << "Creating ConfigReader" << std::endl;
	const char* mode = getenv ("XEFIS_MODE");
	if (mode)
		_config_mode = mode;
}


ConfigReader::~ConfigReader()
{
	_logger << "Destroying ConfigReader" << std::endl;
}


void
ConfigReader::load (QString const& path)
{
	int last_slash = path.lastIndexOf ('/');
	QString dirname = path.left (last_slash + 1);
	QString basename = path.mid (last_slash + 1);

	QDir cwd;
	_current_dir = cwd.absolutePath() + '/' + dirname;

	_config_document = parse_file (basename);
	preprocess();

	_current_dir = cwd;
}


void
ConfigReader::process_settings()
{
	for (auto e: _settings_elements)
		process_settings_element (e);
}


void
ConfigReader::process_modules()
{
	for (auto e: _modules_elements)
		process_modules_element (e);
}


void
ConfigReader::process_windows()
{
	for (auto e: _windows_elements)
		process_windows_element (e);
}


QDomElement
ConfigReader::module_config (QString const& name, QString const& instance) const
{
	auto m = _module_configs.find ({ name, instance });
	if (m == _module_configs.end())
		throw Exception ("no config found for " + name + "#" + instance);
	return m->second;
}


QDomDocument
ConfigReader::parse_file (QString const& path)
{
	QFile file (_current_dir.absolutePath() + '/' + path);
	QDomDocument doc;

	if (!file.exists())
		throw BadConfiguration (("file not found: " + path).toStdString());

	if (!file.open (QFile::ReadOnly))
		throw BadConfiguration (("file access error: " + path).toStdString());

	if (!doc.setContent (&file, true))
		throw BadConfiguration (("config parse error: " + path).toStdString());

	return doc;
}


void
ConfigReader::preprocess()
{
	QDomElement root = _config_document.documentElement();

	process_ifs (root);
	process_includes (root);

	if (root != "xefis-config")
		throw BadConfiguration (("config process error: unsupported root tag: " + root.tagName()).toStdString());

	for (QDomElement const& e: xf::iterate_sub_elements (root))
	{
		if (e == "settings")
			_settings_elements.push_back (e);
		else if (e == "windows")
			_windows_elements.push_back (e);
		else if (e == "modules")
			_modules_elements.push_back (e);
		else if (e == "airframe")
			_airframe_config = e;
		else
			throw BadDomElement (e);
	}
}


void
ConfigReader::process_includes (QDomElement parent)
{
	std::list<QDomElement> to_remove;

	for (QDomElement const& e: xf::iterate_sub_elements (parent))
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

			for (QDomElement const& x: xf::iterate_sub_elements (sub_doc.documentElement()))
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
ConfigReader::process_ifs (QDomElement parent)
{
	std::list<QDomElement> to_remove;

	for (QDomElement& e: xf::iterate_sub_elements (parent))
	{
		if (e == "if")
		{
			if (e.hasAttribute ("mode") && e.attribute ("mode") == _config_mode)
			{
				std::list<QDomElement> moveup;

				for (QDomElement const& c: xf::iterate_sub_elements (e))
					moveup.push_back (c);

				for (QDomElement& c: moveup)
				{
					e.removeChild (c);
					parent.insertAfter (c, e);
				}

				for (QDomElement const& c: xf::iterate_sub_elements (e))
					process_ifs (c);
			}

			to_remove.push_back (e);
		}
		else
			process_ifs (e);
	}

	for (QDomElement& e: to_remove)
		parent.removeChild (e);
}


void
ConfigReader::process_settings_element (QDomElement const& settings_element)
{
	SettingsParser sp ({
		{ "update-frequency", _update_frequency, false },
		{ "navaids.enable", _navaids_enable, false },
		{ "scale.pen", _scale_pen, false },
		{ "scale.font", _scale_font, false },
		{ "scale.master", _scale_master, false },
		{ "scale.windows", _scale_windows, false },
	});
	sp.parse (settings_element);
}


void
ConfigReader::process_windows_element (QDomElement const& windows_element)
{
	for (QDomElement const& e: xf::iterate_sub_elements (windows_element))
	{
		if (e == "window")
			process_window_element (e);
		else
			throw BadDomElement (e);
	}
}


void
ConfigReader::process_window_element (QDomElement const& window_element)
{
	if (window_element.attribute ("disabled") == "true")
		return;

	// Auto delete if Window throws an exception:
	Unique<Window> window = std::make_unique<Window> (_xefis, this, window_element);
	window->show();
	_xefis->window_manager()->add_window (std::move (window));
	// window is now gone
	_has_windows = true;
}


void
ConfigReader::process_modules_element (QDomElement const& modules_element)
{
	for (QDomElement const& e: xf::iterate_sub_elements (modules_element))
	{
		if (e == "module")
			process_module_element (e);
		else
			throw BadDomElement (e);
	}
}


Module*
ConfigReader::process_module_element (QDomElement const& module_element, QWidget* parent_widget)
{
	if (module_element.attribute ("disabled") == "true")
		return nullptr;

	QString name = module_element.attribute ("name");
	QString instance = module_element.attribute ("instance");

	_module_configs[{ name, instance }] = module_element;

	return _module_manager->load_module (name, instance, module_element, parent_widget);
}

} // namespace v1


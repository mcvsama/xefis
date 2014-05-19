/* vim:ts=4
 *
 * Copyleft 2012…2014  Michał Gawron
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
#include <map>
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
class Module;

class ConfigReader
{
	friend class Window;

  public:
	/**
	 * Standard parser for a <settings> element, used in module configurations.
	 */
	class SettingsParser
	{
	  public:
		struct NameAndSetting
		{
			QString			name;
			bool			required;
#define XEFIS_DEF_ATTR_LOW(type, name) \
	type* value_##name = nullptr; \
	Optional<type>* value_optional_##name = nullptr;
#define XEFIS_DEF_ATTR(type_name) \
	XEFIS_DEF_ATTR_LOW (type_name, type_name)
			XEFIS_DEF_ATTR (bool)
			XEFIS_DEF_ATTR (int8_t)
			XEFIS_DEF_ATTR (int16_t)
			XEFIS_DEF_ATTR (int32_t)
			XEFIS_DEF_ATTR (int64_t)
			XEFIS_DEF_ATTR (uint8_t)
			XEFIS_DEF_ATTR (uint16_t)
			XEFIS_DEF_ATTR (uint32_t)
			XEFIS_DEF_ATTR (uint64_t)
			XEFIS_DEF_ATTR (float)
			XEFIS_DEF_ATTR (double)
			XEFIS_DEF_ATTR_LOW (std::string, string)
			XEFIS_DEF_ATTR_LOW (QString, qstring)
			XEFIS_DEF_ATTR_LOW (SI::Value, si_value)
#undef XEFIS_DEF_ATTR
#undef XEFIS_DEF_ATTR_LOW

#define XEFIS_DEF_CONSTR(type) \
	NameAndSetting (QString const& name, type& value, bool required); \
	NameAndSetting (QString const& name, Optional<type>& value, bool required);
#define XEFIS_DEF_CONSTR_SINGLE(type) \
	NameAndSetting (QString const& name, type& value, bool required);
			XEFIS_DEF_CONSTR (bool)
			XEFIS_DEF_CONSTR (int8_t)
			XEFIS_DEF_CONSTR (int16_t)
			XEFIS_DEF_CONSTR (int32_t)
			XEFIS_DEF_CONSTR (int64_t)
			XEFIS_DEF_CONSTR (uint8_t)
			XEFIS_DEF_CONSTR (uint16_t)
			XEFIS_DEF_CONSTR (uint32_t)
			XEFIS_DEF_CONSTR (uint64_t)
			XEFIS_DEF_CONSTR (float)
			XEFIS_DEF_CONSTR (double)
			XEFIS_DEF_CONSTR (std::string)
			XEFIS_DEF_CONSTR (QString)
			//  This is without Optional version, since Optional<SI::Value>
			//  and Optional<derived-of-SI::Value> are not convertible.
			XEFIS_DEF_CONSTR_SINGLE (SI::Value)
#undef XEFIS_DEF_CONSTR_SINGLE
#undef XEFIS_DEF_CONSTR
		};

		typedef std::vector<NameAndSetting>	SettingsList;
		typedef std::set<QString>			SettingsSet;

	  public:
		/**
		 * Create parser with empty settings list.
		 */
		SettingsParser() = default;

		/**
		 * Create parser and register list of name/variable paris.
		 * As settings are parsed from the config DOM elmeent,
		 * appropriate setting values are assigned to variables
		 * referenced in list.
		 */
		explicit SettingsParser (SettingsList const& list);

		/**
		 * Parse element and assign values.
		 */
		void
		parse (QDomElement const&);

		/**
		 * Return list of registered (not parsed) setting names.
		 */
		std::vector<QString>
		registered_names() const;

		/**
		 * Return true if given setting has been found in DOM configuration.
		 */
		bool
		has_setting (QString const& name);

	  private:
		/**
		 * Parse int. Support 0x prefix for hexadecimal values.
		 */
		template<class TargetInt>
			static TargetInt
			parse_int (QString const&);

	  private:
		SettingsList	_list;
		SettingsSet		_set;
	};

	/**
	 * Standard parser for a <properties> element, used in module configurations.
	 */
	class PropertiesParser
	{
	  public:
		struct NameAndProperty
		{
			NameAndProperty (QString const& name, TypedProperty& property, bool required):
				name (name), property (&property), required (required)
			{ }

			QString			name;
			TypedProperty*	property;
			bool			required;
		};

		typedef std::vector<NameAndProperty> PropertiesList;

	  public:
		/**
		 * Create parser with empty properties list.
		 */
		PropertiesParser() = default;

		/**
		 * Create parser and register list of name/variable paris.
		 * As properties are read from DOM configuration, referenced
		 * properties are configured according to the XML configuration
		 * (path, etc).
		 */
		explicit PropertiesParser (PropertiesList const& list);

		/**
		 * Process configuration element <properties> and assign values
		 * to properties.
		 */
		void
		parse (QDomElement const&);

		/**
		 * Return list of registered property names.
		 */
		std::vector<QString>
		registered_names() const;

	  private:
		PropertiesList	_list;
	};

	typedef std::map<std::pair<QString, QString>, QDomElement> ModuleConfigs;

  public:
	// Ctor
	ConfigReader (Application*, ModuleManager*);

	// Dtor
	~ConfigReader();

	/**
	 * Read config, parse it, prepare for processing, but don't yet load anything.
	 * Sub-configurations will become accessible after this call, like fe. airframe_config().
	 */
	void
	load (QString const& path);

	/**
	 * Preprocess config's settings.
	 */
	void
	process_settings();

	/**
	 * Load non-instrument modules.
	 */
	void
	process_modules();

	/**
	 * Process and show windows.
	 */
	void
	process_windows();

	/**
	 * Determines if there are any windows (and instruments)
	 * configured.
	 */
	bool
	has_windows() const;

	/**
	 * Return module update frequency.
	 */
	Frequency
	update_frequency() const noexcept;

	/**
	 * Return true if navaids are supposed to be loaded.
	 */
	bool
	load_navaids() const noexcept;

	/**
	 * Return master pen scale.
	 */
	float
	pen_scale() const noexcept;

	/**
	 * Return master font scale.
	 */
	float
	font_scale() const noexcept;

	/**
	 * Return sub-configuration for the Airframe module.
	 */
	QDomElement
	airframe_config() const;

	/**
	 * Return configuration element for given module name/instance pair.
	 */
	QDomElement
	module_config (QString const& name, QString const& instance) const;

  private:
	QDomDocument
	parse_file (QString const& path);

	void
	preprocess();

	void
	process_includes (QDomElement parent);

	void
	process_ifs (QDomElement parent);

	void
	process_settings_element (QDomElement const& settings_element);

	void
	process_windows_element (QDomElement const& windows_element);

	void
	process_window_element (QDomElement const& window_element);

	void
	process_modules_element (QDomElement const& modules_element);

	Module*
	process_module_element (QDomElement const& module_element, QWidget* parent_widget = nullptr);

  private:
	Logger					_logger;
	Application*			_application		= nullptr;
	ModuleManager*			_module_manager		= nullptr;
	QDomDocument			_config_document;
	QDir					_current_dir;
	QString					_config_mode;
	std::list<QDomElement>	_settings_elements;
	std::list<QDomElement>	_windows_elements;
	std::list<QDomElement>	_modules_elements;
	QDomElement				_airframe_config;
	bool					_has_windows		= false;
	Frequency				_update_frequency	= 100_Hz;
	bool					_navaids_enable		= true;
	float					_scale_pen			= 1.f;
	float					_scale_font			= 1.f;
	ModuleConfigs			_module_configs;
};


#define XEFIS_DEF_CONSTR_SINGLE(type, xname) \
	inline ConfigReader::SettingsParser::NameAndSetting::NameAndSetting (QString const& name, type& value, bool required): \
		name (name), \
		required (required), \
		value_##xname (&value) \
	{ }
#define XEFIS_DEF_CONSTR_LOW(type, xname) \
	inline ConfigReader::SettingsParser::NameAndSetting::NameAndSetting (QString const& name, type& value, bool required): \
		name (name), \
		required (required), \
		value_##xname (&value) \
	{ } \
	inline ConfigReader::SettingsParser::NameAndSetting::NameAndSetting (QString const& name, Optional<type>& value, bool required): \
		name (name), \
		required (required), \
		value_optional_##xname (&value) \
	{ }
#define XEFIS_DEF_CONSTR(type_name) \
	XEFIS_DEF_CONSTR_LOW (type_name, type_name)

XEFIS_DEF_CONSTR (bool)
XEFIS_DEF_CONSTR (int8_t)
XEFIS_DEF_CONSTR (int16_t)
XEFIS_DEF_CONSTR (int32_t)
XEFIS_DEF_CONSTR (int64_t)
XEFIS_DEF_CONSTR (uint8_t)
XEFIS_DEF_CONSTR (uint16_t)
XEFIS_DEF_CONSTR (uint32_t)
XEFIS_DEF_CONSTR (uint64_t)
XEFIS_DEF_CONSTR (float)
XEFIS_DEF_CONSTR (double)
XEFIS_DEF_CONSTR_LOW (std::string, string)
XEFIS_DEF_CONSTR_LOW (QString, qstring)
XEFIS_DEF_CONSTR_SINGLE (SI::Value, si_value)

#undef XEFIS_DEF_CONSTR
#undef XEFIS_DEF_CONSTR_LOW


inline bool
ConfigReader::has_windows() const
{
	return _has_windows;
}


inline Frequency
ConfigReader::update_frequency() const noexcept
{
	return _update_frequency;
}


inline bool
ConfigReader::load_navaids() const noexcept
{
	return _navaids_enable;
}


inline float
ConfigReader::pen_scale() const noexcept
{
	return _scale_pen;
}


inline float
ConfigReader::font_scale() const noexcept
{
	return _scale_font;
}


inline QDomElement
ConfigReader::airframe_config() const
{
	return _airframe_config;
}

} // namespace Xefis

#endif


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
class Module;

class ConfigReader
{
	friend class Window;

  public:
	/**
	 * Standard parser for a <settings> element.
	 */
	class SettingsParser
	{
	  public:
		struct NameAndSetting
		{
			QString			name;
			bool			required;
			bool*			value_bool		= nullptr;
			int8_t*			value_int8		= nullptr;
			int16_t*		value_int16		= nullptr;
			int32_t*		value_int32		= nullptr;
			int64_t*		value_int64		= nullptr;
			uint8_t*		value_uint8		= nullptr;
			uint16_t*		value_uint16	= nullptr;
			uint32_t*		value_uint32	= nullptr;
			uint64_t*		value_uint64	= nullptr;
			float*			value_float		= nullptr;
			double*			value_double	= nullptr;
			std::string*	value_string	= nullptr;
			QString*		value_qstring	= nullptr;
			SI::Value*		value_si_value	= nullptr;

			NameAndSetting (QString const& name, bool& value, bool required);

			NameAndSetting (QString const& name, int8_t& value, bool required);

			NameAndSetting (QString const& name, int16_t& value, bool required);

			NameAndSetting (QString const& name, int32_t& value, bool required);

			NameAndSetting (QString const& name, int64_t& value, bool required);

			NameAndSetting (QString const& name, uint8_t& value, bool required);

			NameAndSetting (QString const& name, uint16_t& value, bool required);

			NameAndSetting (QString const& name, uint32_t& value, bool required);

			NameAndSetting (QString const& name, uint64_t& value, bool required);

			NameAndSetting (QString const& name, float& value, bool required);

			NameAndSetting (QString const& name, double& value, bool required);

			NameAndSetting (QString const& name, std::string& value, bool required);

			NameAndSetting (QString const& name, QString& value, bool required);

			NameAndSetting (QString const& name, SI::Value& value, bool required);
		};

		typedef std::vector<NameAndSetting>	SettingsList;
		typedef std::set<QString>			SettingsSet;

	  public:
		// Ctor
		SettingsParser() = default;

		// Ctor
		explicit SettingsParser (SettingsList const& list);

		/**
		 * Parse element and assign values.
		 */
		void
		parse (QDomElement const&);

		/**
		 * Return true if given setting has been found in configuration.
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
	 * Standard parser for a <properties> element.
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
		// Ctor
		PropertiesParser() = default;

		// Ctor
		explicit PropertiesParser (PropertiesList const& list);

		/**
		 * Parse element and assign values.
		 */
		void
		parse (QDomElement const&);

	  private:
		PropertiesList	_list;
	};

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

	/**
	 * Return master pen scale.
	 */
	float
	pen_scale() const;

	/**
	 * Return master font scale.
	 */
	float
	font_scale() const;

  private:
	QDomDocument
	parse_file (QString const& path);

	void
	process();

	void
	process_includes (QDomElement parent);

	void
	process_settings_element (QDomElement const& settings_element);

	void
	process_windows_element (QDomElement const& windows_element);

	void
	process_window_element (QDomElement const& window_element);

	void
	process_modules_element (QDomElement const& modules_element);

	Module*
	process_module_element (QDomElement const& module_element, QWidget* window = nullptr);

  private:
	Application*	_application		= nullptr;
	ModuleManager*	_module_manager		= nullptr;
	QDomDocument	_config_document;
	QDir			_current_dir;
	bool			_has_windows		= false;
	bool			_navaids_enable		= true;
	float			_scale_pen			= 1.f;
	float			_scale_font			= 1.f;
};


class ConfigException: public Exception
{
  public:
	ConfigException (std::string const& message):
		Exception (message)
	{ }
};


inline
ConfigReader::SettingsParser::NameAndSetting::NameAndSetting (QString const& name, bool& value, bool required):
	name (name),
	required (required),
	value_bool (&value)
{ }


inline
ConfigReader::SettingsParser::NameAndSetting::NameAndSetting (QString const& name, int8_t& value, bool required):
	name (name),
	required (required),
	value_int8 (&value)
{ }


inline
ConfigReader::SettingsParser::NameAndSetting::NameAndSetting (QString const& name, int16_t& value, bool required):
	name (name),
	required (required),
	value_int16 (&value)
{ }


inline
ConfigReader::SettingsParser::NameAndSetting::NameAndSetting (QString const& name, int32_t& value, bool required):
	name (name),
	required (required),
	value_int32 (&value)
{ }


inline
ConfigReader::SettingsParser::NameAndSetting::NameAndSetting (QString const& name, int64_t& value, bool required):
	name (name),
	required (required),
	value_int64 (&value)
{ }


inline
ConfigReader::SettingsParser::NameAndSetting::NameAndSetting (QString const& name, uint8_t& value, bool required):
	name (name),
	required (required),
	value_uint8 (&value)
{ }


inline
ConfigReader::SettingsParser::NameAndSetting::NameAndSetting (QString const& name, uint16_t& value, bool required):
	name (name),
	required (required),
	value_uint16 (&value)
{ }


inline
ConfigReader::SettingsParser::NameAndSetting::NameAndSetting (QString const& name, uint32_t& value, bool required):
	name (name),
	required (required),
	value_uint32 (&value)
{ }


inline
ConfigReader::SettingsParser::NameAndSetting::NameAndSetting (QString const& name, uint64_t& value, bool required):
	name (name),
	required (required),
	value_uint64 (&value)
{ }


inline
ConfigReader::SettingsParser::NameAndSetting::NameAndSetting (QString const& name, float& value, bool required):
	name (name),
	required (required),
	value_float (&value)
{ }


inline
ConfigReader::SettingsParser::NameAndSetting::NameAndSetting (QString const& name, double& value, bool required):
	name (name),
	required (required),
	value_double (&value)
{ }


inline
ConfigReader::SettingsParser::NameAndSetting::NameAndSetting (QString const& name, std::string& value, bool required):
	name (name),
	required (required),
	value_string (&value)
{ }


inline
ConfigReader::SettingsParser::NameAndSetting::NameAndSetting (QString const& name, QString& value, bool required):
	name (name),
	required (required),
	value_qstring (&value)
{ }


inline
ConfigReader::SettingsParser::NameAndSetting::NameAndSetting (QString const& name, SI::Value& value, bool required):
	name (name),
	required (required),
	value_si_value (&value)
{ }


inline bool
ConfigReader::has_windows() const
{
	return _has_windows;
}


inline bool
ConfigReader::load_navaids() const
{
	return _navaids_enable;
}


inline float
ConfigReader::pen_scale() const
{
	return _scale_pen;
}


inline float
ConfigReader::font_scale() const
{
	return _scale_font;
}

} // namespace Xefis

#endif


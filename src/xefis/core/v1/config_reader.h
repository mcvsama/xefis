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

#ifndef XEFIS__CORE__CONFIG_READER_H__INCLUDED
#define XEFIS__CORE__CONFIG_READER_H__INCLUDED

// Standard:
#include <cstddef>
#include <map>
#include <set>
#include <stdexcept>
#include <functional>

// Qt:
#include <QtCore/QDir>
#include <QtXml/QDomElement>
#include <QtXml/QDomDocument>
#include <QtWidgets/QLayout>

// Boost:
#include <boost/any.hpp>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/config/exception.h>
#include <xefis/core/v1/property.h>
#include <xefis/utility/logger.h>


namespace xf {
class Xefis;
}

namespace v1 {
using namespace xf;

class ModuleManager;
class Module;


namespace detail {
namespace name_and_setting {

static inline void
assign (bool& target, std::string const& value_str)
{
	target = value_str == "true";
}


static inline void
assign (std::string& target, std::string const& value_str)
{
	target = value_str;
}


static inline void
assign (QString& target, std::string const& value_str)
{
	target = QString::fromStdString (value_str);
}


// Covers arithmetic types (int_Xt, uint_Xt, float, double).
template<class V,
		 std::enable_if_t<std::is_arithmetic<V>::value, int> = 0>
	static inline void
	assign (V& target, std::string const& value_str)
	{
		target = boost::lexical_cast<V> (value_str);
	}


// Covers si::Quantity<> types.
template<class Q,
		 std::enable_if_t<si::is_quantity<Q>::value, int> = 0>
	static inline void
	assign (Q& target, std::string const& value_str)
	{
		parse (value_str, target);
	}


// Covers Optional<> types.
template<class T>
	static inline void
	assign (Optional<T>& target, std::string const& value_str)
	{
		T t;
		assign (t, value_str);
		target = t;
	}

} // namespace name_and_setting
} // namespace detail


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
		/**
		 * Type-erasing class that takes a reference to any object that will hold
		 * the desired setting value (holder_reference).
		 */
		class NameAndSetting
		{
			/**
			 * Holds reference to a target object, where settings value will be stored.
			 * Actually derived classes will hold that reference.
			 */
			class Holder
			{
			  public:
				virtual void
				assign_setting_value (QString const&) = 0;
			};

		  public:
			QString			name;
			bool			required;

		  public:
			// Ctor
			template<class T>
				NameAndSetting (QString const& name, T& target, bool required);

			// Ctor
			NameAndSetting (NameAndSetting const&);

			// Copy operator
			NameAndSetting&
			operator= (NameAndSetting const&);

			/**
			 * Assign a setting value to a target setting container.
			 */
			void
			assign_setting_value (QString const&);

		  private:
			Shared<Holder> _holder;
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
		SettingsParser (SettingsList const& list);

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
			// Ctor
			NameAndProperty (QString const& name, GenericProperty& property, bool required):
				name (name), property (&property), required (required)
			{ }

			QString				name;
			GenericProperty*	property;
			bool				required;
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
		PropertiesParser (PropertiesList const& list);

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
	explicit
	ConfigReader (Xefis*, ModuleManager*);

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
	 * Return scaling factor for pens/lines.
	 */
	float
	pen_scale() const noexcept;

	/**
	 * Return scaling factor for fonts.
	 */
	float
	font_scale() const noexcept;

	/**
	 * Return scaling factor for UI windows.
	 */
	float
	windows_scale() const noexcept;

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
	Xefis*					_xefis				= nullptr;
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
	float					_scale_master		= 1.f;
	float					_scale_windows		= 1.f;
	ModuleConfigs			_module_configs;
};


template<class T>
	inline
	ConfigReader::SettingsParser::NameAndSetting::NameAndSetting (QString const& name, T& target, bool required):
		name (name),
		required (required)
	{
		class HolderImpl: public Holder
		{
		  public:
			HolderImpl (T& target):
				_target (target)
			{ }

			void
			assign_setting_value (QString const& value_str) override
			{
				detail::name_and_setting::assign (_target, value_str.toStdString());
			}

		  private:
			T& _target;
		};

		_holder = std::make_shared<HolderImpl> (target);
	}


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
	return _scale_master * _scale_pen;
}


inline float
ConfigReader::font_scale() const noexcept
{
	return _scale_master * _scale_font;
}


inline float
ConfigReader::windows_scale() const noexcept
{
	return _scale_windows;
}


inline QDomElement
ConfigReader::airframe_config() const
{
	return _airframe_config;
}

} // namespace v1

#endif


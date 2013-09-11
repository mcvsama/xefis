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

#ifndef XEFIS__CORE__MODULE_H__INCLUDED
#define XEFIS__CORE__MODULE_H__INCLUDED

// Standard:
#include <cstddef>
#include <functional>
#include <ostream>

// Qt:
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/application.h>
#include <xefis/core/navaid_storage.h>
#include <xefis/core/property.h>
#include <xefis/utility/noncopyable.h>
#include <xefis/utility/i2c.h>
#include <xefis/utility/logger.h>


#define XEFIS_REGISTER_MODULE_CLASS(module_name, klass) \
	static Xefis::Module::Registrator module_registrator (module_name, [](Xefis::ModuleManager* module_manager, QDomElement const& config) -> Xefis::Module* { \
		return new klass (module_manager, config); \
	});


namespace Xefis {

class ModuleManager;

class Module: private Noncopyable
{
  public:
	struct NameAndProperty
	{
		QString			name;
		TypedProperty&	property;
		bool			required;
	};

	struct NameAndSetting
	{
		QString			name;
		bool			required;
		bool*			value_bool		= nullptr;
		int*			value_int		= nullptr;
		float*			value_float		= nullptr;
		double*			value_double	= nullptr;
		std::string*	value_string	= nullptr;
		QString*		value_qstring	= nullptr;
		SI::Value*		value_si_value	= nullptr;

		NameAndSetting (QString const& name, bool& value, bool required);

		NameAndSetting (QString const& name, int& value, bool required);

		NameAndSetting (QString const& name, float& value, bool required);

		NameAndSetting (QString const& name, double& value, bool required);

		NameAndSetting (QString const& name, std::string& value, bool required);

		NameAndSetting (QString const& name, QString& value, bool required);

		NameAndSetting (QString const& name, SI::Value& value, bool required);
	};

	typedef std::vector<NameAndProperty>								PropertiesList;
	typedef std::vector<NameAndSetting>									SettingsList;
	typedef std::set<QString>											SettingsSet;
	typedef std::function<Module* (ModuleManager*, QDomElement const&)>	FactoryFunction;
	typedef std::map<std::string, FactoryFunction>						FactoriesMap;

	class Registrator
	{
	  public:
		Registrator (std::string const& module_name, FactoryFunction);
	};

	class Pointer
	{
	  public:
		// Ctor
		Pointer() = default;

		// Ctor
		Pointer (std::string const& name, std::string const& instance);

		/**
		 * Comparison operator: first by name, then by instance.
		 */
		bool
		operator< (Pointer const& other) const;

		/**
		 * Return module name (class).
		 */
		std::string const&
		name() const noexcept;

		/**
		 * Return module instance.
		 */
		std::string const&
		instance() const noexcept;

	  private:
		std::string	_name;
		std::string	_instance;
	};

  public:
	// Ctor
	Module (ModuleManager*, QDomElement const& config);

	// Dtor
	virtual ~Module();

	/**
	 * Signal that the data in property tree has been updated.
	 * Default implementation does nothing.
	 */
	virtual void
	data_updated();

	/**
	 * Return last update time.
	 */
	Time
	update_time() const;

	/**
	 * Return time difference between last and previous update.
	 */
	Time
	update_dt() const;

	/**
	 * Register module factory.
	 */
	static void
	register_factory (std::string const& module_name, FactoryFunction);

	/**
	 * Return factory function by name.
	 */
	static FactoryFunction
	find_factory (std::string const& name);

  protected:
	/**
	 * Parse the <properties> element and initialize properties
	 * by their names matching the <properties> children.
	 * \throw	Xefis::Exception if something's wrong.
	 */
	void
	parse_properties (QDomElement const& properties_element, PropertiesList);

	/**
	 * Parse the <settings> element and initialize variables.
	 * \throw	Xefis::Exception if something's wrong.
	 */
	void
	parse_settings (QDomElement const& settings_element, SettingsList);

	/**
	 * Return true if given setting has been found in configuration.
	 */
	bool
	has_setting (QString const& name);

	/**
	 * Parse the <i2c> element containing I2C bus number and device
	 * address.
	 * \throw	Xefis::Exception if something's wrong.
	 */
	void
	parse_i2c (QDomElement const& i2c_element, I2C::Bus&, I2C::Address&);

	/**
	 * Signal that this module has updated property tree.
	 */
	void
	signal_data_updated();

	/**
	 * Access NavaidStorage.
	 */
	NavaidStorage*
	navaid_storage() const;

	/**
	 * Access work performer.
	 */
	WorkPerformer*
	work_performer() const;

	/**
	 * Access accounting information.
	 */
	Accounting*
	accounting() const;

	/**
	 * Add header with module name to the log stream and
	 * return the stream.
	 */
	Xefis::Logger const&
	log() const;

  private:
	/**
	 * Return list of factories.
	 */
	static FactoriesMap&
	factories();

  private:
	ModuleManager*	_module_manager = nullptr;
	std::string		_name;
	std::string		_instance;
	SettingsSet		_settings_set;
	Xefis::Logger	_logger;
};


inline
Module::NameAndSetting::NameAndSetting (QString const& name, bool& value, bool required):
	name (name),
	required (required),
	value_bool (&value)
{ }


inline
Module::NameAndSetting::NameAndSetting (QString const& name, int& value, bool required):
	name (name),
	required (required),
	value_int (&value)
{ }


inline
Module::NameAndSetting::NameAndSetting (QString const& name, float& value, bool required):
	name (name),
	required (required),
	value_float (&value)
{ }


inline
Module::NameAndSetting::NameAndSetting (QString const& name, double& value, bool required):
	name (name),
	required (required),
	value_double (&value)
{ }


inline
Module::NameAndSetting::NameAndSetting (QString const& name, std::string& value, bool required):
	name (name),
	required (required),
	value_string (&value)
{ }


inline
Module::NameAndSetting::NameAndSetting (QString const& name, QString& value, bool required):
	name (name),
	required (required),
	value_qstring (&value)
{ }


inline
Module::NameAndSetting::NameAndSetting (QString const& name, SI::Value& value, bool required):
	name (name),
	required (required),
	value_si_value (&value)
{ }


inline
Module::Registrator::Registrator (std::string const& module_name, FactoryFunction ff)
{
	Module::register_factory (module_name, ff);
}


inline
Module::Pointer::Pointer (std::string const& name, std::string const& instance):
	_name (name),
	_instance (instance)
{ }


inline bool
Module::Pointer::operator< (Pointer const& other) const
{
	if (_name != other._name)
		return _name < other._name;
	if (_instance != other._instance)
		return _instance < other._instance;
	return false;
}


inline std::string const&
Module::Pointer::name() const noexcept
{
	return _name;
}


inline std::string const&
Module::Pointer::instance() const noexcept
{
	return _instance;
}


inline
Module::~Module()
{ }


inline void
Module::data_updated()
{ }

} // namespace Xefis

#endif


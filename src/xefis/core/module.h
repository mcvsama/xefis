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

// Qt:
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/application/application.h>
#include <xefis/core/navaid_storage.h>
#include <xefis/core/module_manager.h>
#include <xefis/core/property.h>


#define XEFIS_REGISTER_MODULE_CLASS(module_name, klass) \
	static Xefis::Module::Registrator module_registrator (module_name, [](Xefis::ModuleManager* module_manager, QDomElement const& config) -> Xefis::Module* { \
		return new klass (module_manager, config); \
	});


namespace Xefis {

class Module
{
  public:
	struct NameAndProperty
	{
		QString			name;
		BaseProperty&	property;
		bool			required;
	};

	typedef std::vector<NameAndProperty>								PropertiesList;
	typedef std::function<Module* (ModuleManager*, QDomElement const&)>	FactoryFunction;
	typedef std::map<std::string, FactoryFunction>						FactoriesMap;

	struct Registrator
	{
		Registrator (std::string const& module_name, FactoryFunction);
	};

  public:
	// Ctor
	Module (ModuleManager*);

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
	 */
	void
	parse_properties (QDomElement const& properties_element, PropertiesList);

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

  private:
	/**
	 * Return list of factories.
	 */
	static FactoriesMap&
	factories();

  private:
	ModuleManager*	_module_manager = nullptr;
};


inline
Module::Registrator::Registrator (std::string const& module_name, FactoryFunction ff)
{
	Module::register_factory (module_name, ff);
}


inline
Module::~Module()
{ }


inline void
Module::data_updated()
{ }


inline Time
Module::update_time() const
{
	return _module_manager->update_time();
}


inline Time
Module::update_dt() const
{
	return _module_manager->update_dt();
}

} // namespace Xefis

#endif


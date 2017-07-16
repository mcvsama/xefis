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

#ifndef XEFIS__CORE__MODULE_MANAGER_H__INCLUDED
#define XEFIS__CORE__MODULE_MANAGER_H__INCLUDED

// Standard:
#include <cstddef>
#include <set>

// Qt:
#include <QtCore/QString>
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v1/module.h>


namespace xf {

class Module;
class Xefis;

class ModuleManager: public QObject
{
	class ModuleReloadRequest: public QEvent
	{
	  public:
		// Ctor
		explicit
		ModuleReloadRequest (Module::Pointer const&);

		Module::Pointer const&
		module_ptr() const noexcept;

	  private:
		Module::Pointer _module_ptr;
	};

	typedef std::set<Module*>					Modules;
	typedef std::set<Unique<Module>>			OwnedModules;

  public:
	typedef std::map<Module*, Module::Pointer>	ModuleToPointerMap;
	typedef std::map<Module::Pointer, Module*>	PointerToModuleMap;

  public:
	// Ctor
	explicit
	ModuleManager (Xefis*);

	// Dtor
	~ModuleManager();

	/**
	 * Access the Xefis object.
	 */
	Xefis*
	xefis() const;

	/**
	 * Add module by name.
	 * If parent is nullptr, widget will not be shown.
	 */
	Module*
	load_module (QString const& name, QString const& instnace, QDomElement const& config, QWidget* parent);

	/**
	 * Unload module by pointer.
	 */
	void
	unload_module (Module*);

	/**
	 * Signal that the data in property tree has been updated.
	 * Forward call to all loaded modules.
	 */
	void
	data_updated (Time time);

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
	 * Return Module::Pointer from Module*.
	 * \throw	ModuleNotFoundException if module can't be found.
	 */
	Module::Pointer
	find (Module*) const;

	/**
	 * Return Module* by Module::Pointer.
	 * May return nullptr.
	 */
	Module*
	find (Module::Pointer const&) const;

	/**
	 * Return list of loaded modules.
	 */
	PointerToModuleMap const&
	modules() const;

	/**
	 * Request module reload, that is destroy given instance and load it again
	 * with the same configuration.
	 */
	void
	post_module_reload_request (Module::Pointer const&);

  private:
	// QObject
	void
	customEvent (QEvent* event) override;

	/**
	 * Instantiate module by name.
	 */
	Module*
	create_module_by_name (QString const& name, QDomElement const& config, QWidget* parent);

	/**
	 * Call data_updated() on module, measure time it takes to process the call.
	 */
	void
	module_data_updated (Module*) const;

	/**
	 * Module reload.
	 */
	void
	do_module_reload_request (Module::Pointer const&);

	/**
	 * Try to call the rescue() method on a module, and catch exceptions.
	 * If exception is caught, it's described and not rethrown.
	 */
	void
	try_rescue (Module*) const;

  private:
	Logger				_logger;
	Xefis*				_xefis = nullptr;
	OwnedModules		_modules;
	Modules				_instrument_modules;
	Modules				_non_instrument_modules;
	Time				_update_time;
	Time				_update_dt;
	Time				_instrument_update_time;
	ModuleToPointerMap	_module_to_pointer_map;
	PointerToModuleMap	_pointer_to_module_map;
};


class ModuleNotFoundException: public Exception
{
  public:
	ModuleNotFoundException (std::string const& message):
		Exception (message)
	{ }
};


inline Xefis*
ModuleManager::xefis() const
{
	if (!_xefis)
		throw std::logic_error ("The Xefis object is not accessible from ModuleManager");
	return _xefis;
}


inline Time
ModuleManager::update_time() const
{
	return _update_time;
}


inline Time
ModuleManager::update_dt() const
{
	return _update_dt;
}

} // namespace xf

#endif


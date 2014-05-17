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

// Standard:
#include <cstddef>
#include <typeinfo>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/accounting.h>
#include <xefis/core/stdexcept.h>

// Local:
#include "module_manager.h"
#include "module.h"
#include "instrument.h"


namespace Xefis {

ModuleManager::ModuleManager (Application* application):
	_application (application)
{
	std::clog << "Creating ModuleManager" << std::endl;
}


ModuleManager::~ModuleManager()
{
	std::clog << "Destroying ModuleManager" << std::endl;
}


Module*
ModuleManager::load_module (QString const& name, QString const& instance, QDomElement const& config, QWidget* parent)
{
	Module::Pointer pointer (name.toStdString(), instance.toStdString());
	if (_pointer_to_module_map.find (pointer) != _pointer_to_module_map.end())
		throw BadConfiguration (QString ("module '%1' with instance name '%2' already loaded").arg (name).arg (instance).toStdString());

	Module* module = create_module_by_name (name, config, parent);

	// Sink into Unique:
	_modules.emplace (module);

	_module_to_pointer_map[module] = pointer;
	_pointer_to_module_map[pointer] = module;

	Instrument* instrument = dynamic_cast<Instrument*> (module);
	if (instrument)
		_instrument_modules.insert (instrument);
	else
		_non_instrument_modules.insert (module);

	if (_application->has_option (Application::Option::ModulesDebugLog))
		module->dump_debug_log();

	return module;
}


void
ModuleManager::data_updated (Time time)
{
	_update_dt = 1_s * (time.s() - _update_time.s());
	if (_update_dt > 1.0_s)
		_update_dt = Time::epoch() + 1_s;

	_update_time = time;

	// Process non-instrument modules:
	for (Module* mod: _non_instrument_modules)
		module_data_updated (mod);

	// Let instruments display data already computed by all other modules.
	// Also limit FPS of the instrument modules.
	if (time - _instrument_update_time > 1_s / 30.f)
	{
		for (Module* mod: _instrument_modules)
			module_data_updated (mod);
		_instrument_update_time = time;
	}
}


Module::Pointer
ModuleManager::find (Module* module) const
{
	ModuleToPointerMap::const_iterator m = _module_to_pointer_map.find (module);
	if (m != _module_to_pointer_map.end())
		return m->second;
	throw ModuleNotFoundException ("module specified by pointer (Module*) can't be found");
}


Module*
ModuleManager::find (Module::Pointer const& modptr) const
{
	PointerToModuleMap::const_iterator m = _pointer_to_module_map.find (modptr);
	if (m != _pointer_to_module_map.end())
		return m->second;
	return nullptr;
}


ModuleManager::PointerToModuleMap const&
ModuleManager::modules() const
{
	return _pointer_to_module_map;
}


void
ModuleManager::post_module_reload_request (Module*)
{
	// TODO
}


Module*
ModuleManager::create_module_by_name (QString const& name, QDomElement const& config, QWidget* parent)
{
	try {
		Module::FactoryFunction ff = Module::find_factory (name.toStdString());
		if (ff)
		{
			Module* module = ff (this, config);
			QWidget* widget = dynamic_cast<QWidget*> (module);
			if (widget)
				widget->setParent (parent);
			return module;
		}
		else
			throw ModuleNotFoundException ("module not found: " + name.toStdString());
	}
	catch (Exception& e)
	{
		throw Exception ("error when loading module "_str + name.toStdString(), &e);
	}
}


void
ModuleManager::module_data_updated (Module* module) const
{
	Module::Pointer modptr = find (module);

	Time dt = Time::measure ([&] {
		try {
			module->data_updated();
		}
		catch (Xefis::Exception const& e)
		{
			std::cerr << "Exception when processing update from module '" << typeid (*module).name() << "'" << std::endl;
			std::cerr << e << std::endl;
			try_rescue (module);
		}
		catch (std::exception const& e)
		{
			std::cerr << "Caught std::exception when processing update from module '" << typeid (*module).name() << "'" << std::endl;
			std::cerr << "Message: " << e.what() << std::endl;
			try_rescue (module);
		}
		catch (...)
		{
			std::cerr << "Unknown exception when processing update from module '" << typeid (*module).name() << "'" << std::endl;
			try_rescue (module);
		}
	});

	_application->accounting()->add_module_stats (modptr, dt);
}


void
ModuleManager::try_rescue (Module* module) const
{
	try {
		module->rescue();
	}
	catch (Xefis::Exception const& e)
	{
		std::cerr << "Exception when rescuing module '" << typeid (*module).name() << "'; inhibiting from further actions." << std::endl;
		std::cerr << e << std::endl;
	}
	catch (std::exception const& e)
	{
		std::cerr << "Caught std::exception when rescuing module '" << typeid (*module).name() << "'; inhibiting from further actions." << std::endl;
		std::cerr << "Message: " << e.what() << std::endl;
	}
	catch (...)
	{
		std::cerr << "Unknown exception when rescuing module '" << typeid (*module).name() << "'; inhibiting from further actions." << std::endl;
	}
}

} // namespace Xefis


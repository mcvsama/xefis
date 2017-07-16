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
#include <typeinfo>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v1/module.h>
#include <xefis/core/xefis.h>
#include <xefis/core/accounting.h>
#include <xefis/core/stdexcept.h>
#include <xefis/utility/time_helper.h>
#include <xefis/utility/demangle.h>

// Local:
#include "module_manager.h"
#include "module.h"
#include "instrument.h"
#include "window.h"


namespace xf {

ModuleManager::ModuleReloadRequest::ModuleReloadRequest (Module::Pointer const& module_ptr):
	QEvent (QEvent::User),
	_module_ptr (module_ptr)
{ }


Module::Pointer const&
ModuleManager::ModuleReloadRequest::module_ptr() const noexcept
{
	return _module_ptr;
}


ModuleManager::ModuleManager (Xefis* xefis):
	_xefis (xefis)
{
	_logger.set_prefix ("<module manager>");
	_logger << "Creating ModuleManager" << std::endl;
}


ModuleManager::~ModuleManager()
{
	_logger << "Destroying ModuleManager" << std::endl;
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

	if (_xefis->has_option (Xefis::Option::ModulesDebugLog))
		module->dump_debug_log();

	return module;
}


void
ModuleManager::unload_module (Module* module)
{
	for (auto const& umod: _modules)
	{
		if (umod.get() == module)
		{
			// Remove the module:
			_instrument_modules.erase (module);
			_non_instrument_modules.erase (module);

			Module::Pointer ptr = _module_to_pointer_map[module];
			_module_to_pointer_map.erase (module);
			_pointer_to_module_map.erase (ptr);

			_modules.erase (umod);
			break;
		}
	}
}


void
ModuleManager::data_updated (Time time)
{
	_update_dt = time - _update_time;
	if (_update_dt > 1.0_s)
		_update_dt = TimeHelper::epoch() + 1_s;

	_update_time = time;

	// Process non-instrument modules:
	for (Module* mod: _non_instrument_modules)
		module_data_updated (mod);

	// Let instruments display data already computed by all other modules.
	// Also limit FPS of the instrument modules.
	if (time - _instrument_update_time > 1_s / 30.0)
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
ModuleManager::post_module_reload_request (Module::Pointer const& module_ptr)
{
	QApplication::postEvent (this, new ModuleReloadRequest (module_ptr));
}


void
ModuleManager::customEvent (QEvent* event)
{
	auto mrr = dynamic_cast<ModuleReloadRequest*> (event);
	if (mrr)
		do_module_reload_request (mrr->module_ptr());
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

	Time dt = TimeHelper::measure ([&] {
		try {
			module->data_updated();
		}
		catch (xf::Exception const& e)
		{
			std::cerr << "Exception when processing update from module '" << demangle (typeid (*module)) << "'" << std::endl;
			std::cerr << e << std::endl;
			try_rescue (module);
		}
		catch (std::exception const& e)
		{
			std::cerr << "Caught std::exception when processing update from module '" << demangle (typeid (*module)) << "'" << std::endl;
			std::cerr << "Message: " << e.what() << std::endl;
			try_rescue (module);
		}
		catch (...)
		{
			std::cerr << "Unknown exception when processing update from module '" << demangle (typeid (*module)) << "'" << std::endl;
			try_rescue (module);
		}
	});

	_xefis->accounting()->add_module_stats (modptr, dt);
}


void
ModuleManager::do_module_reload_request (Module::Pointer const& module_ptr)
{
	Module* module = find (module_ptr);
	if (module)
	{
		_logger << "ModuleManager: restarting module " << module_ptr << "." << std::endl;
		// If this is instrument module, we need to access its window,
		// then get the decorator widget.
		Window::InstrumentDecorator* decorator = nullptr;
		Instrument* instrument = dynamic_cast<Instrument*> (module);
		if (instrument)
		{
			QWidget* module_window = instrument->window();
			if (module_window)
			{
				Window* window = dynamic_cast<Window*> (module_window);
				if (window)
					decorator = window->get_decorator_for (instrument->get_pointer());
			}
		}

		QString name = QString::fromStdString (module->name());
		QString instance = QString::fromStdString (module->instance());

		unload_module (module);

		ConfigReader* config_reader = _xefis->config_reader();
		if (config_reader)
		{
			Exception::guard ([&] {
				QDomElement module_config = config_reader->module_config (name, instance);
				Module* new_module = load_module (name, instance, module_config, decorator);
				if (decorator)
				{
					Instrument* new_instrument = dynamic_cast<Instrument*> (new_module);
					if (new_instrument)
						decorator->set_instrument (new_instrument);
				}
			});
		}
	}
	else
		_logger << "ModuleManager: couldn't find module " << module_ptr << " to restart." << std::endl;
}


void
ModuleManager::try_rescue (Module* module) const
{
	try {
		module->rescue();
	}
	catch (xf::Exception const& e)
	{
		std::cerr << "Exception when rescuing module '" << demangle (typeid (*module)) << "'; inhibiting from further actions." << std::endl;
		std::cerr << e << std::endl;
	}
	catch (std::exception const& e)
	{
		std::cerr << "Caught std::exception when rescuing module '" << demangle (typeid (*module)) << "'; inhibiting from further actions." << std::endl;
		std::cerr << "Message: " << e.what() << std::endl;
	}
	catch (...)
	{
		std::cerr << "Unknown exception when rescuing module '" << demangle (typeid (*module)) << "'; inhibiting from further actions." << std::endl;
	}
}

} // namespace xf


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

// Local:
#include "module.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/sockets/module_socket.h>
#include <xefis/core/setting.h>

// Neutrino:
#include <neutrino/demangle.h>
#include <neutrino/exception_support.h>

// Standard:
#include <cstddef>
#include <algorithm>


namespace xf {

UninitializedSettings::UninitializedSettings (std::vector<BasicSetting*> settings):
	FastException (make_message (settings))
{ }


std::string
UninitializedSettings::make_message (std::vector<BasicSetting*> settings)
{
	if (settings.empty())
		return "uninitialized settings in a module";
	else
	{
		std::string result = "uninitialized setting(s) found for module " + identifier (*settings[0]->module()) + ": ";

		for (std::size_t i = 0; i < settings.size(); ++i)
		{
			result += settings[i]->name();

			if (i != settings.size() - 1)
				result += ", ";
		}

		return result;
	}
}


void
Module::ModuleSocketAPI::verify_settings()
{
	std::vector<BasicSetting*> uninitialized_settings;

	for (auto* setting: _module._registered_settings)
	{
		if (setting->required() && !*setting)
			uninitialized_settings.push_back (setting);
	}

	if (!uninitialized_settings.empty())
		throw UninitializedSettings (uninitialized_settings);

	_module.verify_settings();
}


void
Module::ModuleSocketAPI::register_setting (BasicSetting& setting)
{
	_module._registered_settings.push_back (&setting);
}


void
Module::ModuleSocketAPI::register_input_socket (BasicModuleIn& socket)
{
	_module._registered_input_sockets.push_back (&socket);
}


void
Module::ModuleSocketAPI::unregister_input_socket (BasicModuleIn& socket)
{
	auto new_end = std::remove (_module._registered_input_sockets.begin(), _module._registered_input_sockets.end(), &socket);
	_module._registered_input_sockets.resize (neutrino::to_unsigned (std::distance (_module._registered_input_sockets.begin(), new_end)));
}


void
Module::ModuleSocketAPI::register_output_socket (BasicModuleOut& socket)
{
	_module._registered_output_sockets.push_back (&socket);
}


void
Module::ModuleSocketAPI::unregister_output_socket (BasicModuleOut& socket)
{
	auto new_end = std::remove (_module._registered_output_sockets.begin(), _module._registered_output_sockets.end(), &socket);
	_module._registered_output_sockets.resize (neutrino::to_unsigned (std::distance (_module._registered_output_sockets.begin(), new_end)));
}


Sequence<std::vector<BasicSetting*>::const_iterator>
Module::ModuleSocketAPI::settings() const noexcept
{
	return { _module._registered_settings.begin(), _module._registered_settings.end() };
}


Sequence<std::vector<BasicModuleIn*>::const_iterator>
Module::ModuleSocketAPI::input_sockets() const noexcept
{
	return { _module._registered_input_sockets.begin(), _module._registered_input_sockets.end() };
}


Sequence<std::vector<BasicModuleOut*>::const_iterator>
Module::ModuleSocketAPI::output_sockets() const noexcept
{
	return { _module._registered_output_sockets.begin(), _module._registered_output_sockets.end() };
}


void
Module::ProcessingLoopAPI::communicate (Cycle const& cycle)
{
	try {
		auto communication_time = TimeHelper::measure ([&] {
			_module.communicate (cycle);
		});

		if (implements_communicate_method())
			Module::AccountingAPI (_module).add_communication_time (communication_time);
	}
	catch (...)
	{
		handle_exception (cycle, "communicate()");
	}
}


void
Module::ProcessingLoopAPI::fetch_and_process (Cycle const& cycle)
{
	try {
		if (!_module._cached)
		{
			_module._cached = true;

			for (auto* socket: _module._registered_input_sockets)
				socket->fetch (cycle);

			auto processing_time = TimeHelper::measure ([&] {
				_module.process (cycle);
			});

			if (implements_process_method())
				Module::AccountingAPI (_module).add_processing_time (processing_time);
		}
	}
	catch (...)
	{
		handle_exception (cycle, "process()");
	}
}


void
Module::ProcessingLoopAPI::handle_exception (Cycle const& cycle, std::string_view const& context_info)
{
	try {
		_module.rescue (cycle, std::current_exception());

		// Set all output sockets to nil.
		if (_module._set_nil_on_exception)
			for (auto* socket: _module._registered_output_sockets)
				*socket = xf::nil;
	}
	catch (...)
	{
		cycle.logger() << "Exception (" << context_info << ") '" << xf::describe_exception (std::current_exception()) << "' during handling exception from module " << identifier (_module) << "\n";
	}
}


Module::Module (std::string_view const& instance):
	NamedInstance (instance)
{
	ModuleSocketAPI (*this).verify_settings();
}


Module::~Module()
{
	// Operate on a copy since the deregister() method modifies vectors in this class.
	for (auto* socket: clone (_registered_input_sockets))
		socket->deregister();

	// Operate on copies since the deregister() method modifies vectors in this class.
	for (auto* socket: clone (_registered_output_sockets))
		socket->deregister();
}


void
Module::initialize()
{ }


void
Module::communicate (xf::Cycle const&)
{
	_did_not_communicate = true;
}


void
Module::process (xf::Cycle const&)
{
	_did_not_process = true;
}


void
Module::rescue (Cycle const& cycle, std::exception_ptr eptr)
{
	cycle.logger() << "Unhandled exception '" << xf::describe_exception (eptr) << "' during processing of module " << identifier (*this) << "\n";
}


std::string
identifier (Module const& module)
{
	auto s = demangle (typeid (module));
	return s.substr (0, s.find ("<")) + "#" + module.instance();
}


std::string
identifier (Module const* module)
{
	return module ? identifier (*module) : "(nullptr)";
}

} // namespace xf


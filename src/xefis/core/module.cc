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
#include <algorithm>

// Neutrino:
#include <neutrino/demangle.h>
#include <neutrino/exception_support.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/sockets/module_socket.h>
#include <xefis/core/setting.h>

// Local:
#include "module.h"


namespace xf {

void
BasicModule::ProcessingLoopAPI::communicate (Cycle const& cycle)
{
	try {
		auto communication_time = TimeHelper::measure ([&] {
			_module.communicate (cycle);
		});

		if (implements_communicate_method())
			BasicModule::AccountingAPI (_module).add_communication_time (communication_time);
	}
	catch (...)
	{
		handle_exception (cycle, "communicate()");
	}
}


void
BasicModule::ProcessingLoopAPI::fetch_and_process (Cycle const& cycle)
{
	try {
		if (!_module._cached)
		{
			_module._cached = true;

			for (auto* socket: _module.io_base()->_registered_input_sockets)
				socket->fetch (cycle);

			auto processing_time = TimeHelper::measure ([&] {
				_module.process (cycle);
			});

			if (implements_process_method())
				BasicModule::AccountingAPI (_module).add_processing_time (processing_time);
		}
	}
	catch (...)
	{
		handle_exception (cycle, "process()");
	}
}


void
BasicModule::ProcessingLoopAPI::handle_exception (Cycle const& cycle, std::string_view const& context_info)
{
	try {
		_module.rescue (cycle, std::current_exception());

		// Set all output sockets to nil.
		if (_module._set_nil_on_exception)
			for (auto* socket: _module._io->_registered_output_sockets)
				*socket = xf::nil;
	}
	catch (...)
	{
		cycle.logger() << "Exception (" << context_info << ") '" << xf::describe_exception (std::current_exception()) << "' during handling exception from module " << identifier (_module) << "\n";
	}
}


BasicModule::BasicModule (std::unique_ptr<ModuleIO> io, std::string_view const& instance):
	NamedInstance (instance),
	_io (std::move (io))
{
	auto api = ModuleIO::ProcessingLoopAPI (*_io.get());
	api.set_module (*this);
	api.verify_settings();
}


void
BasicModule::initialize()
{ }


void
BasicModule::communicate (xf::Cycle const&)
{
	_did_not_communicate = true;
}


void
BasicModule::process (xf::Cycle const&)
{
	_did_not_process = true;
}


void
BasicModule::rescue (Cycle const& cycle, std::exception_ptr eptr)
{
	cycle.logger() << "Unhandled exception '" << xf::describe_exception (eptr) << "' during processing of module " << identifier (*this) << "\n";
}


std::string
identifier (BasicModule const& module)
{
	auto s = demangle (typeid (module));
	return s.substr (0, s.find ("<")) + "#" + module.instance();
}


std::string
identifier (BasicModule const* module)
{
	return module ? identifier (*module) : "(nullptr)";
}

} // namespace xf


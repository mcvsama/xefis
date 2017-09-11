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

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/property.h>
#include <xefis/core/v2/setting.h>
#include <xefis/utility/demangle.h>

// Local:
#include "module.h"


namespace v2 {

void
BasicModule::ProcessingLoopAPI::fetch_and_process (Cycle const& cycle)
{
	if (!_module->_cached)
	{
		_module->_cached = true;

		for (auto* prop: _module->io_base()->_registered_input_properties)
			prop->fetch (cycle);

		_module->process (cycle);
	}
}


BasicModule::BasicModule (std::unique_ptr<ModuleIO> io, std::string const& instance):
	_instance (instance),
	_io (std::move (io))
{
	auto api = ModuleIO::ProcessingLoopAPI (_io.get());
	api.set_module (this);
	api.verify_settings();
}


void
BasicModule::initialize()
{ }


void
BasicModule::process (v2::Cycle const&)
{ }


void
BasicModule::rescue (std::exception_ptr)
{
	// TODO log the exception

	// TODO the following needs to be OPT-IN!
	// Set all output properties to nil.
	for (auto* property: _io->_registered_output_properties)
		property->set_nil();
}


std::string
identifier (BasicModule& module)
{
	return demangle (typeid (module)) + "#" + module.instance();
}


std::string
identifier (BasicModule* module)
{
	return module ? identifier (*module) : "(nullptr)";
}

} // namespace v2


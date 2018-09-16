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
#include <xefis/core/property.h>
#include <xefis/core/setting.h>
#include <xefis/utility/demangle.h>
#include <xefis/utility/exception_support.h>

// Local:
#include "module.h"


namespace xf {

void
BasicModule::ProcessingLoopAPI::fetch_and_process (Cycle const& cycle)
{
	try {
		if (!_module._cached)
		{
			_module._cached = true;

			for (auto* prop: _module.io_base()->_registered_input_properties)
				prop->fetch (cycle);

			auto processing_time = TimeHelper::measure ([&] {
				_module.process (cycle);
			});

			AccountingAPI (_module).add_processing_time (processing_time);
		}
	}
	catch (...)
	{
		try {
			_module.rescue (cycle, std::current_exception());

			// Set all output properties to nil.
			if (_module._set_nil_on_exception)
				for (auto* property: _module._io->_registered_output_properties)
					*property = xf::nil;
		}
		catch (...)
		{
			cycle.logger() << "Exception '" << xf::describe_exception (std::current_exception()) << "' during handling exception from module " << identifier (_module) << "\n";
		}
	}
}


BasicModule::BasicModule (std::unique_ptr<ModuleIO> io, std::string_view const& instance):
	_instance (instance),
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
BasicModule::process (xf::Cycle const&)
{ }


void
BasicModule::rescue (Cycle const& cycle, std::exception_ptr eptr)
{
	cycle.logger() << "Unhandled exception '" << xf::describe_exception (eptr) << "' during processing of module " << identifier (*this) << "\n";
}


std::string
identifier (BasicModule const& module)
{
	return demangle (typeid (module)) + "#" + module.instance();
}


std::string
identifier (BasicModule const* module)
{
	return module ? identifier (*module) : "(nullptr)";
}

} // namespace xf


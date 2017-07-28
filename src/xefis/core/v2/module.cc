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

UninitializedSettings::UninitializedSettings (std::vector<BasicSetting*> settings):
	Exception (make_message (settings))
{ }


std::string
UninitializedSettings::make_message (std::vector<BasicSetting*> settings)
{
	if (settings.empty())
		return "uninitialized settings in a module";
	else
	{
		std::string result = "uninitialized setting(s) found for module " + identifier (settings[0]->io()) + ": ";

		for (std::size_t i = 0; i < settings.size(); ++i)
		{
			result += demangle (typeid (*settings[i]));

			if (i != settings.size() - 1)
				result += ", ";
		}

		return result;
	}
}


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
	_io->set_module (this);
	// TODO verify settings etc
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
identifier (ModuleIO& io)
{
	BasicModule* module = io.module();

	if (module)
		return identifier (*module);
	else
		return "<no module associated with the IO object>";
}


std::string
identifier (BasicModule* module)
{
	return module ? identifier (*module) : "(nullptr)";
}

std::string
identifier (ModuleIO* io)
{
	return io ? identifier (*io) : "(nullptr)";
}

} // namespace v2


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

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/property.h>
#include <xefis/core/v2/setting.h>
#include <xefis/utility/demangle.h>

// Local:
#include "module.h"


namespace x2 {

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
		std::string result = "uninitialized setting(s) found in module " + module_identifier (settings[0]->owner()) + ": ";

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
Module::ProcessingLoopAPI::verify_settings()
{
	std::vector<BasicSetting*> uninitialized_settings;

	for (auto* setting: _module->_registered_settings)
		if (!*setting)
			uninitialized_settings.push_back (setting);

	if (!uninitialized_settings.empty())
		throw UninitializedSettings (uninitialized_settings);
}


void
Module::ProcessingLoopAPI::fetch_and_process (Cycle const& cycle)
{
	if (!_module->_cached)
	{
		_module->_cached = true;

		for (auto* prop: _module->_registered_input_properties)
			prop->fetch (cycle);

		_module->process (cycle);
	}
}


void
Module::ProcessingLoopAPI::register_input_property (BasicPropertyIn* property)
{
	_module->_registered_input_properties.push_back (property);
}


void
Module::ProcessingLoopAPI::unregister_input_property (BasicPropertyIn* property)
{
	auto new_end = std::remove (_module->_registered_input_properties.begin(), _module->_registered_input_properties.end(), property);
	_module->_registered_input_properties.resize (new_end - _module->_registered_input_properties.begin());
}


void
Module::ProcessingLoopAPI::register_output_property (BasicPropertyOut* property)
{
	_module->_registered_output_properties.push_back (property);
}


void
Module::ProcessingLoopAPI::unregister_output_property (BasicPropertyOut* property)
{
	auto new_end = std::remove (_module->_registered_output_properties.begin(), _module->_registered_output_properties.end(), property);
	_module->_registered_output_properties.resize (new_end - _module->_registered_output_properties.begin());
}


Module::Module (std::string const& instance):
	_instance (instance)
{
	_logger.set_prefix ((boost::format ("[%-30s#%-20s]") % demangle (typeid (this).name()) % _instance).str());
}


void
Module::initialize()
{ }


void
Module::process (x2::Cycle const&)
{ }


void
Module::rescue (std::exception_ptr)
{
	// TODO log the exception

	// TODO the following needs to be OPT-IN!
	// Set all output properties to nil.
	for (auto* property: _registered_output_properties)
		property->set_nil();
}


std::string
module_identifier (Module& module)
{
	return demangle (typeid (module)) + "#" + module.instance();
}


std::string
module_identifier (Module* module)
{
	if (module)
		return module_identifier (*module);

	return "(nullptr)";
}

} // namespace x2


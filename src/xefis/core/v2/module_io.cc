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

namespace module_io {

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
		std::string result = "uninitialized setting(s) found for module-io " + identifier (*settings[0]->io()) + ": ";

		for (std::size_t i = 0; i < settings.size(); ++i)
		{
			result += settings[i]->name();

			if (i != settings.size() - 1)
				result += ", ";
		}

		return result;
	}
}

} // namespace module_io


void
ModuleIO::ProcessingLoopAPI::verify_settings()
{
	std::vector<BasicSetting*> uninitialized_settings;

	for (auto* setting: _io->_registered_settings)
	{
		if (!*setting)
			uninitialized_settings.push_back (setting);
	}

	if (!uninitialized_settings.empty())
		throw module_io::UninitializedSettings (uninitialized_settings);

	_io->verify_settings();
}


void
ModuleIO::ProcessingLoopAPI::register_setting (BasicSetting* setting)
{
	_io->_registered_settings.push_back (setting);
}


void
ModuleIO::ProcessingLoopAPI::register_input_property (BasicPropertyIn* property)
{
	_io->_registered_input_properties.push_back (property);
}


void
ModuleIO::ProcessingLoopAPI::unregister_input_property (BasicPropertyIn* property)
{
	auto new_end = std::remove (_io->_registered_input_properties.begin(), _io->_registered_input_properties.end(), property);
	_io->_registered_input_properties.resize (new_end - _io->_registered_input_properties.begin());
}


void
ModuleIO::ProcessingLoopAPI::register_output_property (BasicPropertyOut* property)
{
	_io->_registered_output_properties.push_back (property);
}


void
ModuleIO::ProcessingLoopAPI::unregister_output_property (BasicPropertyOut* property)
{
	auto new_end = std::remove (_io->_registered_output_properties.begin(), _io->_registered_output_properties.end(), property);
	_io->_registered_output_properties.resize (new_end - _io->_registered_output_properties.begin());
}


std::string
identifier (ModuleIO& io)
{
	BasicModule* module = io.module();
	std::string module_name = module ? identifier (*module) : "<no module associated with the IO object>";

	return demangle (typeid (io)) + " of " + module_name;
}


std::string
identifier (ModuleIO* io)
{
	return io ? identifier (*io) : "(nullptr)";
}

} // namespace v2


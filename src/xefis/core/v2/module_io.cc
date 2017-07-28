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
ModuleIO::ProcessingLoopAPI::verify_settings()
{
	std::vector<BasicSetting*> uninitialized_settings;

	for (auto* setting: _io->_registered_settings)
		if (!*setting)
			uninitialized_settings.push_back (setting);

	if (!uninitialized_settings.empty())
		throw UninitializedSettings (uninitialized_settings);
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

} // namespace v2


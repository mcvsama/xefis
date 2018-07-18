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

// Local:
#include "property_string_converter.h"


namespace xf {

PropertyStringConverter::StringConverter::StringConverter (Property<std::string>& property, PropertyConversionSettings const& settings):
	BasicConverter (settings),
	_property (property)
{ }


std::string
PropertyStringConverter::StringConverter::to_string() const
{
	return _property ? *_property : settings().nil_value;
}


void
PropertyStringConverter::StringConverter::from_string (std::string const& s)
{
	// TODO _property << ConstantSource (s);
}


PropertyStringConverter::BoolConverter::BoolConverter (Property<bool>& property, PropertyConversionSettings const& settings):
	BasicConverter (settings),
	_property (property)
{ }


std::string
PropertyStringConverter::BoolConverter::to_string() const
{
	if (_property)
		return *_property ? settings().true_value : settings().false_value;
	else
		return settings().nil_value;
}

void
PropertyStringConverter::BoolConverter::from_string (std::string const& s)
{
	// TODO _property << ConstantSource (s == _true_value);
}

} // namespace xf


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

// Local:
#include "si_config.h"
#include "exception.h"
#include "unit.h"


namespace si {

IncompatibleTypes::IncompatibleTypes (DynamicUnit const& got, DynamicUnit const& expected):
	Exception ("incompatible types; expected '" + expected.symbol() + "', got '" + got.symbol() + "'")
{ }


UnparsableValue::UnparsableValue (std::string const& message):
	Exception (message)
{ }


UnsupportedUnit::UnsupportedUnit (std::string const& message):
	Exception (message)
{ }

} // namespace si


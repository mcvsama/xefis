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
#include <xefis/utility/mutex.h>

// Local:
#include "property_utils.h"


namespace v1 {
using namespace xf;

static xf::Mutex $check_validity_entry_mutex;


std::string const&
PropertyType::check_validity (std::string const& type)
{
	// Must acquire lock before statically- and non-statically initializing static variables:
	auto lock = $check_validity_entry_mutex.acquire_lock();

	static std::set<std::string> valid_types = {
		"boolean",
		"integer",
		"float",
		"string",
		// SI types:
		// TODO support all types from standard_quantities.h
		"acceleration",
		"angle",
		"area",
		"charge",
		"current",
		"density",
		"force",
		"frequency",
		"angular-velocity",
		"length",
		"pressure",
		"speed",
		"time",
		"torque",
		"volume",
		"mass",
		"temperature",
	};

	if (valid_types.find (type) == valid_types.end())
		throw BadType (type);

	return type;
}

} // namespace v1


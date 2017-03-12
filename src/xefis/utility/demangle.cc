/* vim:ts=4
 *
 * Copyleft 2008…2013  Michał Gawron
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
#include <memory>
#include <cxxabi.h>

// Local:
#include "demangle.h"


namespace xf {

std::string
demangle (std::string mangled_cxx_name)
{
	int demangle_status = 0;
	std::size_t demangled_max_size = 256;
	std::unique_ptr<char, decltype(&::free)> demangled_name {
		abi::__cxa_demangle (mangled_cxx_name.c_str(), nullptr, &demangled_max_size, &demangle_status),
		std::free
	};

	if (demangle_status == 0)
		return std::string (demangled_name.get());
	else
		return mangled_cxx_name;
}


std::string
demangle (std::type_info const& type_info)
{
	return demangle (type_info.name());
}

} // namespace xf


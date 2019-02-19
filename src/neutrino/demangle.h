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

#ifndef NEUTRINO__DEMANGLE_H__INCLUDED
#define NEUTRINO__DEMANGLE_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>
#include <typeinfo>


namespace neutrino {

std::string
demangle (std::string mangled_cxx_name);


std::string
demangle (std::type_info const& type_info);

} // namespace neutrino

#endif


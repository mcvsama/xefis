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

#ifndef NEUTRINO__EXCEPTION_SUPPORT_H__INCLUDED
#define NEUTRINO__EXCEPTION_SUPPORT_H__INCLUDED

// Standard:
#include <cstddef>
#include <exception>
#include <functional>
#include <optional>


namespace neutrino {

/**
 * Describe exception.
 */
extern std::string
describe_exception (std::exception_ptr);

/**
 * Describe boost::format exceptions if provided block throws.
 */
extern std::optional<std::string>
handle_format_exception (std::function<void()> try_block);

} // namespace neutrino

#endif


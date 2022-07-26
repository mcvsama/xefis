/* vim:ts=4
 *
 * Copyleft 2020  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__UTILITY__IS_OPTIONAL_H__INCLUDED
#define XEFIS__UTILITY__IS_OPTIONAL_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>
#include <type_traits>


namespace xf {

template<class>
	struct is_optional: public std::false_type
	{ };


template<class OptionalValue>
	struct is_optional<std::optional<OptionalValue>>: public std::true_type
	{ };


template<class T>
	static constexpr bool is_optional_v = is_optional<T>::value;

} // namespace xf

#endif


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

#ifndef XEFIS__UTILITY__UTILITY_H__INCLUDED
#define XEFIS__UTILITY__UTILITY_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>


namespace xf {

template<class Value, class ...Rest>
	Value*
	coalesce (Value* value, Rest* ...rest)
	{
		if (value)
			return value;
		else if (sizeof...(rest) > 0)
			return coalesce (rest...);
		else
			return nullptr;
	}


/**
 * Create a copy of an argument.
 */
template<class T>
	inline T
	clone (T t)
	{
		return T (t);
	}

} // namespace xf

#endif


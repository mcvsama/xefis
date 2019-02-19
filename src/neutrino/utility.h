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

#ifndef NEUTRINO__UTILITY_H__INCLUDED
#define NEUTRINO__UTILITY_H__INCLUDED

// Standard:
#include <cstddef>
#include <future>


namespace neutrino {

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


/**
 * Return true if a std::future is ready.
 */
template<typename R>
	inline bool
	is_ready (std::future<R> const& future)
	{
		return future.wait_for (std::chrono::seconds (0)) == std::future_status::ready;
	}

} // namespace neutrino

#endif


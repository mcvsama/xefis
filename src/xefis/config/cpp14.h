/* vim:ts=4
 *
 * Copyleft 2012…2014  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__CONFIG__CPP14_H__INCLUDED
#define XEFIS__CONFIG__CPP14_H__INCLUDED

#include <memory>
#include <type_traits>
#include <utility>


namespace std {

template<class T, class... Args>
	static inline std::unique_ptr<T>
	make_unique_helper (std::false_type, Args&&... args)
	{
		return std::unique_ptr<T> (new T (std::forward<Args> (args)...));
	}


template<class T, class... Args>
	static inline std::unique_ptr<T>
	make_unique_helper (std::true_type, Args&&... args)
	{
		static_assert (std::extent<T>::value == 0,
					   "make_unique<T[N]>() is forbidden, please use make_unique<T[]>().");

		typedef typename std::remove_extent<T>::type U;
		return std::unique_ptr<T> (new U[sizeof...(Args)]{ std::forward<Args>(args)... });
	}


template<class T, class... Args>
	std::unique_ptr<T> make_unique (Args&&... args)
	{
		return make_unique_helper<T> (std::is_array<T>(), std::forward<Args>(args)...);
	}

} // namespace std

#endif


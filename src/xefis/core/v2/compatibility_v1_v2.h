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

#ifndef XEFIS__CORE__V2__COMPATIBILITY_V1_V2_H__INCLUDED
#define XEFIS__CORE__V2__COMPATIBILITY_V1_V2_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/property.h>


namespace xf {

extern std::list<std::function<void()>>	g_copy_to_v1;
extern std::list<std::function<void()>>	g_copy_to_v2;


template<class T>
	void
	operator<< (Property<T> target, x2::PropertyOut<T>& source)
	{
		auto source_ptr = &source;

		g_copy_to_v1.push_back ([=]() {
			std::cout << "COPYING v2 >> v1: ";
			if (*source_ptr)
				std::cout << *source_ptr;
			else
				std::cout << "(nil)";
			std::cout << "\n";
			target = source_ptr->get_optional();
		});
	}


template<class T>
	void
	operator<< (x2::PropertyIn<T>& target, Property<T> source)
	{
		auto target_ptr = &target;

		g_copy_to_v2.push_back ([=]() {
			std::cout << "COPYING v1 >> v2: ";
			if (source.valid())
				std::cout << *source;
			else
				std::cout << "(nil)";
			std::cout << "\n";
			*target_ptr = source.get_optional();
		});
	}

} // namespace xf

#endif


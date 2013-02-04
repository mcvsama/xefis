/* vim:ts=4
 *
 * Copyleft 2012…2013  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 * --
 * Here be basic, global functions and macros like asserts, debugging helpers, etc.
 */

#ifndef XEFIS__CONFIG__SYSTEM_H__INCLUDED
#define XEFIS__CONFIG__SYSTEM_H__INCLUDED

// Standard:
#include <cstdio>
#include <iostream>

/**
 * Return size (number of elements) of an array.
 */
template<class T, std::size_t N>
	inline constexpr std::size_t
	countof (T(&)[N])
	{
		return N;
	}


/**
 * Return size of an array. Can be used in const expressions.
 */
template<class T, std::size_t N>
	inline constexpr const char (&sizer (T (&)[N]))[N];


inline void
assert_function (bool expression, const char* message = nullptr) noexcept
{
	if (!expression)
	{
		if (message)
			std::clog << "Assertion failed: " << message << std::endl;
# if XEFIS_ENABLE_FATAL_ASSERT
		raise (SIGTRAP);
# endif
	}
}


/**
 * Prints debug output.
 */
#define xdebug(x...) fprintf (stderr, x)

#endif


/**
 * Packed structs.
 */
#define BEGIN_PACKED_STRUCT
#define END_PACKED_STRUCT __attribute__((packed));
/**
 * Since most of standard headers override our assert, ensure
 * that it's redefined every possible time, when this
 * header is included.
 */

#undef assert
#if XEFIS_ENABLE_ASSERT
# include <signal.h>
# undef assert
# define assert assert_function
#else // XEFIS_ENABLE_ASSERT
# undef assert
# define assert(a, b...)
#endif // XEFIS_ENABLE_ASSERT


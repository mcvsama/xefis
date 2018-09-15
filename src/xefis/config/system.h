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
 * --
 * Here be basic, global functions and macros like asserts, debugging helpers, etc.
 */

#ifndef XEFIS__CONFIG__SYSTEM_H__INCLUDED
#define XEFIS__CONFIG__SYSTEM_H__INCLUDED

// Standard:
#include <cstdio>
#include <iostream>

// System:
#include <signal.h>


// Fixes for std::ostream which has broken support for unsigned/signed/char types
// and prints 8-bit integers like they were characters.
namespace ostream_fixes {

inline std::ostream&
operator<< (std::ostream& os, unsigned char i)
{
	return os << static_cast<unsigned int> (i);
}


inline std::ostream&
operator<< (std::ostream& os, signed char i)
{
	return os << static_cast<signed int> (i);
}

} // namespace ostream_fixes


// Add std::basic_string_view + std::basic_string operators.
namespace string_view_plus_string_fixes {

template<class CharT, class Traits, class Allocator>
	inline std::basic_string<CharT, Traits, Allocator>
	operator+ (std::basic_string<CharT, Traits, Allocator> const& s,
			   std::basic_string_view<CharT, Traits> const& sv)
	{
		return std::basic_string<CharT, Traits, Allocator> (s).append (sv);
	}


template<class CharT, class Traits, class Allocator>
	inline std::basic_string<CharT, Traits, Allocator>
	operator+ (std::basic_string_view<CharT, Traits> const& sv,
			   std::basic_string<CharT, Traits, Allocator> const& s)
	{
		return std::basic_string<CharT, Traits, Allocator> (sv).append (s);
	}

} // namespace string_view_plus_string_fixes


using namespace ostream_fixes;
using namespace string_view_plus_string_fixes;


inline void
dynamic_assert (bool expression, const char* message = nullptr) noexcept
{
	if (!expression)
	{
		if (message)
			std::clog << "Assertion failed: " << message << std::endl;
	}
}


/**
 * Prints debug output.
 */
#define xdebug(...) fprintf (stderr, __VA_ARGS__)

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
# define assert dynamic_assert
#else // XEFIS_ENABLE_ASSERT
# undef assert
# define assert(a, ...)
#endif // XEFIS_ENABLE_ASSERT

#ifdef __GNUC__
#define DO_PRAGMA(x) _Pragma(#x)
#define TODO(x) DO_PRAGMA(message ("TODO: " #x))
#endif // __GNUC__


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

#ifndef XEFIS__UTILITY__BLOB_H__INCLUDED
#define XEFIS__UTILITY__BLOB_H__INCLUDED

// Standard:
#include <cstddef>
#include <type_traits>

// Boost:
#include <boost/endian/conversion.hpp>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/string.h>


namespace xf {

inline Blob
make_blob (void const* pointer, std::size_t bytes)
{
	uint8_t const* begin = reinterpret_cast<uint8_t const*> (pointer);
	uint8_t const* end = begin + bytes;

	return Blob (begin, end);
}


inline std::string
to_hex_string (Blob const& blob)
{
	return to_hex_string (std::string (blob.begin(), blob.end()));
}


template<class Value,
		 class = std::enable_if_t<std::is_integral<Value>::value || std::is_floating_point<Value>::value>>
	inline Blob
	to_blob (Value value)
	{
		boost::endian::native_to_little (value);
		return make_blob (&value, sizeof (value));
	}

} // namespace xf

#endif


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

/**
 * Thrown by unserialize() functions.
 */
class InvalidBlobSize: public std::invalid_argument
{
  public:
	explicit InvalidBlobSize():
		invalid_argument ("BlobSerializer: invalid blob size")
	{ }
};


inline void
value_to_blob (bool value, Blob& blob)
{
	blob.resize (1);
	blob[0] = !!value;
}


inline void
value_to_blob (std::string const& value, Blob& blob)
{
	blob.assign (value.begin(), value.end());
}


template<class Trivial>
	inline void
	value_to_blob (Trivial value, std::enable_if_t<std::is_trivial_v<Trivial>, Blob>& blob)
	{
		uint8_t const* const begin = reinterpret_cast<uint8_t const*> (&value);
		uint8_t const* const end = begin + sizeof (value);

		blob.resize (std::distance (begin, end));
		std::copy (begin, end, blob.begin());
	}


template<class Quantity>
	inline void
	value_to_blob (Quantity value, std::enable_if_t<si::is_quantity_v<Quantity>, Blob>& blob)
	{
		value_to_blob (value.quantity(), blob);
	}


inline void
blob_to_value (Blob const& blob, bool& value)
{
	if (blob.size() != sizeof (value))
		throw InvalidBlobSize();

	value = !!blob[0];
}


inline void
blob_to_value (Blob const& blob, std::string& value)
{
	value.assign (blob.begin(), blob.end());
}


template<class Trivial>
	inline void
	blob_to_value (std::enable_if_t<std::is_trivial_v<Trivial>, Blob> const& blob, Trivial& value)
	{
		if (blob.size() != sizeof (value))
			throw InvalidBlobSize();

		uint8_t* const output = reinterpret_cast<uint8_t*> (&value);

		std::copy (blob.begin(), blob.end(), output);
	}


template<class Quantity>
	inline void
	blob_to_value (std::enable_if_t<si::is_quantity_v<Quantity>, Blob> const& blob, Quantity& value)
	{
		typename Quantity::Value aux;
		blob_to_value (blob, aux);
		value = Quantity (aux);
	}


inline std::string
to_hex_string (Blob const& blob)
{
	return to_hex_string (std::string (blob.begin(), blob.end()));
}

} // namespace xf

#endif


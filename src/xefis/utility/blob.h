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
#include <xefis/core/stdexcept.h>
#include <xefis/utility/string.h>
#include <xefis/utility/span.h>


namespace xf {

class BlobView: public Span<uint8_t const>
{
  public:
	using Span<uint8_t const>::Span;

	BlobView (Blob const& blob):
		Span (blob)
	{ }
};


/**
 * Thrown by unserialize() functions.
 */
class InvalidBlobSize: public InvalidArgument
{
  public:
	explicit InvalidBlobSize():
		InvalidArgument ("invalid blob size")
	{ }
};


template<class Trivial>
	inline void
	value_to_blob (Trivial value, std::enable_if_t<std::is_trivial_v<Trivial>, Blob>& blob)
	{
		boost::endian::native_to_little (value);

		uint8_t const* const begin = reinterpret_cast<uint8_t const*> (&value);
		uint8_t const* const end = begin + sizeof (value);

		blob.resize (std::distance (begin, end));
		std::copy (begin, end, blob.begin());
	}


template<class Quantity>
	inline void
	value_to_blob (Quantity value, std::enable_if_t<si::is_quantity_v<Quantity>, Blob>& blob)
	{
		blob = si::to_blob (value);
	}


inline void
blob_to_value (BlobView const blob, bool& value)
{
	if (blob.size() != sizeof (value))
		throw InvalidBlobSize();

	value = !!blob[0];
}


inline void
blob_to_value (BlobView const blob, std::string& value)
{
	value.assign (blob.cbegin(), blob.cend());
}


template<class Trivial>
	inline void
	blob_to_value (std::enable_if_t<std::is_trivial_v<Trivial>, BlobView> const blob, Trivial& value)
	{
		if (blob.size() != sizeof (value))
			throw InvalidBlobSize();

		uint8_t* const output = reinterpret_cast<uint8_t*> (&value);
		std::copy (blob.cbegin(), blob.cend(), output);
		boost::endian::little_to_native (value);
	}


template<class Quantity>
	inline void
	blob_to_value (std::enable_if_t<si::is_quantity_v<Quantity>, BlobView> const blob, Quantity& value)
	{
		// TODO Inefficient, change to use BlobView.
		si::parse (Blob { blob.cbegin(), blob.cend() }, value);
	}


inline std::string
to_hex_string (BlobView const blob)
{
	return to_hex_string (std::string (blob.cbegin(), blob.cend()));
}

} // namespace xf

#endif


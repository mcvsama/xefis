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

// Neutrino:
#include <neutrino/blob.h>

// Xefis:
#include <xefis/config/all.h>


// Add some Xefis-specific stuff to the neutrino namespace.
namespace neutrino {

inline void
value_to_blob (float16_t value, Blob& blob)
{
	union {
		float16_t	value;
		uint16_t	equivalent_int;
		uint8_t		data[sizeof (float16_t)];
	} u { value };

	boost::endian::native_to_little (u.equivalent_int);
	blob.resize (sizeof (u));
	std::copy (std::begin (u.data), std::end (u.data), blob.begin());
}


inline void
blob_to_value (BlobView const blob, float16_t& value)
{
	if (blob.size() != sizeof (value))
		throw InvalidBlobSize (blob.size(), sizeof (value));

	union {
		float16_t	value;
		uint16_t	equivalent_int;
		uint8_t		data[sizeof (float16_t)];
	} u { 0.0_half };

	std::copy (blob.cbegin(), blob.cend(), u.data);
	boost::endian::little_to_native (u.equivalent_int);
	value = u.value;
}

} // namespace neutrino

#endif


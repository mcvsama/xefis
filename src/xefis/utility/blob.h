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
#include <cstring>

// Neutrino:
#include <neutrino/blob.h>

// Xefis:
#include <xefis/config/all.h>


// Add some Xefis-specific stuff to the neutrino namespace.
namespace neutrino {

inline void
value_to_blob (float16_t const value, Blob& blob)
{
	blob.resize (sizeof (value));
	std::memcpy (blob.data(), &value, sizeof (value));
}


inline void
blob_to_value (BlobView const blob, float16_t& value)
{
	if (blob.size() != sizeof (value))
		throw InvalidBlobSize (blob.size(), sizeof (value));

	std::memcpy (&value, blob.data(), sizeof (value));
}

} // namespace neutrino

#endif


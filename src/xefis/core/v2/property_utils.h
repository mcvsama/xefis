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

#ifndef XEFIS__CORE__V2__PROPERTY_UTILS_H__INCLUDED
#define XEFIS__CORE__V2__PROPERTY_UTILS_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/property.h>


namespace v2 {

template<class Value>
	inline std::string
	to_string (Property<Value> const& property)
	{
		using std::string;

		return to_string (property.get());
	}


template<class Value>
	inline Blob
	to_blob (Property<Value> const& property)
	{
		using si::to_blob;

		return to_blob (property.get());
	}


inline std::string
to_string (Property<bool> const& property)
{
	return property.get() ? "true" : "false";
}


inline Blob
to_blob (Property<bool> const& property)
{
	return property.get() ? Blob ({ 0x01 }) : Blob ({ 0x00 });
}

} // namespace v2

#endif


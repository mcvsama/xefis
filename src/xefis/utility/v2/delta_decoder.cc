/* vim:ts=4
 *
 * Copyleft 2008…2013  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

// Standard:
#include <cstddef>

// Local:
#include "delta_decoder.h"


namespace v2 {

DeltaDecoder::DeltaDecoder (PropertyIn<int64_t>& value_property, Callback callback):
	_prev (value_property.value_or (0)),
	_property (value_property),
	_callback (callback)
{ }


void
DeltaDecoder::operator()()
{
	if (_property.valid() && *_property != _prev)
	{
		auto cur = *_property;

		if (_callback)
			_callback (cur - _prev);

		_prev = cur;
	}
}

} // namespace v2


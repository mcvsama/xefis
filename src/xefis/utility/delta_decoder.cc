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


namespace xf {

DeltaDecoder::DeltaDecoder (v1::PropertyInteger value_property, Callback callback):
	_prev (value_property.read (0)),
	_value_property (value_property),
	_callback (callback)
{ }


void
DeltaDecoder::data_updated()
{
	if (_value_property.valid_and_fresh() && _callback)
	{
		auto cur = *_value_property;
		_callback (cur - _prev);
		_prev = cur;
	}
}

} // namespace xf


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

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "paint_request.h"


namespace xf {

AsyncPaintRequest&
AsyncPaintRequest::operator= (AsyncPaintRequest&& other)
{
	if (_paint_request)
		_paint_request->_finished = true;

	_paint_request = other._paint_request;
	other._paint_request = nullptr;
	return *this;
}

} // namespace xf


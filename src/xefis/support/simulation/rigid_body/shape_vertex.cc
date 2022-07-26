/* vim:ts=4
 *
 * Copyleft 2019  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

// Local:
#include "shape_vertex.h"

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>


namespace xf::rigid_body {

void
ShapeVertex::rotate (RotationMatrix<BodySpace> const& rotation)
{
	_position = rotation * _position;

	if (_normal)
		_normal = rotation * *_normal;
}

} // namespace xf::rigid_body

